#include "../Source/PocketDSP.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
static void check(bool ok,const char* message){if(!ok){std::cerr<<"FAIL "<<message<<'\n';std::exit(1);}}
int main(){
 check(pocket::Engine::latency(1)==0,"Amplitude latency");check(pocket::Engine::latency(0)==2048,"Spectrum latency");
 {auto e=std::make_unique<pocket::Engine>();e->reset(48000,1,1);e->configure(1,200,1,20,20000,false,0,2000,1);float last=1;for(int n=0;n<6000;++n)last=e->process({1,1},{.5f,.5f}).out[0];check(last<.55f,"Infinite duration follows key");}
 for(float keyLevel:{.8f,.02f}){auto e=std::make_unique<pocket::Engine>();e->reset(48000,1,1);e->configure(1,40,1,20,20000,false,0,30,0);float last=0;for(int n=0;n<6000;++n)last=e->process({.5f,.5f},{keyLevel,keyLevel}).out[0];check(std::abs(last-.5f)<1e-3f,"Duration/Sustain releases sustained key");}
 {auto e=std::make_unique<pocket::Engine>();e->reset(48000,1,1);e->configure(1,500,1);for(int n=0;n<1000;++n)e->process({.5f,.5f},{.8f,.8f});float out=0;for(int n=0;n<1000;++n)out=e->process({.5f,.5f},{0,0}).out[0];check(std::abs(out-.5f)<1e-3f,"No smoothing tail");}
 {auto e=std::make_unique<pocket::Engine>();e->reset(48000,1,0);e->configure(1,40,0);float guarded=1;for(int n=0;n<5000;++n){float bass=n==0?1.f:0.f,key=n==0?1.f:0.f;auto y=e->process({bass,bass},{key,key});if(n==pocket::N)guarded=y.out[0];}check(std::abs(guarded)<1e-3f,"Spectrum transient guard");}
 std::cout<<"PASS v0.6 latency, envelope shaping, tail and transient guard\n";
}
