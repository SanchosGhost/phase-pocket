#include "../Source/PocketDSP.h"
#include <iostream>
#include <memory>
#include <random>
#include <cstdlib>
#include <vector>
static void check(bool ok,const char* name){if(!ok){std::cerr<<"FAIL "<<name<<'\n';std::exit(1);}}
static double bandGain(double frequency){
    constexpr double sr=48000.;auto e=std::make_unique<pocket::Engine>();e->reset(sr);e->configure(.65f,2000,20,20000,false,0,500,2000);
    double inPower=0,outPower=0;const int latency=e->latency();
    for(int n=0;n<48000;++n){float x=float(.5*std::sin(6.283185307179586*frequency*n/sr));auto v=e->process({x,x},{.8f,.8f});if(n>12000){float delayed=float(.5*std::sin(6.283185307179586*frequency*(n-latency)/sr));inPower+=double(delayed)*delayed;outPower+=double(v.out[0])*v.out[0];}}
    return std::sqrt(outPower/inPower);
}
int main(){
    check(pocket::Engine::latencyForRate(48000)==48,"1 ms = 48 samples");
    check(pocket::Engine::latencyForRate(44100)==45,"44.1k latency rounds up");
    for(float d:{1.f,2.f,5.f,10.f,100.f,200.f}){check(pocket::Engine::durationGain(d*.8,d)>.9999f,"full hold at 80 percent");check(std::abs(pocket::Engine::durationGain(d*.9,d)-.5f)<1e-5f,"half fade at 90 percent");check(pocket::Engine::durationGain(d,d)<1e-6f,"zero at requested duration");}
    for(double sr:{44100.,48000.,96000.,192000.}){
        const int latency=pocket::Engine::latencyForRate(sr);std::mt19937 random(17);std::uniform_real_distribution<float> noise(-.8f,.8f);std::vector<std::array<float,2>> input(16000);for(auto& s:input)s={noise(random),noise(random)};
        for(bool noKey:{true,false}){auto e=std::make_unique<pocket::Engine>();e->reset(sr,noKey?1.f:0.f);e->configure(noKey?1.f:0.f,2000);for(size_t i=0;i<input.size();++i){auto v=e->process(input[i],noKey?std::array<float,2>{0,0}:std::array<float,2>{noise(random),noise(random)});auto wanted=i>=size_t(latency)?input[i-size_t(latency)]:std::array<float,2>{0,0};check(v.out==wanted,"delayed dry identity");}}
        {auto e=std::make_unique<pocket::Engine>();e->reset(sr);e->configure(1,2000);for(int n=0;n<latency+8;++n){float x=n==0?1.f:0.f;auto v=e->process({x,x},{x,x});if(n==latency)check(std::abs(v.out[0])<1e-6f,"first impulse fully caught");}}
        for(float duration:{1.f,2.f,5.f,10.f,100.f,200.f}){auto e=std::make_unique<pocket::Engine>();e->reset(sr);e->configure(1,duration);float beforeFade=1,halfFade=0,afterEnd=0;for(int n=0;n<int(sr*.27)+latency;++n){auto v=e->process({1,1},{.8f,.8f});int aligned=n-latency;if(aligned<0)continue;double ms=aligned*1000./sr;if(aligned==int(std::floor(duration*.75*sr*.001)))beforeFade=v.gain;if(aligned==int(std::ceil(duration*.9*sr*.001)))halfFade=v.gain;if(ms>=duration+.1){afterEnd=v.gain;check(std::abs(v.gain-1)<1e-5f,"no smoothing extension or event retrigger");}}check(beforeFade<.21f,"duration keeps full depth before fade");check(halfFade>.3f&&halfFade<.9f,"audible proportional fade");check(afterEnd>.9999f,"1 ms length is real");}
        for(float balance:{-1.f,1.f}){auto e=std::make_unique<pocket::Engine>();e->reset(sr);e->configure(1,2000,20,20000,false,balance);pocket::Sample v;for(int n=0;n<int(sr*.2);++n)v=e->process(balance<0?std::array<float,2>{.5f,-.5f}:std::array<float,2>{.5f,.5f},{1,1});check(std::abs(v.out[0]-.5f)<1e-5f,"M/S leaves excluded component dry");}
        {auto e=std::make_unique<pocket::Engine>();e->reset(sr);e->configure(1,2000);for(int n=0;n<latency+64;++n)e->process({.3f,-.5f},{1,1});e->configure(1,2000,20,20000,true);auto v=e->process({.3f,-.5f},{1,1});check(std::abs(v.out[0]-.3f)<1e-6f&&std::abs(v.out[1]+.5f)<1e-6f,"bypass switches to delayed dry immediately");}
        {auto e=std::make_unique<pocket::Engine>();e->reset(sr);e->configure(0,2000,20,20000,false,0,20,20000,6);pocket::Sample v;for(int n=0;n<int(sr*.2);++n)v=e->process({.25f,.25f},{0,0});check(std::abs(v.out[0]-.25f*std::pow(10.f,6.f/20.f))<1e-4f,"output gain reaches +6 dB");e->configure(0,2000,20,20000,false,0,20,20000,-100);for(int n=0;n<int(sr*.2);++n)v=e->process({.25f,.25f},{0,0});check(std::abs(v.out[0])<1e-6f,"output gain minus infinity mutes");}
    }
    const double low=bandGain(100),inside=bandGain(1000),high=bandGain(8000);
    check(inside<.65,"selected processing band is reduced");check(low>.9,"low frequencies stay substantially dry");check(high>.9,"high frequencies stay substantially dry");
    std::cout<<"PASS v0.8: v0.7 behavior plus smooth selective processing range (100/1000/8000 Hz gains "<<low<<", "<<inside<<", "<<high<<")\n";
}
