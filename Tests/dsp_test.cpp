#include <memory>
#include "../Source/PocketDSP.h"
#include <iostream>
#include <random>
#include <cstdlib>
void require(bool b,const char* message){if(!b){std::cerr<<"FAIL: "<<message<<'\n';std::exit(1);}}
int main(){
    std::mt19937 rng(42);std::uniform_real_distribution<float>d(-1,1);
    for(int i=0;i<200000;++i){pocket::Complex b(d(rng),d(rng)),k(d(rng),d(rng));float g=pocket::budgetGain(b,k);require(g>=0&&g<=1,"gain range");require(std::abs(k+g*b)<=std::max(std::abs(k),std::abs(b))+2e-6f,"spectral budget");}
    for(double sr:{44100.,48000.,96000.,192000.})for(int mode:{0,1}){
        auto e=std::make_unique<pocket::Engine>();
        for(float influence:{0.f,1.f}){
            e->reset(sr,influence,mode);e->configure(influence,0,mode);
            std::array<float,16000>source{};for(auto& x:source)x=d(rng);
            for(int n=0;n<16000;++n){auto r=e->process({source[n],source[n]},influence==0?std::array<float,2>{d(rng),d(rng)}:std::array<float,2>{0,0});float want=n>=pocket::N?source[n-pocket::N]:0;require(std::abs(r.out[0]-want)<3e-6f,"identity including startup and reported latency");}
        }
        e->reset(sr,1,1);e->configure(1,0,1);
        std::array<float,16000>key{};for(auto& x:key)x=d(rng);
        for(int n=0;n<16000;++n){auto r=e->process({0.7f,0.35f},{key[n],key[n]});float want=n>=pocket::N?0.7f*(1-std::abs(key[n-pocket::N])):0;require(std::abs(r.out[0]-want)<2e-6f,"amplitude exact waveform");require(std::abs(r.out[1]-r.out[0]*0.5f)<2e-6f,"linked stereo");}
    }
    pocket::Engine e;e.reset(48000,1,1);e.configure(1,100,1);
    for(int n=0;n<8000;++n){auto r=e.process({1,1},{n==0?1.f:0.f,0});if(n==pocket::N+4800)require(std::abs(r.out[0]-(1-std::exp(-1.f)))<0.001f,"100ms release time constant");}
    e.reset(48000,1,0);e.configure(1,0,0);
    for(int n=0;n<20000;++n){float x=0.8f*std::sin(float(6.283185307179586*93.75*n/48000));auto r=e.process({x,x},{x,x});if(n>6000)require(std::abs(r.out[0])<0.025f,"identical in-phase sources carved in spectrum mode");}
    for(int n=0;n<12000;++n){if(n%300==0)e.configure((n/300)%2?0.f:1.f,40,(n/600)%2);auto r=e.process({d(rng),d(rng)},{d(rng),d(rng)});require(std::isfinite(r.out[0])&&std::isfinite(r.out[1]),"automation and switch finite");}
    std::cout<<"PASS: quadratic budget; identity/startup/latency at four sample rates; 0% dry; amplitude waveform; stereo link; smoothing; spectral carve; mode automation.\n";
}
