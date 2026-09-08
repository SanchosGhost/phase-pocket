#pragma once
#include <array>
#include <cmath>
#include <algorithm>
namespace pocket {
struct StereoSVF {
    double s1=0,s2=0;
    double tick(double x,double g,bool high) noexcept {
        const double a=1/(1+g*(g+1.4142135623730951));
        const double v1=a*(s1+g*(x-s2)),v2=s2+g*v1;
        s1=2*v1-s1;s2=2*v2-s2;
        return high?x-1.4142135623730951*v1-v2:v2;
    }
};
class SidechainFilter {
    std::array<StereoSVF,2> hp{},lp{};
    double sr=48000,lowLog=std::log(20.),highLog=std::log(20000.),targetLow=20,targetHigh=20000;
    double hpMix=0,lpMix=0,gh=0,gl=0,step=0;int count=0;
public:
    void reset(double rate) noexcept {sr=std::max(100.,rate);hp={};lp={};targetLow=20;targetHigh=20000;lowLog=std::log(20.);highLog=std::log(20000.);hpMix=lpMix=0;count=0;step=std::exp(-16/(sr*.01));update();}
    void set(float low,float high) noexcept {targetLow=std::clamp(double(std::min(low,high)),20.,20000.);targetHigh=std::clamp(double(std::max(low,high)),20.,20000.);}
    void update() noexcept {lowLog=std::log(targetLow)+step*(lowLog-std::log(targetLow));highLog=std::log(targetHigh)+step*(highLog-std::log(targetHigh));const double useHp=targetLow>20.01?1:0,useLp=targetHigh<19999.?1:0;hpMix=useHp+step*(hpMix-useHp);lpMix=useLp+step*(lpMix-useLp);if(std::abs(hpMix-useHp)<1e-8)hpMix=useHp;if(std::abs(lpMix-useLp)<1e-8)lpMix=useLp;gh=std::tan(3.141592653589793*std::min(std::exp(lowLog),sr*.45)/sr);gl=std::tan(3.141592653589793*std::min(std::exp(highLog),sr*.45)/sr);}
    std::array<float,2> process(std::array<float,2> x) noexcept {if(count++==0)update();count%=16;for(int c=0;c<2;++c){double v=x[size_t(c)],h=hp[size_t(c)].tick(v,gh,true);v+=hpMix*(h-v);double l=lp[size_t(c)].tick(v,gl,false);x[size_t(c)]=float(v+lpMix*(l-v));}return x;}
};
class ProcessingRangeFilter {
    std::array<StereoSVF,2> hpA{},hpB{},lpA{},lpB{};
    double sr=48000,lowLog=std::log(20.),highLog=std::log(20000.),targetLow=20,targetHigh=20000;
    double hpMix=0,lpMix=0,gh=0,gl=0,step=0;int count=0;
public:
    void reset(double rate) noexcept {sr=std::max(100.,rate);hpA={};hpB={};lpA={};lpB={};targetLow=20;targetHigh=20000;lowLog=std::log(20.);highLog=std::log(20000.);hpMix=lpMix=0;count=0;step=std::exp(-16/(sr*.01));update();}
    void set(float low,float high) noexcept {targetLow=std::clamp(double(std::min(low,high)),20.,20000.);targetHigh=std::clamp(double(std::max(low,high)),20.,20000.);}
    void update() noexcept {lowLog=std::log(targetLow)+step*(lowLog-std::log(targetLow));highLog=std::log(targetHigh)+step*(highLog-std::log(targetHigh));const double useHp=targetLow>20.01?1:0,useLp=targetHigh<19999.?1:0;hpMix=useHp+step*(hpMix-useHp);lpMix=useLp+step*(lpMix-useLp);if(std::abs(hpMix-useHp)<1e-8)hpMix=useHp;if(std::abs(lpMix-useLp)<1e-8)lpMix=useLp;gh=std::tan(3.141592653589793*std::min(std::exp(lowLog),sr*.45)/sr);gl=std::tan(3.141592653589793*std::min(std::exp(highLog),sr*.45)/sr);}
    std::array<float,2> process(std::array<float,2> x) noexcept {if(count++==0)update();count%=16;for(int c=0;c<2;++c){const double dry=x[size_t(c)],h1=hpA[size_t(c)].tick(dry,gh,true),h2=hpB[size_t(c)].tick(h1,gh,true);double band=dry+hpMix*(h2-dry);const double l1=lpA[size_t(c)].tick(band,gl,false),l2=lpB[size_t(c)].tick(l1,gl,false);band+=lpMix*(l2-band);x[size_t(c)]=float(band);}return x;}
};
inline float effectiveDepth(float amount) noexcept {amount=std::clamp(amount,0.f,1.5f);return amount<=1?amount:std::exp2(6.f*(amount-1.f));}
inline float depthGain(float baseGain,float influence) noexcept {return std::clamp(1.f-effectiveDepth(influence)*(1.f-baseGain),0.f,1.f);}
}
