#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float pi = juce::MathConstants<float>::pi;
constexpr float eps = 1.0e-12f;
}

SpectralSidechainEngine::SpectralSidechainEngine()
{
    for (int i = 0; i < fftSize; ++i)
    {
        const auto hann = 0.5f - 0.5f * std::cos(2.0f * pi * static_cast<float>(i) / static_cast<float>(fftSize));
        window[static_cast<size_t>(i)] = std::sqrt(juce::jmax(0.0f, hann));
    }
    reset();
}

void SpectralSidechainEngine::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
}

void SpectralSidechainEngine::reset()
{
    bassRing.fill(0.0f); kickRing.fill(0.0f); overlap.fill(0.0f);
    bassFFT.fill(0.0f); kickFFT.fill(0.0f); ready.fill(0.0f); gainState.fill(1.0f);
    writePosition = 0; hopCounter = 0; readyPosition = hopSize; samplesSeen = 0;
    reductionDb.store(0.0f);
}

float SpectralSidechainEngine::processSample(float bass, float kick, const Settings& settings)
{
    const auto output = readyPosition < hopSize ? ready[static_cast<size_t>(readyPosition++)] : 0.0f;
    bassRing[static_cast<size_t>(writePosition)] = bass;
    kickRing[static_cast<size_t>(writePosition)] = kick;
    writePosition = (writePosition + 1) % fftSize;
    ++samplesSeen;
    if (++hopCounter >= hopSize)
    {
        hopCounter = 0;
        if (samplesSeen >= fftSize) processFrame(settings);
    }
    return output;
}

void SpectralSidechainEngine::processFrame(const Settings& settings)
{
    bassFFT.fill(0.0f); kickFFT.fill(0.0f);
    for (int i = 0; i < fftSize; ++i)
    {
        const auto sourceIndex = (writePosition + i) % fftSize;
        const auto w = window[static_cast<size_t>(i)];
        bassFFT[static_cast<size_t>(i)] = bassRing[static_cast<size_t>(sourceIndex)] * w;
        kickFFT[static_cast<size_t>(i)] = kickRing[static_cast<size_t>(sourceIndex)] * w;
    }
    fft.performRealOnlyForwardTransform(bassFFT.data(), true);
    fft.performRealOnlyForwardTransform(kickFFT.data(), true);

    const auto tolerance = juce::Decibels::decibelsToGain(settings.toleranceDb);
    const auto floorGain = juce::Decibels::decibelsToGain(-settings.maxReductionDb);
    const auto releaseSeconds = juce::jmax(0.001f, settings.releaseMs * 0.001f);
    const auto releaseCoeff = std::exp(-static_cast<float>(hopSize) / static_cast<float>(sampleRate * releaseSeconds));
    float minimumGain = 1.0f;

    for (int bin = 0; bin <= fftSize / 2; ++bin)
    {
        const auto frequency = static_cast<float>(bin) * static_cast<float>(sampleRate) / static_cast<float>(fftSize);
        const auto complexIndex = 2 * bin;
        const float br = bassFFT[static_cast<size_t>(complexIndex)];
        const float bi = bassFFT[static_cast<size_t>(complexIndex + 1)];
        const float kr = kickFFT[static_cast<size_t>(complexIndex)];
        const float ki = kickFFT[static_cast<size_t>(complexIndex + 1)];
        const float bassPower = br * br + bi * bi;
        const float kickPower = kr * kr + ki * ki;
        const float cross = settings.phaseAware ? (br * kr + bi * ki) : 0.0f;
        const float targetMagnitude = juce::jmax(std::sqrt(bassPower), std::sqrt(kickPower)) * tolerance;
        const float targetPower = targetMagnitude * targetMagnitude;
        const float predictedPower = bassPower + kickPower + 2.0f * cross;
        float targetGain = 1.0f;
        if (frequency >= settings.lowHz && frequency <= settings.highHz && predictedPower > targetPower && bassPower > eps)
        {
            const float discriminant = juce::jmax(0.0f, cross * cross + bassPower * (targetPower - kickPower));
            const float strictGain = (-cross + std::sqrt(discriminant)) / (bassPower + eps);
            targetGain = 1.0f - settings.amount * (1.0f - juce::jlimit(0.0f, 1.0f, strictGain));
            targetGain = juce::jmax(floorGain, targetGain);
        }
        auto& smoothed = gainState[static_cast<size_t>(bin)];
        smoothed = targetGain < smoothed ? targetGain : releaseCoeff * smoothed + (1.0f - releaseCoeff) * targetGain;
        minimumGain = juce::jmin(minimumGain, smoothed);
        bassFFT[static_cast<size_t>(complexIndex)] = br * smoothed;
        bassFFT[static_cast<size_t>(complexIndex + 1)] = bi * smoothed;
    }

    fft.performRealOnlyInverseTransform(bassFFT.data());
    constexpr float overlapScale = 0.5f;
    for (int i = 0; i < fftSize; ++i)
        overlap[static_cast<size_t>(i)] += bassFFT[static_cast<size_t>(i)] * window[static_cast<size_t>(i)] * overlapScale;
    std::copy_n(overlap.begin(), hopSize, ready.begin());
    std::move(overlap.begin() + hopSize, overlap.end(), overlap.begin());
    std::fill(overlap.end() - hopSize, overlap.end(), 0.0f);
    readyPosition = 0;
    reductionDb.store(juce::Decibels::gainToDecibels(juce::jmax(minimumGain, 1.0e-6f)));
}

