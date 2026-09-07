#pragma once

#include <JuceHeader.h>

class SpectralSidechainEngine
{
public:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 4;

    struct Settings
    {
        float amount = 0.75f;
        float toleranceDb = 1.0f;
        float lowHz = 25.0f;
        float highHz = 220.0f;
        float maxReductionDb = 24.0f;
        float releaseMs = 80.0f;
        bool phaseAware = true;
    };

    SpectralSidechainEngine();
    void prepare(double newSampleRate);
    void reset();
    float processSample(float bass, float kick, const Settings& settings);
    float getReductionDb() const noexcept { return reductionDb.load(); }

private:
    void processFrame(const Settings& settings);

    juce::dsp::FFT fft { fftOrder };
    std::array<float, fftSize> window {};
    std::array<float, fftSize> bassRing {}, kickRing {}, overlap {};
    std::array<float, fftSize * 2> bassFFT {}, kickFFT {};
    std::array<float, hopSize> ready {};
    std::array<float, fftSize / 2 + 1> gainState {};

    int writePosition = 0;
    int hopCounter = 0;
    int readyPosition = hopSize;
    int samplesSeen = 0;
    double sampleRate = 48000.0;
    std::atomic<float> reductionDb { 0.0f };
};

class PhasePocketAudioProcessor final : public juce::AudioProcessor
{
public:
    PhasePocketAudioProcessor();
    ~PhasePocketAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;
    float getReductionDb() const noexcept;

private:
    std::array<SpectralSidechainEngine, 2> engines;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessor)
};
