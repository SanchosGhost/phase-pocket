#pragma once
#include <JuceHeader.h>
#include "PocketDSP.h"
struct PocketTrace { float inLo=0,inHi=0,keyLo=0,keyHi=0,outLo=0,outHi=0,gain=1; double time=0; };
struct SpectrumTrace { std::array<float,32> mid{},side{}; SpectrumTrace(){mid.fill(1);side.fill(1);} };
class PhasePocketAudioProcessor final : public juce::AudioProcessor, private juce::AudioProcessorValueTreeState::Listener, private juce::AsyncUpdater {
public:
 PhasePocketAudioProcessor(); ~PhasePocketAudioProcessor() override;
 bool popSpectrum(SpectrumTrace&); bool popTrace(PocketTrace&);
 std::atomic<bool> displayBypass{false},editorOpen{false}; std::atomic<int> editorWidth{0}; std::atomic<float> keyPeak{0},gainMeter{1};
 void prepareToPlay(double,int) override; void releaseResources() override {} void reset() override;
 bool isBusesLayoutSupported(const BusesLayout&) const override; void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 void processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer& m) override{processAudio(b,m,true);} juce::AudioProcessorEditor* createEditor() override;
 juce::AudioProcessorParameter* getBypassParameter() const override{return parameters.getParameter("bypass");} bool hasEditor() const override{return true;} const juce::String getName() const override{return JucePlugin_Name;}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;} double getTailLengthSeconds() const override{return pocket::N/juce::jmax(1.,getSampleRate());}
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;} void setCurrentProgram(int) override{} const juce::String getProgramName(int) override{return {};} void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override; static juce::AudioProcessorValueTreeState::ParameterLayout layout(); juce::AudioProcessorValueTreeState parameters;
private:
 pocket::Engine engine; std::atomic<float>* influence=nullptr,*smoothing=nullptr,*mode=nullptr,*scLow=nullptr,*scHigh=nullptr,*bypass=nullptr,*msBalance=nullptr,*duration=nullptr,*sustain=nullptr;
 std::atomic<int> requestedMode{1},activeMode{1}; void parameterChanged(const juce::String&,float) override; void handleAsyncUpdate() override; void applyRequestedMode(); void processAudio(juce::AudioBuffer<float>&,juce::MidiBuffer&,bool);
 double traceTime=0; int captured=0,decimation=40; juce::AbstractFifo fifo{2048}; std::array<PocketTrace,2048> traces{}; PocketTrace capture; juce::AbstractFifo spectrumFifo{8}; std::array<SpectrumTrace,8> spectrumTraces{};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessor)
};
