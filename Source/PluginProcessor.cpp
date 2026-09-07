#include "PluginProcessor.h"
#include "PluginEditor.h"
PhasePocketAudioProcessor::PhasePocketAudioProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withInput("Sidechain",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"PARAMETERS",layout()) {influence=parameters.getRawParameterValue("amount");smoothing=parameters.getRawParameterValue("release");mode=parameters.getRawParameterValue("mode");scLow=parameters.getRawParameterValue("scLow");scHigh=parameters.getRawParameterValue("scHigh");bypass=parameters.getRawParameterValue("bypass");}
juce::AudioProcessorValueTreeState::ParameterLayout PhasePocketAudioProcessor::layout(){
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 p.push_back(std::make_unique<juce::AudioParameterFloat>("amount","Influence",juce::NormalisableRange<float>(0,150,.1f),100));
 p.push_back(std::make_unique<juce::AudioParameterFloat>("tolerance","Legacy Tolerance",0.f,6.f,1.f));
 p.push_back(std::make_unique<juce::AudioParameterFloat>("low","Legacy Low",20.f,150.f,25.f));
 p.push_back(std::make_unique<juce::AudioParameterFloat>("high","Legacy High",80.f,500.f,220.f));
 p.push_back(std::make_unique<juce::AudioParameterFloat>("maxReduction","Legacy Reduction",0.f,48.f,24.f));
 p.push_back(std::make_unique<juce::AudioParameterFloat>("release","Smoothing",juce::NormalisableRange<float>(0,500,.1f,.4f),40));
 p.push_back(std::make_unique<juce::AudioParameterBool>("phaseAware","Legacy Phase",true));
 p.push_back(std::make_unique<juce::AudioParameterChoice>("mode","Mode",juce::StringArray{"Spectrum","Amplitude"},1));
 p.push_back(std::make_unique<juce::AudioParameterFloat>("scLow","Sidechain Low",juce::NormalisableRange<float>(20,20000,1,.2f),20));
 p.push_back(std::make_unique<juce::AudioParameterFloat>("scHigh","Sidechain High",juce::NormalisableRange<float>(20,20000,1,.2f),20000));
 p.push_back(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false));return {p.begin(),p.end()};
}
void PhasePocketAudioProcessor::prepareToPlay(double sr,int){engine.reset(sr,influence->load()*.01f,(int)mode->load());decimation=juce::jmax(1,(int)(sr/1200));captured=0;capture={};setLatencySamples(pocket::N);}
void PhasePocketAudioProcessor::reset(){engine.reset(juce::jmax(1.,getSampleRate()),influence->load()*.01f,(int)mode->load());captured=0;capture={};}
bool PhasePocketAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {if(l.inputBuses.size()!=2||l.outputBuses.size()!=1)return false;auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet(),side=l.getChannelSet(true,1);return in==out&&(out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo())&&(side.isDisabled()||side==juce::AudioChannelSet::mono()||side==juce::AudioChannelSet::stereo());}
void PhasePocketAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& m){processAudio(b,m,false);}
void PhasePocketAudioProcessor::processAudio(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&,bool hostBypass){
 juce::ScopedNoDenormals noDenormals;auto main=getBusBuffer(buffer,true,0),side=getBusBuffer(buffer,true,1);
 engine.configure(influence->load()*.01f,smoothing->load(),(int)mode->load(),scLow->load(),scHigh->load(),hostBypass||bypass->load()>.5f);
 if(main.getNumChannels()==0)return;
 float peak=0,minimum=1;double traceStep=1.0/juce::jmax(1.,getSampleRate());bool showTrace=editorOpen.load(std::memory_order_relaxed);
 for(int n=0;n<buffer.getNumSamples();++n){std::array<float,2>b{},k{};for(int c=0;c<2;++c){b[c]=main.getSample(juce::jmin(c,main.getNumChannels()-1),n);if(side.getNumChannels()>0)k[c]=side.getSample(juce::jmin(c,side.getNumChannels()-1),n);}peak=juce::jmax(peak,std::abs(k[0]),std::abs(k[1]));auto v=engine.process(b,k);for(int c=0;c<main.getNumChannels();++c)main.setSample(c,n,v.out[(size_t)c]);minimum=juce::jmin(minimum,v.gain);traceTime+=traceStep;if(!showTrace){captured=0;continue;}capture.time=traceTime;
  if(captured==0)capture={v.dry[0],v.dry[0],v.key[0],v.key[0],v.out[0],v.out[0],v.gain,traceTime};
  else{capture.inLo=juce::jmin(capture.inLo,v.dry[0]);capture.inHi=juce::jmax(capture.inHi,v.dry[0]);capture.keyLo=juce::jmin(capture.keyLo,v.key[0]);capture.keyHi=juce::jmax(capture.keyHi,v.key[0]);capture.outLo=juce::jmin(capture.outLo,v.out[0]);capture.outHi=juce::jmax(capture.outHi,v.out[0]);capture.gain=juce::jmin(capture.gain,v.gain);}
  if(++captured>=decimation){int a,s,b2,s2;fifo.prepareToWrite(1,a,s,b2,s2);if(s){traces[(size_t)a]=capture;fifo.finishedWrite(1);}captured=0;}
 }
 keyPeak.store(peak);gainMeter.store(minimum);
}
bool PhasePocketAudioProcessor::popTrace(PocketTrace& v){int a,s,b,s2;fifo.prepareToRead(1,a,s,b,s2);if(!s)return false;v=traces[(size_t)a];fifo.finishedRead(1);return true;}
void PhasePocketAudioProcessor::getStateInformation(juce::MemoryBlock& d){if(auto x=parameters.copyState().createXml())copyXmlToBinary(*x,d);}
void PhasePocketAudioProcessor::setStateInformation(const void* d,int n){if(auto x=getXmlFromBinary(d,n))if(x->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessorEditor* PhasePocketAudioProcessor::createEditor(){return new PhasePocketAudioProcessorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new PhasePocketAudioProcessor();}
