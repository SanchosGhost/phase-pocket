#pragma once
#include <array>
#include "SidechainFilter.h"
#include <complex>
#include <algorithm>
#include <cmath>
namespace pocket {
constexpr int N=2048,H=N/4,R=N*2;using Complex=std::complex<float>;
inline float budgetGain(Complex b,Complex k) noexcept{double pb=std::norm(std::complex<double>(b)),pk=std::norm(std::complex<double>(k)),x=std::real(std::complex<double>(b)*std::conj(std::complex<double>(k))),t=std::max(pb,pk),c=t-pk;if(pb<1e-24||pb+pk+2*x<=t)return 1;double root=std::sqrt(std::max(0.,x*x+pb*c)),g=x>=0?((root+x)>0?c/(root+x):0):(root-x)/pb;return(float)std::clamp(g,0.,1.);}
class FFT{std::array<int,N>rev{};std::array<Complex,N/2>roots{};public:FFT(){for(int i=0;i<N;++i){int x=i,r=0;for(int j=0;j<11;++j){r=(r<<1)|(x&1);x>>=1;}rev[i]=r;}for(int i=0;i<N/2;++i)roots[i]=std::polar(1.f,(float)(-6.283185307179586*i/N));}void run(std::array<Complex,N>&a,bool inverse)const noexcept{for(int i=0;i<N;++i)if(i<rev[i])std::swap(a[i],a[rev[i]]);for(int len=2;len<=N;len*=2)for(int i=0;i<N;i+=len)for(int j=0;j<len/2;++j){auto w=inverse?std::conj(roots[j*N/len]):roots[j*N/len],u=a[i+j],v=a[i+j+len/2]*w;a[i+j]=u+v;a[i+j+len/2]=u-v;}if(inverse)for(auto&v:a)v/=float(N);}};
struct Sample{std::array<float,2>dry{},key{},out{};float gain=1;};
class Engine{
 FFT fft;SidechainFilter keyFilter;static constexpr int bandCount=32;
 std::array<int,N/2+1>bandIndex{};std::array<float,N/2+1>bandFrac{};std::array<float,bandCount>bandDb{},lastCurve{};
 std::array<float,N>window{},ampGainDelay{},guardDelay{};std::array<std::array<float,N>,2>input{},key{};std::array<std::array<float,R>,2>ola{};std::array<float,R>gainOla{};std::array<std::array<Complex,N>,2>B{},K{};
 float bypassMix=0,targetBypass=0;double msPosition=0,targetMs=0;unsigned frameCounter=0;int pos=0,outPos=0,hop=0,priming=0;double rate=48000;
 float envelope=0,releaseSample=0,releaseFrame=0,amount=1,targetAmount=1,mode=0,targetMode=0,slew=0;
 float fastEnv=0,slowEnv=0,eventPeak=0,keyShape=0,targetDurationMs=2000,targetSustain=1;int eventAge=0,refractory=0,silenceSamples=0;bool eventActive=false;
 static float clean(float x)noexcept{return std::isfinite(x)?x:0;}
 float shapeKey(float level)noexcept{
  float fastC=(float)std::exp(-1.0/(rate*.0015)),slowC=(float)std::exp(-1.0/(rate*.035));fastEnv=level+fastC*(fastEnv-level);slowEnv=level+slowC*(slowEnv-level);if(refractory>0)--refractory;
  constexpr float floor=1e-7f;bool onset=level>floor&&fastEnv>std::max(slowEnv*1.8f,floor)&&refractory==0;
  if(onset||(!eventActive&&level>floor)){eventActive=true;eventAge=0;silenceSamples=0;eventPeak=std::max(level,fastEnv);refractory=std::max(1,(int)(rate*.012));}
  if(eventActive){++eventAge;eventPeak=std::max(eventPeak,level);float threshold=std::max(floor,eventPeak*.001f);if(fastEnv<threshold)++silenceSamples;else silenceSamples=0;if(silenceSamples>std::max(1,(int)(rate*.002)))eventActive=false;}
  float target=eventActive?1.f:0.f;bool infinite=targetDurationMs>=1999.5f;if(eventActive&&!infinite&&eventAge>(int)(rate*targetDurationMs*.001f))target=targetSustain;
  float fadeC=(float)std::exp(-1.0/(rate*.004));keyShape=target>keyShape?target:target+fadeC*(keyShape-target);if(target==0&&keyShape<1e-5f)keyShape=0;return keyShape;
 }
 void frame()noexcept{
  for(int c=0;c<2;++c){for(int i=0;i<N;++i){int j=(pos+i)%N;B[c][i]={input[c][j]*window[i],0};K[c][i]={key[c][j]*window[i],0};}fft.run(B[c],false);fft.run(K[c],false);}
  std::array<double,bandCount>bp{},kp{};double totalKeyPower=0;for(int k=1;k<=N/2;++k){if(k*rate/N>20000)continue;int j=bandIndex[k];float f=bandFrac[k];double b=std::norm(B[0][k])+std::norm(B[1][k]),v=std::norm(K[0][k])+std::norm(K[1][k]);totalKeyPower+=v;bp[j]+=b*(1-f);kp[j]+=v*(1-f);bp[j+1]+=b*f;kp[j+1]+=v*f;}
  std::array<float,bandCount>shape{};bool keyEnded=totalKeyPower<1e-14;for(int j=0;j<bandCount;++j){double b=0,k=0;for(int d=-1;d<=1;++d){int q=std::clamp(j+d,0,bandCount-1);double w=d==0?2:1;b+=bp[q]*w;k+=kp[q]*w;}float target=(float)(24*k/(b+k+N*N*1e-10));if(keyEnded)bandDb[j]=0;else bandDb[j]=target>bandDb[j]?target:target+releaseFrame*(bandDb[j]-target);if(bandDb[j]<1e-5f)bandDb[j]=0;}
  for(int j=0;j<bandCount;++j)shape[j]=(bandDb[std::max(0,j-1)]+2*bandDb[j]+bandDb[std::min(bandCount-1,j+1)])*.25f;
  ++frameCounter;float depth=effectiveDepth(amount);for(int j=0;j<bandCount;++j)lastCurve[j]=std::pow(10.f,-std::min(60.f,shape[j]*depth)/20.f);
  double before=0,after=0;for(int k=0;k<=N/2;++k){int j=bandIndex[k];float f=bandFrac[k],db=(shape[j]*(1-f)+shape[j+1]*f)*depth,applied=(k*rate/N>=20&&k*rate/N<=20000)?std::pow(10.f,-std::min(60.f,db)/20):1.f;for(int c=0;c<2;++c){double p=std::norm(B[c][k]);before+=p;after+=p*applied*applied;B[c][k]*=applied;if(k>0&&k<N/2)B[c][N-k]=std::conj(B[c][k]);}}
  float frameGain=before>1e-20?(float)std::sqrt(after/before):1;for(int c=0;c<2;++c)fft.run(B[c],true);for(int i=0;i<N;++i){int dst=(outPos+i)%R;float w=window[i]*.5f;for(int c=0;c<2;++c)ola[c][dst]+=B[c][i].real()*w;gainOla[dst]+=frameGain*window[i]*w;}
 }
public:
 Engine(){for(int i=0;i<N;++i)window[i]=(float)std::sqrt(.5-.5*std::cos(6.283185307179586*i/N));reset(48000,1,0);}
 void reset(double sr,float influence,int selectedMode)noexcept{rate=std::max(1.,sr);keyFilter.reset(rate);bandDb.fill(0);lastCurve.fill(1);frameCounter=0;bypassMix=targetBypass=0;msPosition=targetMs=0;for(int k=0;k<=N/2;++k){double p=std::clamp(std::log(std::max(20.,k*rate/N)/20.)/std::log(1000.)*(bandCount-1),0.,double(bandCount-1));int j=std::min(bandCount-2,(int)p);bandIndex[k]=j;bandFrac[k]=(float)(p-j);}pos=outPos=hop=priming=0;envelope=0;fastEnv=slowEnv=eventPeak=keyShape=0;eventAge=refractory=silenceSamples=0;eventActive=false;for(auto*a:{&input,&key})for(auto&ch:*a)ch.fill(0);for(auto&ch:ola)ch.fill(0);gainOla.fill(0);ampGainDelay.fill(1);guardDelay.fill(0);amount=targetAmount=std::clamp(influence,0.f,1.5f);mode=targetMode=selectedMode?1.f:0.f;configure(influence,0,selectedMode);}
 void configure(float influence,float smoothingMs,int selectedMode,float scLow=20,float scHigh=20000,bool bypass=false,float ms=0,float durationMs=2000,float sustain=1)noexcept{keyFilter.set(scLow,scHigh);targetMs=std::clamp(ms,-1.f,1.f);targetBypass=bypass?1.f:0.f;targetAmount=std::clamp(influence,0.f,1.5f);targetMode=selectedMode?1.f:0.f;targetDurationMs=std::clamp(durationMs,20.f,2000.f);targetSustain=std::clamp(sustain,0.f,1.f);double seconds=std::max(0.f,smoothingMs)*.001;releaseSample=seconds>0?(float)std::exp(-1/(rate*seconds)):0;releaseFrame=seconds>0?(float)std::exp(-H/(rate*seconds)):0;slew=(float)std::exp(-1/(rate*.005));}
 unsigned getFrameCounter()const noexcept{return frameCounter;}void readCurve(std::array<float,32>&mid,std::array<float,32>&side)const noexcept{float wm=(float)(1-std::max(0.,msPosition)),ws=(float)(1+std::min(0.,msPosition));for(int j=0;j<32;++j){float reduction=(1-lastCurve[j])*(1-bypassMix);mid[j]=1-reduction*wm;side[j]=1-reduction*ws;}}static int latency(int selectedMode)noexcept{return selectedMode?0:N;}
 Sample process(std::array<float,2>bass,std::array<float,2>kick)noexcept{
  Sample result;for(auto&v:bass)v=clean(v);for(auto&v:kick)v=clean(v);kick=keyFilter.process(kick);float rawLevel=std::clamp(std::max(std::abs(kick[0]),std::abs(kick[1])),0.f,1.f),shape=shapeKey(rawLevel);for(auto&v:kick)v*=shape;float detector=std::clamp(std::max(std::abs(kick[0]),std::abs(kick[1])),0.f,1.f);envelope=detector>0?std::max(detector,envelope*releaseSample):0;
  amount=targetAmount+slew*(amount-targetAmount);mode=targetMode;msPosition=targetMs+slew*(msPosition-targetMs);bypassMix=targetBypass+slew*(bypassMix-targetBypass);if(std::abs(msPosition-targetMs)<1e-7f)msPosition=targetMs;if(std::abs(amount-targetAmount)<1e-7f)amount=targetAmount;if(std::abs(bypassMix-targetBypass)<1e-7f)bypassMix=targetBypass;
  float ampGain=depthGain(1-envelope,amount),delayedAmpGain=ampGainDelay[pos],delayedGuard=guardDelay[pos],guardNow=eventActive&&eventAge<(int)(rate*.012)?1.f:0.f;ampGainDelay[pos]=ampGain;guardDelay[pos]=guardNow;
  float spectralGain=std::clamp(gainOla[outPos],0.f,1.f);result.gain=mode>.5f?ampGain:std::min(spectralGain,delayedGuard>.001f?delayedAmpGain:1.f);result.gain+=bypassMix*(1-result.gain);if(priming<N){if(mode<.5f)result.gain=1;++priming;}gainOla[outPos]=0;
  for(int c=0;c<2;++c){result.dry[c]=mode>.5f?bass[c]:input[c][pos];result.key[c]=mode>.5f?kick[c]:key[c][pos];float spectralWet=ola[c][outPos],guardedWet=spectralWet+delayedGuard*(result.dry[c]*delayedAmpGain-spectralWet),wet=mode>.5f?bass[c]*ampGain:guardedWet;result.out[c]=amount==0?result.dry[c]:wet;result.out[c]+=bypassMix*(result.dry[c]-result.out[c]);ola[c][outPos]=0;input[c][pos]=bass[c];key[c][pos]=kick[c];}
  float wm=(float)(1-std::max(0.,msPosition)),ws=(float)(1+std::min(0.,msPosition)),dl=result.out[0]-result.dry[0],dr=result.out[1]-result.dry[1],dm=.5f*(dl+dr)*wm,ds=.5f*(dl-dr)*ws;if(msPosition!=0){result.out[0]=result.dry[0]+dm+ds;result.out[1]=result.dry[1]+dm-ds;}if(mode>.5f){double im=.5*(result.dry[0]+result.dry[1]),is=.5*(result.dry[0]-result.dry[1]),gm=1-(1-ampGain)*(1-bypassMix)*wm,gs=1-(1-ampGain)*(1-bypassMix)*ws,power=im*im+is*is;result.gain=power>1e-20?(float)std::sqrt((im*im*gm*gm+is*is*gs*gs)/power):1.f;}
  pos=(pos+1)%N;outPos=(outPos+1)%R;if(++hop==H){hop=0;frame();}return result;
 }
};
}
