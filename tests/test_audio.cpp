#include "audio.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace kh {
struct AudioAudit {
  static std::vector<int16_t> effect(Sound sound,int weapon,float distance=0){
    Audio a(false);a.musicEnabled=false;a.play(sound,distance,weapon);
    // Mute score only: exercise the same sample mixer used by the SDL callback.
    std::vector<int16_t> output(48000);
    a.mix(output.data(),int(output.size()));
    return output;
  }
  static void silence(){Audio a(false);a.play(Sound::Shot);a.settings(0,true,false,false);int16_t data[1000];a.mix(data,1000);for(auto s:data)if(s)throw std::runtime_error("Mute leaks sound");a.settings(0,false,true,false);a.mix(data,1000);for(auto s:data)if(s)throw std::runtime_error("Pause leaks sound");}
  static void offline(){
    Audio direct(false),chunked(false);direct.musicEnabled=chunked.musicEnabled=false;
    direct.play(Sound::BodyLand);chunked.play(Sound::BodyLand);
    std::vector<int16_t> expected(32000),actual(32000);direct.mix(expected.data(),32000);
    int previous=0;for(int frame=1;frame<=60;++frame){int target=frame*32000/60;chunked.renderOffline(actual.data()+previous,target-previous);previous=target;}
    if(actual!=expected)throw std::runtime_error("Offline capture changes the mixer across video frame boundaries");
  }
};
}
double energy(const std::vector<int16_t>&v){double sum=0;for(auto s:v)sum+=double(s)*s;return sum/v.size();}
void wav(const char *path,const std::vector<int16_t>&v){std::ofstream out(path,std::ios::binary);auto n=[&](uint32_t x,int bytes){for(int i=0;i<bytes;++i)out.put(char((x>>(i*8))&255));};out.write("RIFF",4);n(36+v.size()*2,4);out.write("WAVEfmt ",8);n(16,4);n(1,2);n(1,2);n(32000,4);n(64000,4);n(2,2);n(16,2);out.write("data",4);n(v.size()*2,4);for(auto s:v)n(uint16_t(s),2);}
int main(int argc,char **argv){try{std::vector<std::vector<int16_t>> sounds;std::vector<int16_t> review;
 for(int w=0;w<6;++w){auto close=kh::AudioAudit::effect(kh::Sound(w),w);auto far=kh::AudioAudit::effect(kh::Sound(w),w,600);
 if(energy(close)<100 || energy(far)>=energy(close)*.3)throw std::runtime_error("Invalid distance attenuation or silent shot");
 for(const auto &other:sounds)if(other==close)throw std::runtime_error("Two weapons share an identical sound");sounds.push_back(close);
 for(auto type:{kh::Sound(w),kh::Sound::Reload,kh::Sound::Empty}){auto part=kh::AudioAudit::effect(type,w);if(energy(part)<1)throw std::runtime_error("Missing weapon action audio");review.insert(review.end(),part.begin(),part.end());}
 }
 auto land=kh::AudioAudit::effect(kh::Sound::BodyLand,-1),farLand=kh::AudioAudit::effect(kh::Sound::BodyLand,-1,600);
 if(energy(land)<10 || energy(farLand)>=energy(land)*.3)throw std::runtime_error("Landing thud missing or distance attenuation broken");
 if(land==kh::AudioAudit::effect(kh::Sound::Stomp,-1))throw std::runtime_error("Landing thud duplicates stomp");
 kh::AudioAudit::silence();kh::AudioAudit::offline();if(argc>1)wav(argv[1],review);
 std::cout<<"Six distinct reports, 12 reload/empty cues, distance rolloff, mute and pause passed\n";
 }catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}}
