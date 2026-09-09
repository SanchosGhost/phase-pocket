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
struct LinkwitzRiley {
    StereoSVF a{},b{};
    void clear() noexcept {a={};b={};}
    double tick(double x,double g,bool high) noexcept {return b.tick(a.tick(x,g,high),g,high);}
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
/*
    Linkwitz-Riley 4th order three way splitter.
    band = the selected processing range, rest = everything outside it.
    The lower branch runs through the allpass complement of the upper crossover so
    band + rest stays magnitude flat: no comb filtering, no level jumps.
    blend() reports whether the splitter is engaged at all; it stays at zero while
    both handles sit at the extremes so the plugin keeps its plain wideband path.
*/
class CrossoverSplitter {
    std::array<LinkwitzRiley,2> lowLp{},lowHp{},bandLp{},topHp{},alignLp{},alignHp{};
    double sr=48000,lowLog=std::log(20.),highLog=std::log(20000.),targetLow=20,targetHigh=20000;
    double lowG=0,highG=0,step=0,mix=0,targetMix=0,mixStep=0;int count=0;
public:
    void reset(double rate) noexcept {
        sr=std::max(100.,rate);
        for(size_t c=0;c<2;++c){lowLp[c].clear();lowHp[c].clear();bandLp[c].clear();topHp[c].clear();alignLp[c].clear();alignHp[c].clear();}
        targetLow=20;targetHigh=20000;lowLog=std::log(20.);highLog=std::log(20000.);
        mix=targetMix=0;count=0;step=std::exp(-16/(sr*.01));mixStep=std::exp(-16/(sr*.02));update();
    }
    void set(float low,float high) noexcept {
        targetLow=std::clamp(double(std::min(low,high)),20.,20000.);
        targetHigh=std::clamp(double(std::max(low,high)),20.,20000.);
        targetMix=(targetLow>20.5||targetHigh<19500.)?1.:0.;
    }
    bool engaged() const noexcept {return mix>0||targetMix>0;}
    float blend() const noexcept {return float(mix);}
    void update() noexcept {
        lowLog=std::log(targetLow)+step*(lowLog-std::log(targetLow));
        highLog=std::log(targetHigh)+step*(highLog-std::log(targetHigh));
        mix=targetMix+mixStep*(mix-targetMix);if(std::abs(mix-targetMix)<1e-6)mix=targetMix;
        lowG=std::tan(3.141592653589793*std::min(std::exp(lowLog),sr*.45)/sr);
        highG=std::tan(3.141592653589793*std::min(std::exp(highLog),sr*.45)/sr);
    }
    std::array<float,2> process(std::array<float,2> x,std::array<float,2>& rest) noexcept {
        if(count++==0)update();count%=16;
        std::array<float,2> band{};
        for(size_t c=0;c<2;++c){
            const double in=x[c];
            const double below=lowLp[c].tick(in,lowG,false),above=lowHp[c].tick(in,lowG,true);
            const double centre=bandLp[c].tick(above,highG,false),top=topHp[c].tick(above,highG,true);
            const double aligned=alignLp[c].tick(below,highG,false)+alignHp[c].tick(below,highG,true);
            band[c]=float(centre);rest[c]=float(aligned+top);
        }
        return band;
    }
};
inline float effectiveDepth(float amount) noexcept {amount=std::clamp(amount,0.f,1.5f);return amount<=1?amount:std::exp2(6.f*(amount-1.f));}
inline float depthGain(float baseGain,float influence) noexcept {return std::clamp(1.f-effectiveDepth(influence)*(1.f-baseGain),0.f,1.f);}
}
