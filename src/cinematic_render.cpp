#include "render.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>

namespace kh {
namespace {
uint32_t opacity(uint32_t c, float a) { return (c & 0xFFFFFF00u) | uint32_t(std::clamp(a, 0.f, 255.f)); }
}
void Renderer::atmosphere(const Game &g, float camera, float time, bool near) {
  const auto &m = tuning::maps[g.levelIndex];
  const auto &p = tuning::lighting;
  // World-space layers; no accumulated screen texture or previous actor image.
  float spacing = p.fogSpacing;
  float drift = time * p.fogSpeed * m.fogDrift;
  float depth = near ? .93f : .3f;
  int first = int(std::floor((camera * depth - drift) / spacing)) - 1;
  for (int i = first; i < first + 6; ++i) {
    float x = i * spacing - camera * depth + drift;
    float y = near ? 221.f : 163.f;
    y += std::sin(time * tuning::environment.fogWaveSpeed + i * 2.1f) * p.fogHeight * .12f;
    softLight(x, y, spacing, p.fogHeight * (near ? .48f : 1.f), m.fogColor,
              Uint8(m.fogDensity * (near ? 95 : 180)), false);
  }
}
void Renderer::cinematicLights(const Game &g, float camera, float time) {
  const auto &p = tuning::lighting;
  SDL_BlendMode previous; SDL_GetRenderDrawBlendMode(r, &previous);
  // Sample the scattering field in bounded cells. Each sample traces back to
  // its real source against authored occluders. Portable SDL/GXM equivalent
  // of a light-shaft shader, evaluated on CPU (no fictitious GLSL backend).
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
  for (const auto &l : lights) {
    if (!l.fixture && !l.window) continue;
    float reach = std::max(1.f, l.floor - l.y);
    for (float y = l.y + p.shaftStep; y < l.floor; y += p.shaftStep) {
      if (y + offsetY < 0 || y + offsetY > H) continue;
      float distance = y - l.y, half = 4 + distance * p.shaftSpread;
      float center = l.x + (l.window ? distance * tuning::environment.windowShaftSlant : 0);
      for (float x = center - half; x < center + half; x += p.shaftStep) {
        if (x < 0 || x > W || g.lightBlocked(l.x + camera, l.y, x + camera, y)) continue;
        float edge = std::max(0.f, 1.f - std::fabs(x - center) / half);
        float density = 1 + p.shaftNoise * std::sin(x * .15f + y * .09f - time * p.shaftSpeed);
        float radiance = p.key * l.strength * edge * density * (1 - distance / (reach * 1.7f));
        float mapped = p.exposure * radiance / (1 + p.exposure * radiance);
        rect(x, y, p.shaftStep, std::min(p.shaftStep, l.floor-y), opacity(l.color, mapped * p.shaft * tuning::environment.shaftRadiance));
      }
    }
    // Source bloom uses the same root and occlusion checks as the shaft.
    softLight(l.x, l.y, tuning::environment.sourceBloomRadius, tuning::environment.sourceBloomHeight, l.color, Uint8(p.bloom * l.strength * 110));
  }
  SDL_SetRenderDrawBlendMode(r, previous);
  // Authored contact occlusion, not screen-space AO from a nonexistent depth buffer.
  for (const auto &b : g.level().buildings) {
    if(g.levelIndex<=2 && !g.cinematicReview())continue; // Painted structural contact shadows are registered already.
    if (b.style == 7 || b.box.x + b.box.w < camera || b.box.x > camera + W) continue;
    for (float d = 0; d < p.aoWidth; d += 2) {
      float a = p.ao * 80 * (1 - d / p.aoWidth);
      rect(b.box.x-camera+d, b.box.y+8, 2, b.box.h-8, opacity(0x03090DFF,a));
      rect(b.box.x+b.box.w-camera-d-2, b.box.y+8, 2, b.box.h-8, opacity(0x03090DFF,a));
    }
  }
}
void Renderer::cinematicGrade(const Game &g) {
  const auto &m=tuning::maps[g.levelIndex];
  // Low-opacity scene grade after world rendering; the HUD is drawn afterwards.
  float ox=offsetX, oy=offsetY; offsetX=offsetY=0;
  rect(0,0,W,H,opacity(m.skyColor,tuning::lighting.grade*255));
  offsetX=ox; offsetY=oy;
}
} // namespace kh

