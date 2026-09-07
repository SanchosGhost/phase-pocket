#include "../Source/PocketDSP.h"
#include <memory>
#include <iostream>
#include <random>
#include <cstdlib>
void require(bool b,const char* m){if(!b){std::cerr<<"FAIL: "<<m<<'\n';std::exit(1);}}
int main(){std::mt19937 rng(42);std::uniform_real_distribution<float>d(-1,1);
 for(double sr:{44100.,48000.,96000.,192000.})for(int mode:{0,1}){auto e=std::make_unique<pocket::Engine>();for(float influence:{0.f,1.f}){e->reset(sr,influence,mode);e->configure(influence,0,mode);std::array<float,16000> source{};for(auto& x:source)x=d(rng);for(int n=0;n<16000;++n){auto v=e->process({source[n],source[n]},influence==0?std::array<float,2>{d(rng),d(rng)}:std::array<float,2>{0,0});float want=n>=pocket::N?source[n-pocket::N]:0;require(std::abs(v.out[0]-want)<3e-6f,"identity, startup and latency");}}e->reset(sr,1,1);e->configure(1,0,1);std::array<float,16000> key{};for(auto& x:key)x=d(rng);for(int n=0;n<16000;++n){auto v=e->process({.7f,.35f},{key[n],key[n]});float want=n>=pocket::N?.7f*(1-std::abs(key[n-pocket::N])):0;require(std::abs(v.out[0]-want)<2e-6f,"unchanged amplitude 100 percent");require(std::abs(v.out[1]-v.out[0]*.5f)<2e-6f,"stereo link");}}
 auto e=std::make_unique<pocket::Engine>();e->reset(48000,1,1);e->configure(1,100,1);for(int n=0;n<8000;++n){auto v=e->process({1,1},{n==0?1.f:0.f,0});if(n==pocket::N+4800)require(std::abs(v.out[0]-(1-std::exp(-1.f)))<.001f,"100ms recovery");}for(int n=0;n<12000;++n){if(n%300==0)e->configure((n/300)%2?0.f:1.5f,40,(n/600)%2,20,20000,(n/900)%2);auto v=e->process({d(rng),d(rng)},{d(rng),d(rng)});require(std::isfinite(v.out[0])&&std::isfinite(v.out[1]),"automation finite");}std::cout<<"PASS baseline: startup, four rates, unity, latency, amplitude, stereo, recovery, automation\n";}
