#include "../Source/PocketDSP.h"
#include <iostream>
#include <cstdlib>
#include <memory>
static void check(bool b,const char* s){if(!b){std::cerr<<"FAIL "<<s<<'\n';std::exit(1);}}
static double tone(double f,double sr){pocket::SidechainFilter filter;filter.reset(sr);filter.set(3000,20000);double sum=0;for(int n=0;n<(int)sr;++n){float v=(float)std::sin(6.283185307179586*f*n/sr);auto y=filter.process({v,v});if(n>sr/2)sum+=y[0]*y[0];}return std::sqrt(sum/(sr/2));}
int main(){
 for(float influence:{0.f,0.5f,1.f,1.5f})for(float base:{0.f,0.2f,0.5f,1.f}){float g=pocket::depthGain(base,influence);check(g>=0&&g<=1,"no polarity inversion");}
 for(double sr:{44100.,48000.,96000.,192000.}){
  auto engine=std::make_unique<pocket::Engine>();engine->reset(sr,1.5f,1);engine->configure(1.5f,0,1);
  for(int n=0;n<6000;++n){auto r=engine->process({0.8f,0.4f},{0.5f,0.5f});if(n>pocket::N){check(std::abs(r.out[0]-0.2f)<1e-6,"150 percent depth: half-level key gives 75 percent duck");check(std::abs(r.gain-0.25f)<1e-6,"gain graph matches");}}
  engine->reset(sr,1.5f,1);engine->configure(1.5f,0,1);
  for(int n=0;n<6000;++n){auto r=engine->process({0.8f,0.4f},{1.f,1.f});if(n>pocket::N)check(r.out[0]==0,"deep duck clamps to zero");}
  engine->reset(sr,0,1);engine->configure(0,10,1,3000,8000);
  for(int n=0;n<6000;++n){auto r=engine->process({0.8f,0.4f},{0.5f,0.5f});if(n>pocket::N)check(r.out[0]==0.8f,"filter cannot touch bass at zero influence");}
  check(tone(80,sr)<0.002,"high-only key rejects kick low tone");check(tone(6000,sr)>0.6,"high-only key passes high tone");
  pocket::SidechainFilter filter;filter.reset(sr);
  for(int n=0;n<2000;++n){float x=(float)std::sin(n*0.4);auto y=filter.process({x,-x});check(y[0]==x && y[1]==-x,"Full Range exact key bypass");}
  for(int n=0;n<30000;++n){if(n%300==0)filter.set(n%600?20:20000,n%900?20000:20);auto y=filter.process({0.8f,-0.8f});check(std::isfinite(y[0])&&std::abs(y[0])<4,"filter automation stable");}
 }
 std::cout<<"PASS: 150% depth, zero-floor, live gain, filter bypass, key high-pass selectivity, sample-rate and automation checks\n";
}