namespace kh {
void Renderer::wallLights(const Game &g,float camera) {
  const auto &s=tuning::surface;
  SDL_BlendMode previous;SDL_GetRenderDrawBlendMode(r,&previous);
  SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_ADD);
  for(const auto &b:g.level().buildings) {
    if(b.style==7 || b.box.x+b.box.w<camera || b.box.x>camera+W)continue;
    float left=std::max(b.box.x,camera),right=std::min(b.box.x+b.box.w,camera+W);
    int material=(b.style==1||b.style==2||b.style==5)?1:0;
    const auto &m=tuning::surfaceMaterials[material];
    for(const auto &light:lights) {
      float lx=light.x+camera,radius=light.radius*s.wallRadius;
      float x0=std::max(left,lx-radius),x1=std::min(right,lx+radius);
      float y0=std::max(b.box.y+8,light.y-radius),y1=std::min(232.f,light.y+radius);
      for(float y=y0;y<y1;y+=s.sampleStep)for(float x=x0;x<x1;x+=s.sampleStep) {
        float cx=x+std::min(s.sampleStep,x1-x)*.5f,cy=y+std::min(s.sampleStep,y1-y)*.5f;
        float distance=std::hypot(cx-lx,cy-light.y)/radius;
        if(distance>=1 || g.lightBlocked(lx,light.y,cx,cy))continue;
        float response=(1-distance)*(1-distance)*light.strength*m.diffuse;
        rect(x-camera,y,std::min(s.sampleStep,x1-x),std::min(s.sampleStep,y1-y),opacity(light.color,response*s.wallIntensity));
        if(material==1)line(x-camera,y,x-camera+std::min(s.sampleStep,x1-x),y,opacity(light.color,response*m.specular*s.wallIntensity));
      }
    }
  }
  SDL_SetRenderDrawBlendMode(r,previous);
}
void Renderer::surfaceLights(const Game &g,float camera) {
  const auto &s=tuning::surface;
  SDL_BlendMode previous;SDL_GetRenderDrawBlendMode(r,&previous);SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_ADD);
  for(const auto &platform:g.level().platforms) {
    const auto &b=platform.box;if(b.x+b.w<camera||b.x>camera+W)continue;
    const auto &m=tuning::surfaceMaterials[std::clamp(platform.material,0,2)];
    float left=std::max(b.x,camera),right=std::min(b.x+b.w,camera+W);
    for(const auto &light:lights) {
      if(light.y>=b.y)continue;
      float lx=light.x+camera,radius=light.radius*s.lightReach;
      float x0=std::max(left,lx-radius),x1=std::min(right,lx+radius);
      for(float x=x0;x<x1;x+=s.sampleStep) {
        float width=std::min(s.sampleStep,x1-x),cx=x+width*.5f;
        float dx=cx-lx,dy=b.y-light.y,distance=std::hypot(dx,dy);
        if(distance>=radius || g.lightBlocked(lx,light.y,cx,b.y-1))continue;
        float facing=dy/std::max(1.f,distance);
        float energy=std::pow(1-distance/radius,2)*facing*light.strength;
        rect(x-camera,b.y,width,std::min(b.h,s.reflectionHeight),opacity(light.color,energy*m.diffuse*s.floorIntensity));
        float hotspot=std::pow(std::max(0.f,1-std::fabs(dx)/radius),1+6*(1-m.roughness));
        line(x-camera,b.y,x-camera+width,b.y,opacity(light.color,energy*hotspot*m.specular*s.specularIntensity));
      }
    }
  }
  SDL_SetRenderDrawBlendMode(r,previous);
}
} // namespace kh