PhasePocketAudioProcessor::PhasePocketAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout PhasePocketAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("amount", "Amount", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 75.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("tolerance", "Tolerance", juce::NormalisableRange<float>(0.0f, 6.0f, 0.1f), 1.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("low", "Low", juce::NormalisableRange<float>(20.0f, 150.0f, 1.0f, 0.5f), 25.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("high", "High", juce::NormalisableRange<float>(80.0f, 500.0f, 1.0f, 0.5f), 220.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("maxReduction", "Max Reduction", juce::NormalisableRange<float>(0.0f, 48.0f, 0.1f), 24.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(10.0f, 300.0f, 1.0f, 0.45f), 80.0f));
    p.push_back(std::make_unique<juce::AudioParameterBool>("phaseAware", "Phase Aware", true));
    return { p.begin(), p.end() };
}

void PhasePocketAudioProcessor::prepareToPlay(double sampleRate, int)
{
    for (auto& engine : engines) engine.prepare(sampleRate);
    setLatencySamples(SpectralSidechainEngine::fftSize);
}

bool PhasePocketAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getChannelSet(true, 0), mainOut = layouts.getChannelSet(false, 0);
    if (mainIn != mainOut || (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())) return false;
    const auto side = layouts.getChannelSet(true, 1);
    return side == juce::AudioChannelSet::mono() || side == juce::AudioChannelSet::stereo() || side.isDisabled();
}

void PhasePocketAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto main = getBusBuffer(buffer, true, 0);
    auto side = getBusBuffer(buffer, true, 1);
    SpectralSidechainEngine::Settings s;
    s.amount = parameters.getRawParameterValue("amount")->load() * 0.01f;
    s.toleranceDb = parameters.getRawParameterValue("tolerance")->load();
    s.lowHz = parameters.getRawParameterValue("low")->load();
    s.highHz = juce::jmax(s.lowHz, parameters.getRawParameterValue("high")->load());
    s.maxReductionDb = parameters.getRawParameterValue("maxReduction")->load();
    s.releaseMs = parameters.getRawParameterValue("release")->load();
    s.phaseAware = parameters.getRawParameterValue("phaseAware")->load() > 0.5f;
    for (int ch = 0; ch < main.getNumChannels(); ++ch)
    {
        auto* output = main.getWritePointer(ch);
        const auto* key = side.getNumChannels() > 0 ? side.getReadPointer(juce::jmin(ch, side.getNumChannels() - 1)) : nullptr;
        for (int n = 0; n < main.getNumSamples(); ++n)
            output[n] = engines[static_cast<size_t>(juce::jmin(ch, 1))].processSample(output[n], key != nullptr ? key[n] : 0.0f, s);
    }
}

float PhasePocketAudioProcessor::getReductionDb() const noexcept { return juce::jmin(engines[0].getReductionDb(), engines[1].getReductionDb()); }
void PhasePocketAudioProcessor::getStateInformation(juce::MemoryBlock& dest) { if (auto xml = parameters.copyState().createXml()) copyXmlToBinary(*xml, dest); }
void PhasePocketAudioProcessor::setStateInformation(const void* data, int size) { if (auto xml = getXmlFromBinary(data, size)) if (xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessorEditor* PhasePocketAudioProcessor::createEditor() { return new PhasePocketAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PhasePocketAudioProcessor(); }
