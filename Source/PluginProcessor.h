#pragma once
#include <JuceHeader.h>
#include "PocketDSP.h"
struct PocketTrace {float inLo=0,inHi=0,keyLo=0,keyHi=0,outLo=0,outHi=0,gain=1;};
class PhasePocketAudioProcessor final : public juce::AudioProcessor {
public:
    PhasePocketAudioProcessor();
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return JucePlugin_Name;}
    bool acceptsMidi() const override {return false;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return pocket::N/juce::jmax(1.0,getSampleRate());}
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {return {};}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    juce::AudioProcessorValueTreeState parameters;
    bool popTrace(PocketTrace&);
    std::atomic<float> keyPeak{0},gainMeter{1};
private:
    pocket::Engine engine;
    std::atomic<float>* influence=nullptr,* smoothing=nullptr,* mode=nullptr,* scLow=nullptr,* scHigh=nullptr;
    juce::AbstractFifo fifo{2048};
    std::array<PocketTrace,2048> traces{};
    PocketTrace capture;
    int captured=0,decimation=40;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessor)
};