namespace kh {
void Renderer::loadFoundry() {
  if(forgeTitan.texture)return;
  forgeTitan=load("forge-titan-v3.png",4,2,false);
  for(auto &baseline:forgeTitan.baselines)baseline=tuning::forgeTitan.baseline/tuning::forgeTitan.cellSize;
  foundryHall=load("foundry-hall-v2.png",1,1,false);
}
void Renderer::loadWorkshop() {
  if(workshopPlate.texture)return;
  workshopPlate=load("workshop-plate-v1.png",1,1,false);
  workshopShell=load("workshop-plate-v1.png",1,1,false,false,true);
  workshopDistance=load("workshop-harbor-distance-v1.png",1,1,false);
  loadActors();
}
void Renderer::loadActors() {
  if(workshopHero.texture)return;
  workshopHero=load("workshop-hero-v2.png",8,8,false);
  heroLocomotion=load("hero-locomotion-v3.png",8,2,false);
  guardReactions=load("guard-reactions-v3.png",4,5,false);
  for(auto &baseline:guardReactions.baselines)
    baseline=tuning::guardReactions.baseline/tuning::guardReactions.cellSize;
  // Reflection and full-pose drawing use the same authored ground plane.
  for(auto &baseline:heroLocomotion.baselines)
    baseline=tuning::heroLocomotion.baseline/tuning::heroLocomotion.cellSize;
  workshopGuard=load("workshop-guard-v2.png",8,3,false);
  workshopAim=load("workshop-aim-v2.png",4,3,false);
  workshopClimb=load("workshop-climb-v2.png",4,3,false);
}
void Renderer::releaseSceneLayers(const Game &g) {
  auto release=[](Atlas &a){if(a.texture){SDL_DestroyTexture(a.texture);a=Atlas{};}};
  // Vita keeps only the current room's cinematic backplates resident.
  // Actor atlases remain shared across transitions and keep their anchors.
  if(!g.workshopReview)for(auto *a:{&workshopPlate,&workshopShell,&workshopDistance})release(*a);
  if(!g.ironlineReview)release(ironlineTrain);
  if(!g.ironlineReview && (g.levelIndex!=2 || g.workshopReview))
    for(auto *a:{&ironlineDistance,&ironlineForest})release(*a);
  if(g.cinematicReview()){release(scenePlate);plateTheme=-1;}
  if(g.levelIndex!=3)for(auto *a:{&forgeTitan,&foundryHall})release(*a);
}
void Renderer::workshopMotion(float camera, float time) {
  const auto &room=tuning::workshop;
  const auto &motion=tuning::workshopMotion;
  const float height=room.width/room.sourceAspect;
  const float top=232-height*room.sourceFloor;
  // Coordinates refer to the approved painting, so rain cannot cross walls,
  // mullions or actors. All animation is derived from game time (pause-safe).
  int paneIndex=0;
  for(const auto &pane:tuning::workshopPanes) {
    for(int drop=0;drop<int(motion.rainPerPane);++drop) {
      float seed=drop*0.618034f+paneIndex*.371f;
      float horizontal=seed-std::floor(seed);
      float phase=std::fmod(time*motion.rainSpeed+seed,pane.h);
      float u=pane.x+horizontal*(pane.w-std::fabs(motion.rainSlant));
      float v=pane.y+phase;
      float end=std::min(pane.y+pane.h,v+motion.rainLength);
      line(u*room.width-camera,top+v*height,
           (u+std::fabs(motion.rainSlant))*room.width-camera,top+end*height,motion.rainColor);
    }
    ++paneIndex;
  }
  for(const auto &screen:tuning::workshopScreens) {
    float phase=time*motion.terminalSpeed+screen.x*13;
    float pulse=.7f+.3f*std::sin(phase);
    float x=screen.x*room.width-camera,y=top+screen.y*height;
    float w=screen.w*room.width,h=screen.h*height;
    rect(x,y,w,h,opacity(motion.terminalColor,motion.terminalAlpha*pulse));
    float scan=std::fmod(time*motion.terminalSpeed*.25f+screen.x,1.f);
    line(x,y+scan*h,x+w,y+scan*h,opacity(motion.terminalColor,motion.terminalAlpha));
  }
  // Low-contrast ripples stay below the walking surface, away from actor feet.
  for(const auto &wet:world->level().puddles) {
    float phase=std::fmod(time*motion.rippleSpeed+wet.x*.017f,1.f);
    float x=wet.x+wet.w*.57f-camera;
    ring(x,wet.y+wet.h*.55f,phase*motion.rippleRadius,phase*1.3f,
         opacity(tuning::maps[0].keyColor,(1-phase)*motion.rippleAlpha));
  }
}
void Renderer::ironlineScene(const Game &,float camera,float time) {
  if(!ironlineTrain.texture) {
    ironlineTrain=load("ironline-carriages-review-v1.png",1,1,false);
    ironlineDistance=load("ironline-mountains-review-v1.png",1,1,false);
    ironlineForest=load("ironline-forest-review-v1.png",1,1,false);
  }
  const auto &s=tuning::ironlineReview;
  auto layer=[&](const Atlas &art,float width,float y,float depth,Uint8 alpha) {
    float scroll=(camera+time*s.travelSpeed)*depth;
    int first=int(std::floor(scroll/width));
    // Alternating mirrored panels share identical edge pixels at every seam.
    for(int tile=first;tile<=first+2;++tile)
      sprite(art,0,tile*width-scroll,y,width,width*art.height/art.width,(tile&1)!=0,0,alpha);
  };
  layer(ironlineDistance,s.mountainWidth,s.mountainY,s.mountainDepth,255);
  layer(ironlineForest,s.forestWidth,s.forestY,s.forestDepth,Uint8(s.forestAlpha));
  for(int i=0;i<5;++i) {
    float x=i*150-std::fmod(time*s.fogSpeed+camera*s.forestDepth,150.f);
    softLight(x,s.fogY+std::sin(time*.2f+i)*8,140,26,s.fogColor,Uint8(s.fogAlpha),false);
  }
  float height=s.width*ironlineTrain.height/ironlineTrain.width;
  sprite(ironlineTrain,0,-camera,s.roofY-height*s.roofFraction,s.width,height);
  for(int i=0;i<int(s.windCount);++i) {
    float x=W-std::fmod(time*s.windSpeed+i*53.71f,W+25.f);
    float y=16+std::fmod(i*29.73f,s.roofY-28);
    line(x,y,x+8,y-1,s.windColor);
  }
}
void Renderer::ironlineCampaignScene(const Game &g,float camera,float time,float cameraY) {
  if(plateTheme!=2) {
    Atlas next=load("ironline-integrated-v2.png",1,1,false);
    if(scenePlate.texture)SDL_DestroyTexture(scenePlate.texture);
    scenePlate=std::move(next);plateTheme=2;
  }
  if(!ironlineDistance.texture)ironlineDistance=load("ironline-mountains-review-v1.png",1,1,false);
  if(!ironlineForest.texture)ironlineForest=load("ironline-forest-review-v1.png",1,1,false);
  const auto &travel=tuning::ironlineTravel;
  const float surfaceOffset=offsetY;
  rect(-offsetX,-offsetY,W,H,tuning::maps[2].skyColor);
  auto layer=[&](const Atlas &art,float width,float y,float depth,float gain,Uint8 alpha) {
    float scroll=(camera+time*travel.speed)*depth;
    int first=int(std::floor(scroll/width));
    // Vertical scenery parallax never changes the roof/actor contact transform.
    offsetY=surfaceOffset+cameraY*(1-depth);
    Uint8 tint=Uint8(gain*255);
    SDL_SetTextureColorMod(art.texture,tint,tint,tint);
    for(int tile=first;tile<=first+2;++tile)
      sprite(art,0,tile*width-scroll,y,width,width*art.height/art.width,(tile&1)!=0,0,alpha);
    SDL_SetTextureColorMod(art.texture,255,255,255);
  };
  layer(ironlineDistance,travel.mountainWidth,travel.mountainY,travel.mountainDepth,travel.mountainGain,255);
  layer(ironlineForest,travel.forestWidth,travel.forestY,travel.forestDepth,travel.forestGain,Uint8(travel.forestAlpha));
  for(int i=0;i<5;++i) {
    float x=i*150-std::fmod(time*travel.fogSpeed+camera*travel.forestDepth,150.f);
    softLight(x,travel.fogY+std::sin(time*.2f+i)*8,140,26,travel.fogColor,Uint8(travel.fogAlpha),false);
  }
  offsetY=surfaceOffset;
  const auto &rig=tuning::ironlineBuiltScene;
  float width=g.level().width,height=width/rig.sourceAspect;
  sprite(scenePlate,0,-camera,232-height*rig.sourceFloor,width,height);
  float roof=232+(tuning::ironlineGalleries.front().v-rig.sourceFloor)*height;
  for(int i=0;i<int(travel.windCount);++i) {
    float x=W-std::fmod(time*travel.windSpeed+i*53.71f,W+25.f);
    float y=roof-130+std::fmod(i*29.73f,110.f);
    line(x,y,x+8,y-1,travel.windColor);
  }
}
void Renderer::cinematicHarbor(const Game &g,float camera,float time,float cameraY) {
  if(g.ironlineReview){ironlineScene(g,camera,time);return;}
  if(g.levelIndex==2 && !g.workshopReview){ironlineCampaignScene(g,camera,time,cameraY);return;}
  if(g.levelIndex==3) {
    loadFoundry();const auto &hall=tuning::foundryHall;
    productionPlate(g,foundryHall,hall.sourceFloor,hall.sourceAspect,camera,time);
    for(int i=0;i<6;++i) {
      float x=175+i*260-camera;
      float rise=std::fmod(time*hall.steamSpeed+i*17.f,60.f);
      softLight(x+std::sin(time*.4f+i)*5,225-rise,16+rise*.4f,10+rise*.25f,
                hall.steamColor,Uint8(hall.steamAlpha*(1-rise/60)),false);
    }
    for(int i=0;i<int(hall.emberCount);++i) {
      float x=std::fmod(i*97.3f+std::sin(time*.7f+i)*9,hall.width)-camera;
      float y=232-std::fmod(time*hall.emberSpeed+i*31.7f,180.f);
      if(x>0 && x<W)line(x,y,x-1,y+2,hall.emberColor);
    }
    return;
  }
  if(g.workshopReview) {
    const auto &s=tuning::workshop;float h=s.width/s.sourceAspect;
    const auto &distance=tuning::workshopParallax;
    // One continuous panorama is seen through every aperture. It moves slowly
    // behind the room; no per-window repetition, texture upload or actor history.
    sprite(workshopDistance,0,distance.originX-camera*distance.depth,distance.originY,
           distance.width,distance.width*workshopDistance.height/workshopDistance.width);
    for(int i=0;i<int(distance.fogCount);++i) {
      float span=distance.fogSpacing*distance.fogCount;
      float x=std::fmod(i*distance.fogSpacing+time*distance.fogSpeed,span)-distance.fogSpacing;
      softLight(x-camera*distance.fogDepth,distance.fogY+std::sin(time*distance.fogWave+i)*distance.fogAmplitude,
                distance.fogWidth,distance.fogHeight,distance.fogColor,Uint8(distance.fogAlpha),false);
    }
    sprite(workshopShell,0,-camera,232-h*s.sourceFloor,s.width,h);
    // Perspective is confined to the painted near floor. The walk plane,
    // colliders, feet, puddle contact reflections and walls keep one transform.
    // Sampling environment-only strips never samples or duplicates an actor.
    const auto &view=tuning::workshopView;
    const float top=232-h*s.sourceFloor;
    const float start=std::max(view.depthStart,s.sourceFloor)*h;
    for(float y=start;y<h;y+=view.stripHeight) {
      float height=std::min(view.stripHeight,h-y);
      float t=(y-start)/std::max(1.f,h-start);
      float depth=1+(view.nearDepth-1)*t*t;
      int sy=std::clamp(int(y/h*workshopPlate.height),0,workshopPlate.height-1);
      int ey=std::clamp(int(std::ceil((y+height)/h*workshopPlate.height)),sy+1,workshopPlate.height);
      SDL_Rect source{0,sy,workshopPlate.width,ey-sy};
      SDL_FRect dest{-camera*depth-s.width*(depth-1)*.5f+offsetX,
                     top+y+offsetY,s.width*depth,height};
      SDL_RenderCopyF(r,workshopPlate.texture,&source,&dest);
    }
    return;
  }
  const auto &production=tuning::productionPlates[g.levelIndex];
  if(production.enabled>0) {
    if(plateTheme!=g.levelIndex) {
      static const char *names[]={"harbor-integrated-v3.png","marsh-integrated-v3.png","ironline-cinematic-v1.png",
                                  "foundry-hall-v2.png","relay-observatory-v2.png","command-hangar-v2.png"};
      Atlas next=load(names[g.levelIndex],1,1,false);
      if(scenePlate.texture)SDL_DestroyTexture(scenePlate.texture);
      scenePlate=std::move(next);plateTheme=g.levelIndex;
    }
    productionPlate(g,scenePlate,production.sourceFloor,production.sourceAspect,camera,time);
    return;
  }
  const auto &p=tuning::scenePlates[g.levelIndex];
  float start=g.levelIndex==0?tuning::harborScene.width:0.f;
  float width=g.level().width-start, height=width/p.sourceAspect;
  if(g.levelIndex==0) {
    const auto &s=tuning::harborScene;
    float h=s.width/s.sourceAspect;
    sprite(harborScene,0,-camera,232-h*s.sourceFloor,s.width,h);
    sprite(harborExterior,0,start-camera,232-height*p.sourceFloor,width,height);
    // A physical dark warehouse jamb masks the interior/exterior seam.
    rect(start-camera-3,-300,6,532,tuning::maps[0].skyColor);
  } else {
    if(plateTheme!=g.levelIndex) {
      static const char *names[]={"harbor", "marsh", "ironline", "foundry", "relay", "command"};
      Atlas next=load(std::string(names[g.levelIndex])+"-cinematic-v1.png",1,1,false);
      if(scenePlate.texture)SDL_DestroyTexture(scenePlate.texture);
      scenePlate=std::move(next);plateTheme=g.levelIndex;
    }
    sprite(scenePlate,0,-camera,232-height*p.sourceFloor,width,height);
  }
}
void Renderer::productionPlate(const Game &g,const Atlas &art,float floor,float aspect,float camera,float time) {
  const float width=g.level().width,height=width/aspect,top=232-height*floor;
  sprite(art,0,-camera,top,width,height);
  if(g.levelIndex==1) {
    // Flow only inside measured spillways, below the gallery and behind actors.
    // The phase uses scene time, so pause and deterministic replay stay aligned.
    for(const auto &fall:tuning::marshCascades)for(int i=0;i<int(fall.count);++i) {
      float seed=std::fmod(i*.618034f,.999f);
      float u=fall.u0+seed*(fall.u1-fall.u0);
      float v=fall.v0+std::fmod(time*fall.speed+i*.137f,fall.v1-fall.v0);
      float x=u*width-camera;
      if(x>-2 && x<W+2)line(x,top+v*height,x,top+std::min(fall.v1,v+fall.length)*height,fall.color);
    }
  }
  const auto &depth=tuning::productionDepth;
  float start=std::min(.98f,floor+depth.floorMargin)*height;
  // Environment-only strip perspective starts below the contact plane.
  for(float y=start;y<height;y+=depth.stripHeight) {
    float h=std::min(depth.stripHeight,height-y),t=(y-start)/std::max(1.f,height-start);
    float z=1+(depth.nearDepth-1)*t*t;
    int sy=std::clamp(int(y/height*art.height),0,art.height-1);
    int ey=std::clamp(int(std::ceil((y+h)/height*art.height)),sy+1,art.height);
    SDL_Rect source{0,sy,art.width,ey-sy};
    SDL_FRect dest{-camera*z-width*(z-1)*.5f+offsetX,top+y+offsetY,width*z,h};
    SDL_RenderCopyF(r,art.texture,&source,&dest);
  }
  for(const auto &w:tuning::productionWeather)if(int(w.map)==g.levelIndex) {
    for(int i=0;i<int(w.count);++i) {
      float seed=std::fmod(i*.618034f,.999f);
      float u=w.u0+seed*(w.u1-w.u0);
      float v=w.v0+std::fmod(time*w.speed+i*.137f,w.v1-w.v0);
      float x=u*width-camera,y=top+v*height;
      if(x>-8 && x<W+8)line(x,y,(u+w.slant)*width-camera,top+std::min(w.v1,v+w.length)*height,w.color);
    }
  }
  if(g.levelIndex==1) {
    const auto &mist=tuning::marshMist;
    for(int i=0;i<8;++i) {
      float u=std::fmod(i*mist.spacing+time*mist.speed,1.1f)-.05f;
      float x=u*width-camera,y=top+mist.top*height;
      if(x>-100 && x<W+100)
        softLight(x,y,width*.075f,mist.height*height,0x74979800,Uint8(mist.opacity),false);
    }
  }
}
} // namespace kh
