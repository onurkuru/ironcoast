#include "render.h"
#include "animation.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace kh;

namespace kh {
struct RendererAudit {
  static void infantryState(const Renderer &renderer) {
    for(const Atlas *atlas:{&renderer.workshopGuard,&renderer.guardReactions,&renderer.guardShield}) {
      if(!atlas->texture)throw std::runtime_error("Infantry atlas missing after actor render");
      SDL_BlendMode blend;Uint8 red,green,blue,alpha;
      SDL_GetTextureBlendMode(atlas->texture,&blend);
      SDL_GetTextureColorMod(atlas->texture,&red,&green,&blue);
      SDL_GetTextureAlphaMod(atlas->texture,&alpha);
      if(blend!=SDL_BLENDMODE_BLEND || red!=255 || green!=255 || blue!=255 || alpha!=255)
        throw std::runtime_error("Infantry role/visibility pass leaked blend, tint or alpha to next actor");
    }
  }
  static void railBackdrop(Renderer &renderer,const Game &game,float time) {
    renderer.offsetX=renderer.offsetY=0;renderer.ironlineScene(game,0,time);
  }
  static void campaignRailBackdrop(Renderer &renderer,const Game &game,float time,float cameraY=0) {
    renderer.releaseSceneLayers(game);
    renderer.offsetX=0;renderer.offsetY=-cameraY;
    renderer.ironlineCampaignScene(game,0,time,cameraY);
    renderer.offsetY=0;
  }
  static void depthPlate(Renderer &renderer,const Game &game,float camera,float time=0) {
    renderer.loadWorkshop();
    renderer.offsetY=-26; // same workshop camera lift as drawGame
    renderer.cinematicHarbor(game,camera,time);
    renderer.offsetY=0;
  }
  static std::vector<std::pair<std::string, const Atlas *>> atlases(const Renderer &r) {
    std::vector<std::pair<std::string, const Atlas *>> result = {
        {"hero", &r.hero}, {"climb", &r.climb}, {"enemies", &r.enemies}, {"aim", &r.aim}, {"vehicle", &r.vehicle}};
    for (int i = 0; i < 6; ++i) result.push_back({"boss" + std::to_string(i), &r.bosses[i]});
    return result;
  }
  static void surfaceResponse(Renderer &renderer) {
    auto energy=[&](int x,int y) {
      Uint8 pixel[4]{};SDL_Rect area{x*2,y*2,1,1};
      if(SDL_RenderReadPixels(renderer.r,&area,SDL_PIXELFORMAT_RGBA32,pixel,4))throw std::runtime_error(SDL_GetError());
      return int(pixel[0])+int(pixel[1])+int(pixel[2]);
    };
    auto clear=[&](){SDL_SetRenderDrawColor(renderer.r,0,0,0,255);SDL_RenderClear(renderer.r);};
    int responses[3]{};
    for(int theme=0;theme<3;++theme) {
      Game g;g.load(theme);clear();renderer.lights={{80,155,232,130,1,0xFFD09000,true}};
      renderer.surfaceLights(g,0);responses[theme]=energy(80,234);
      if(responses[theme]<=0)throw std::runtime_error("A floor material does not receive light");
    }
    if(responses[0]==responses[1] && responses[1]==responses[2])throw std::runtime_error("Material light response is identical");
    Game g;g.load(0);clear();
    const auto &roof=g.level().platforms.at(1).box;
    float coveredX=roof.x+roof.w*.5f;
    renderer.lights={{coveredX,roof.y-30,232,400,1,0xFFD09000,true}};
    renderer.surfaceLights(g,0);
    if(energy(int(coveredX),234)!=0)throw std::runtime_error("Lamp light leaked through a solid roof");
    clear();renderer.lights={{365,185,232,100,1,0xFFD09000,true}};renderer.wallLights(g,0);
    if(energy(365,210)<=0)throw std::runtime_error("Wall failed to receive local light");
  }
  static void reflect(Renderer &r, const Game &g, bool flip) {
    const auto &wet = g.level().puddles.front();
    r.reflection(g, r.hero, 8, wet.x + 5, wet.y, 48, flip, 0, 0);
  }
  static void lights(Renderer &r, const Game &g, const Input &input, float camera, float alpha) {
    r.collectLights(g, input, camera, g.time, alpha);
    auto p = g.player;
    p.x = between(p.prevX, p.x, alpha);
    p.y = between(p.prevY, p.y, alpha);
    const auto muzzle = muzzlePoint(p, input);
    if (p.recoil > 0 && p.action <= 0) {
      auto it = std::find_if(r.lights.begin(), r.lights.end(), [&](const auto &l) { return !l.fixture && std::fabs(l.x-(muzzle.x-camera))<.01f && std::fabs(l.y-muzzle.y)<.01f; });
      if (it == r.lights.end() || std::fabs(it->x - (muzzle.x - camera)) > .01f ||
          std::fabs(it->y - muzzle.y) > .01f)
        throw std::runtime_error("Muzzle light detached from the interpolated shot origin");
    }
    if(g.cinematicReview())for(const auto &source:g.enemies) {
      auto e=interpolatedEnemy(source,alpha);
      if(!guardFlashVisible(e))continue;
      auto muzzle=guardMuzzle(e);
      auto it=std::find_if(r.lights.begin(),r.lights.end(),[&](const auto &l){
        return !l.fixture && std::fabs(l.x-(muzzle.x-camera))<.001f && std::fabs(l.y-muzzle.y)<.001f;
      });
      if(it==r.lights.end())throw std::runtime_error("Enemy flash light detached from barrel pose");
    }
    for (const auto &l : r.lights)
      if (!std::isfinite(l.strength) || l.strength < 0 || l.strength > 1.1f)
        throw std::runtime_error("Light animation has an invalid strength");
    if (g.boss.active && !g.boss.dead) {
      auto core = r.bossCore(g, camera, alpha);
      const auto &l = r.lights.back();
      if (std::hypot(core.x - l.x, core.y - l.y) > .01f)
        throw std::runtime_error("Boss light detached from its authored core");
    }
  }
};
}

// Use the actual SDL renderer on a software surface. The visual checks do not
// depend on a logged-in desktop, a window, or a GPU screenshot permission.
int main(int argc, char **argv) {
  if (argc < 2)
    return 2;
  SDL_Init(0);
  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 960, 544, 32, SDL_PIXELFORMAT_RGBA32);
  SDL_Renderer *device = surface ? SDL_CreateSoftwareRenderer(surface) : nullptr;
  if (!device) {
    std::cerr << SDL_GetError() << '\n';
    return 1;
  }
  SDL_RenderSetLogicalSize(device, 480, 272);
  int result = 0;
  try {
    ViewState menu;
    menu.menu = 4;
    menu.enterPause();
    if (menu.menu != 0) throw std::runtime_error("Pause must default to resume");
    menu.pauseInput(false, true, true, false, false);
    if (menu.screen != Screen::Play) throw std::runtime_error("Escape must resume, not abandon the mission");
    menu.enterPause();
    menu.pauseInput(false, false, false, false, true);
    menu.pauseInput(true, false, false, false, false);
    if (menu.screen != Screen::Controls) throw std::runtime_error("Pause controls entry failed");
    menu.closeControls();
    if (menu.screen != Screen::Pause || menu.menu != 1)
      throw std::runtime_error("Controls must return to the paused mission");
    menu.pauseInput(true, false, true, false, false);
    if (menu.screen != Screen::Play) throw std::runtime_error("Controller Start must resume before confirming a selection");
    menu.enterPause();
    menu.pauseInput(false, false, false, true, false);
    menu.pauseInput(true, false, false, false, false);
    if (menu.screen != Screen::Title || menu.menu != 0)
      throw std::runtime_error("Explicit main menu selection failed");
    menu.screen = Screen::Controls;
    menu.closeControls();
    if (menu.screen != Screen::Title || menu.menu != 3)
      throw std::runtime_error("Title controls return context failed");
    Renderer renderer(device, argv[1]);
    auto pixels = [&]() {
      std::vector<Uint32> data(960 * 544);
      if (SDL_RenderReadPixels(device, nullptr, SDL_PIXELFORMAT_RGBA32, data.data(), 960 * 4) != 0)
        throw std::runtime_error(SDL_GetError());
      return data;
    };
    ViewState view;
    view.screen = Screen::Play;
    view.interpolation = 1;
    {
      Game room;room.load(0,false,205,true);
      RendererAudit::depthPlate(renderer,room,0);auto a=pixels();
      RendererAudit::depthPlate(renderer,room,80);auto b=pixels();
      int nearChanges=0;
      for(int x=80;x<780;++x) {
        if(a[300*960+x+160]!=b[300*960+x])
          throw std::runtime_error("Perspective moved the wall/contact plane independently");
        nearChanges+=a[530*960+x+160]!=b[530*960+x];
      }
      if(nearChanges<10)throw std::runtime_error("Near plane has no independent perspective movement");
      int distanceChanges=0;
      // Compare the same world-space glass region under an 80-unit pan.
      // The wall must match above, while the transmitted harbor must move.
      for(int x=660;x<740;++x)
        distanceChanges+=a[120*960+x+160]!=b[120*960+x];
      if(distanceChanges<10)throw std::runtime_error("Distant harbor is attached to the wall");
      RendererAudit::depthPlate(renderer,room,80,4);auto movingFog=pixels();
      if(movingFog==b)throw std::runtime_error("Window atmosphere has no independent motion");
      RendererAudit::depthPlate(renderer,room,80);auto repeated=pixels();
      if(repeated!=b)throw std::runtime_error("Window composite accumulates frame history");
    }
    // The recovered room must animate without accumulating old actor pixels.
    Game workshop;workshop.load(0,false,205,true);workshop.player.inv=0;
    workshop.player.grounded=true;workshop.syncPresentation();
    renderer.render(workshop,view);auto quiet=pixels();
    Game later=workshop;later.time=1.25f;later.syncPresentation();
    renderer.render(later,view);
    if(pixels()==quiet)throw std::runtime_error("Workshop environment is static");
    renderer.render(workshop,view);
    if(pixels()!=quiet)throw std::runtime_error("Workshop animation leaked previous frame state");
    {
      Game hit=workshop;hit.enemies.clear();
      Enemy e;e.active=true;e.x=e.prevX=325;e.y=e.prevY=232;
      e.flinch=e.flinchDuration=.34f;e.hitZone=HitZone::Head;
      hit.enemies.push_back(e);renderer.render(hit,view);auto impact=pixels();
      hit.enemies.front().flinch=.14f;renderer.render(hit,view);auto recovery=pixels();
      if(recovery==impact)throw std::runtime_error("Guard reaction does not change its rendered pose");
      auto &corpse=hit.enemies.front();corpse.dead=true;corpse.deathDuration=1.52f;corpse.death=.2f;
      renderer.render(hit,view);
      hit.enemies.front()=e;renderer.render(hit,view);
      if(pixels()!=impact)throw std::runtime_error("Faded corpse leaks alpha or pixels into next guard pose");
      auto &firing=hit.enemies.front();firing.flinch=0;firing.state=2;firing.fireAge=0;firing.prevFireAge=1;
      RendererAudit::lights(renderer,hit,{},0,.5f);
      renderer.render(hit,view);auto shot=pixels();
      firing.fireAge=firing.prevFireAge=.2f;renderer.render(hit,view);
      if(pixels()==shot)throw std::runtime_error("Enemy muzzle flash never expires");
      firing.fireAge=0;firing.prevFireAge=1;renderer.render(hit,view);
      if(pixels()!=shot)throw std::runtime_error("Enemy flash light retains frame history");
    }
    {
      Game gallery;gallery.load(0,false,40,true);gallery.enemies.clear();gallery.player.inv=0;
      Input up;up.up=true;
      for(int frame=0;frame<230;++frame)gallery.update(up);
      gallery.syncPresentation();renderer.render(gallery,view);
      if(gallery.workshopShot!=WorkshopShot::Gallery)throw std::runtime_error("Gallery capture did not reach upper framing");
      if(argc>2)renderer.screenshot(std::string(argv[2])+"/workshop-gallery-camera.png");
    }
    Game original;
    {
      Game rail;rail.load(2,false,145,false,true);
      RendererAudit::railBackdrop(renderer,rail,0);auto start=pixels();
      for(Uint32 packed:start) {
        const auto *p=reinterpret_cast<const Uint8*>(&packed);
        if(p[1]>p[0]+40 && p[1]>p[2]+40)
          throw std::runtime_error("Chroma green leaked into train composite");
      }
      RendererAudit::railBackdrop(renderer,rail,2);auto travel=pixels();
      int distantMovement=0;
      for(int x=20;x<640;++x) {
        distantMovement+=start[80*960+x]!=travel[80*960+x];
        if(start[400*960+x]!=travel[400*960+x])
          throw std::runtime_error("Train body moved with the landscape");
      }
      if(distantMovement<50)throw std::runtime_error("Train landscape is stationary while player is idle");
      RendererAudit::railBackdrop(renderer,rail,0);
      if(pixels()!=start)throw std::runtime_error("Train parallax leaves previous frame artifacts");
    }
    {
      Game rail;rail.load(2);
      RendererAudit::campaignRailBackdrop(renderer,rail,0);auto start=pixels();
      for(Uint32 packed:start) {
        const auto *p=reinterpret_cast<const Uint8*>(&packed);
        if(p[1]>p[0]+40 && p[1]>p[2]+40)
          throw std::runtime_error("Chroma green leaked into campaign freight");
      }
      RendererAudit::campaignRailBackdrop(renderer,rail,2);auto travel=pixels();
      int distantMovement=0;
      for(int x=100;x<700;++x) {
        distantMovement+=start[30*960+x]!=travel[30*960+x];
        if(start[400*960+x]!=travel[400*960+x])
          throw std::runtime_error("Campaign train moved independently of its floor");
      }
      if(distantMovement<100)throw std::runtime_error("Campaign scenery does not move while idle");
      RendererAudit::campaignRailBackdrop(renderer,rail,0,-70);auto climbed=pixels();
      for(int x=100;x<700;++x)
        if(start[160*960+x]!=climbed[300*960+x])
          throw std::runtime_error("Climb camera moves train body away from contact geometry");
      RendererAudit::campaignRailBackdrop(renderer,rail,0);
      if(pixels()!=start)throw std::runtime_error("Campaign scenery accumulates previous frames");
      Game other;other.load(0);renderer.render(other,view);
      RendererAudit::campaignRailBackdrop(renderer,rail,0);
      if(pixels()!=start)throw std::runtime_error("Rail layer reload changes campaign appearance");
    }
    original.load(0);
    original.player.inv = 0;
    original.player.grounded = true;
    original.syncPresentation();
    renderer.render(original, view);
    const auto baseline = pixels();
    Game moved = original;
    moved.player.x += 95;
    moved.player.vx = 145;
    moved.player.stride = .5f;
    moved.syncPresentation();
    renderer.render(moved, view);
    if (pixels() == baseline)
      throw std::runtime_error("Moving player did not change the rendered frame");
    renderer.render(original, view);
    if (pixels() != baseline)
      throw std::runtime_error("Previous moving sprite or lighting state leaked into the next frame");
    std::vector<Uint32> lastScene;
    for (int stage = 0; stage < 6; ++stage) {
      Game scene;
      scene.load(stage, false, 120);
      scene.player.inv = 0;
      scene.player.grounded = true;
      scene.time = 6; // unobstructed gameplay, after the controls hint
      scene.syncPresentation();
      renderer.render(scene, view);
      auto current = pixels();
      if (current == lastScene)
        throw std::runtime_error("Two missions rendered the same scene");
      lastScene = std::move(current);
      if (argc > 2)
        renderer.screenshot(std::string(argv[2]) + "/stage" + std::to_string(stage + 1) + ".png");
    }
    // Sweep the complete world, including platform edges and every checkpoint.
    // Exercise scenery, lighting, actor indexing and camera clamping beyond the
    // opening screenshots. Alternate grounded and airborne presentation.
    int worldSamples = 0;
    for (int stage = 0; stage < 6; ++stage) {
      for (float x = 12; x < campaign()[stage].width; x += 32) {
        Game scene;
        scene.load(stage, false, x);
        scene.time = 9;
        scene.player.inv = 0;
        scene.player.weapon = worldSamples % 6;
        scene.player.vx = worldSamples % 2 ? 145 : -145;
        scene.player.dir = worldSamples % 2 ? 1 : -1;
        scene.player.y = worldSamples % 2 ? 160 : scene.player.y;
        scene.player.grounded = worldSamples % 2 == 0;
        scene.player.stride = worldSamples * .19f;
        scene.syncPresentation();
        view.input = {};
        view.input.shoot = true;
        view.input.up = worldSamples % 3 == 0;
        renderer.render(scene, view);
        ++worldSamples;
      }
    }
    std::cout << worldSamples << " full-world render samples passed\n";
    view.input = {};
    // Stage texture replacement must not change a later identical harbor frame.
    renderer.render(original, view);
    if (pixels() != baseline)
      throw std::runtime_error("Stage switch retained a color, light, or texture state");

    // Draw every active actor cell through the runtime path in both facing
    // directions, checking its real screen bounds and floor contact. This
    // catches destination scaling/baseline bugs that PNG border checks cannot.
    int auditedFrames = 0;
    for (const auto &[name, atlas] : RendererAudit::atlases(renderer)) {
      float size = (name == "hero" || name == "climb") ? 48 : name == "enemies" ? 43 : name == "aim" ? 56
                       : name == "vehicle" ? 76 : name == "boss4" ? 142 : 152;
      SDL_SetTextureColorMod(atlas->texture, 255, 255, 255);
      for (int frame = 0; frame < int(atlas->cells.size()); ++frame) {
        for (bool flip : {false, true}) {
          SDL_SetRenderDrawColor(device, 0, 0, 0, 255);
          SDL_RenderClear(device);
          renderer.groundedSprite(*atlas, frame, 240 - size / 2, 230 - size, size, size, flip);
          auto image = pixels();
          int bottom = -1, left = 960, right = -1;
          for (int y = 0; y < 544; ++y)
            for (int x = 0; x < 960; ++x)
              if (image[y * 960 + x] != image[0]) {
                bottom = std::max(bottom, y);
                left = std::min(left, x);
                right = std::max(right, x);
              }
          if (bottom < 456 || bottom > 460 || left < (240 - size / 2) * 2 - 1 ||
              right > (240 + size / 2) * 2 + 1)
            throw std::runtime_error(name + ":" + std::to_string(frame) + " escaped its draw box/floor: " +
                                     std::to_string(left) + "," + std::to_string(right) + "," + std::to_string(bottom));
        }
        auditedFrames++;
      }
      // Contact sheets use the real SDL sampler, with the foot plane visible.
      if (argc > 2) {
        SDL_SetRenderDrawColor(device, 23, 35, 44, 255);
        SDL_RenderClear(device);
        float cw = 480.0f / atlas->cols, ch = 248.0f / atlas->rows;
        float reviewSize = std::min(cw - 8, ch - 9);
        for (int frame = 0; frame < int(atlas->cells.size()); ++frame) {
          float x = (frame % atlas->cols + .5f) * cw;
          float feet = 20 + (frame / atlas->cols + 1) * ch - 2;
          renderer.line(x - cw / 2 + 3, feet, x + cw / 2 - 3, feet, 0x426879FF);
          renderer.groundedSprite(*atlas, frame, x - reviewSize / 2, feet - reviewSize,
                                  reviewSize, reviewSize);
        }
        renderer.text(name + " / ALL FRAMES", 8, 5, 1, 0xF5D798FF);
        renderer.screenshot(std::string(argv[2]) + "/atlas-" + name + ".png");
      }
    }

    // Full moving climb sequences cover negative world heights, light clipping,
    // transition poses and return to the floor. Optional captures use this renderer.
    for (int stage = 0; stage < 6; ++stage) {
      Game scene;
      const auto &ladder = campaign()[stage].ladders[stage == 4 ? 4 : 0];
      scene.load(stage, false, ladder.x);
      scene.debugInvincible = true;
      for (int n = 0; n < 700; ++n) {
        Input input;
        input.up = n < 300;
        input.down = n >= 350;
        scene.update(input);
        view.input = input;
        if (n % 10 == 0) renderer.render(scene, view);
        if (argc > 3 && (stage == 0 || stage == 4) && n % 4 == 0 && n < 600) {
          renderer.render(scene, view);
          char filename[80];
          std::snprintf(filename, sizeof(filename), "/motion-%d-%04d.png", stage, n / 4);
          renderer.screenshot(std::string(argv[3]) + filename);
        }
        if (argc > 2 && n == 100) {
          renderer.render(scene, view);
          renderer.screenshot(std::string(argv[2]) + "/climbing-" + std::to_string(stage + 1) + ".png");
        }
      }
    }
    view.input = {};

    for (bool flip : {false, true}) {
      Game wetScene; wetScene.load(0);
      SDL_SetRenderDrawColor(device, 0, 0, 0, 255);
      SDL_RenderClear(device);
      RendererAudit::reflect(renderer, wetScene, flip);
      auto image = pixels();
      const auto &wet = wetScene.level().puddles.front();
      int reflected = 0;
      for (int y = 0; y < 544; ++y) for (int x = 0; x < 960; ++x)
        if (image[y*960+x] != image[0]) {
          ++reflected;
          if (x < wet.x*2 || x >= (wet.x+wet.w)*2 || y <= wet.y*2 || y >= (wet.y+wet.h)*2)
            throw std::runtime_error("Reflection escaped wet-surface mask at " + std::to_string(x) + "," + std::to_string(y) + " bounds " + std::to_string(wet.x*2) + "," + std::to_string(wet.y*2) + ".." + std::to_string((wet.x+wet.w)*2) + "," + std::to_string((wet.y+wet.h)*2));
        }
      if (!reflected) throw std::runtime_error("Missing character reflection");
    }

    // Exercise every presentation family that can expose an atlas mapping
    // regression: directional aim, airborne/down aim, vehicle damage, worker
    // rescue, enemy hurt/death and every boss state. The assertions compare
    // complete frames so a state that silently falls back to the previous
    // pose cannot pass unnoticed.
    Game actors = original;
    actors.time = 9;
    actors.player.grounded = true;
    actors.player.anim = .23f;
    actors.player.stride = .41f;
    actors.player.weapon = 2;
    view.input = Input{};
    renderer.render(actors, view);
    auto idle = pixels();
    view.input.up = true;
    view.input.shoot = true;
    actors.player.recoil = .12f;
    renderer.render(actors, view);
    auto aim = pixels();
    if (aim == idle)
      throw std::runtime_error("Upward firing pose did not change the frame");
    view.input = Input{};
    view.input.down = true;
    actors.player.grounded = false;
    actors.player.vy = 180;
    renderer.render(actors, view);
    if (pixels() == aim)
      throw std::runtime_error("Downward airborne pose did not change the frame");
    actors.player.grounded = true;
    actors.player.vehicleHP = 2;
    actors.player.hitFlash = .1f;
    view.input = Input{};
    renderer.render(actors, view);
    if (pixels() == idle)
      throw std::runtime_error("Vehicle damage pose did not change the frame");
    actors.player.vehicleHP = 0;
    actors.player.vehicleDeath = .3f;
    actors.player.vehicleDeathX = actors.player.x;
    actors.player.vehicleDeathY = actors.player.y;
    renderer.render(actors, view);
    if (pixels() == idle)
      throw std::runtime_error("Vehicle destruction strip was not rendered");

    for (int kind = 0; kind < 5; ++kind) {
      Game scene = original;
      scene.time = 9;
      scene.enemies.clear();
      Enemy e;
      e.kind = kind; e.x = e.prevX = 180; e.y = e.prevY = 232;
      e.active = true;
      scene.enemies.push_back(e);
      for (int state = 0; state < 5; ++state) {
        auto &enemy = scene.enemies.front();
        enemy.state = std::min(state, 2); enemy.hurt = state == 3 ? .1f : 0;
        enemy.dead = state == 4; enemy.death = .2f;
        renderer.render(scene, view);
        if (argc > 2 && state == 4)
          renderer.screenshot(std::string(argv[2]) + "/enemy-death-" + std::to_string(kind) + ".png");
      }
    }
    for (float age : {.1f, .3f, .6f, .9f, 1.4f}) {
      Game scene = original;
      scene.items = {{180, 215, 0, true, age}};
      renderer.render(scene, view);
      if (argc > 2)
        renderer.screenshot(std::string(argv[2]) + "/worker-" + std::to_string(int(age * 10)) + ".png");
    }

    for (int dir : {-1, 1})
      for (int aimDirection = 0; aimDirection < 3; ++aimDirection)
        for (float alpha : {0.0f, .25f, .5f, .75f, 1.0f}) {
          Game scene = original;
          scene.player.recoil = .1f; scene.player.dir = dir;
          scene.player.x = 120; scene.player.prevX = 118;
          scene.player.y = 150; scene.player.prevY = 148; scene.player.grounded = false;
          Input input; input.up = aimDirection == 1; input.down = aimDirection == 2;
          RendererAudit::lights(renderer, scene, input, 30, alpha);
        }

    for (int stage = 0; stage < 6; ++stage) {
      Game bossScene;
      bossScene.load(stage, false, 120);
      bossScene.time = 8;
      bossScene.player.inv = 0;
      bossScene.boss.active = true;
      bossScene.camera = bossScene.level().width - W;
      bossScene.boss.x = bossScene.level().width - 150;
      bossScene.boss.y = stage == 4 ? 174 : 232;
      bossScene.boss.state = BossState::Move;
      bossScene.boss.stateAge = .18f;
      bossScene.boss.duration = 1.2f;
      bossScene.boss.gait = .35f;
      bossScene.syncPresentation();
      renderer.render(bossScene, view);
      auto move = pixels();
      bossScene.boss.state = BossState::Windup;
      bossScene.boss.stateAge = .38f;
      bossScene.boss.duration = .92f;
      renderer.render(bossScene, view);
      auto windup = pixels();
      bossScene.boss.state = BossState::Attack;
      bossScene.boss.stateAge = .31f;
      bossScene.boss.duration = .82f;
      bossScene.boss.recoil = .12f;
      renderer.render(bossScene, view);
      auto attack = pixels();
      bossScene.boss.state = BossState::Recover;
      bossScene.boss.stateAge = .4f;
      bossScene.boss.duration = 1.45f;
      renderer.render(bossScene, view);
      auto recover = pixels();
      for (float alpha : {0.0f, .5f, 1.0f})
        RendererAudit::lights(renderer, bossScene, {}, bossScene.camera, alpha);
      if (move == windup || windup == attack || attack == recover)
        throw std::runtime_error("Boss animation state reused an identical pose");
      bossScene.boss.dead = true;
      bossScene.boss.death = 1.7f;
      renderer.render(bossScene, view);
      if (pixels() == recover)
        throw std::runtime_error("Boss destruction pose was not rendered");
      if (argc > 2)
        renderer.screenshot(std::string(argv[2]) + "/boss-death-" + std::to_string(stage) + ".png");
    }
    for(int stage=0;stage<6;++stage) {
      Game scene;scene.load(stage,false,900);scene.time=6;scene.player.inv=0;scene.syncPresentation();
      view.screen=Screen::Play;view.interpolation=1;view.input={};
      renderer.render(scene,view);
      if(argc>2)renderer.screenshot(std::string(argv[2])+"/cinematic-map-"+std::to_string(stage)+".png");
    }
    {
      Game lineup;lineup.load(0);lineup.enemies.clear();lineup.items.clear();lineup.props.clear();
      lineup.player.x=70;lineup.player.y=232;lineup.player.inv=0;lineup.time=8;lineup.camera=0;
      for(int kind=0;kind<3;++kind) {
        Enemy soldier;soldier.kind=kind;soldier.x=170+kind*110;soldier.y=232;
        soldier.active=true;soldier.entryAge=1;soldier.hp=soldier.maxhp=7;lineup.enemies.push_back(soldier);
      }
      lineup.syncPresentation();view.input={};renderer.render(lineup,view);
      RendererAudit::infantryState(renderer);
      if(argc>2)renderer.screenshot(std::string(argv[2])+"/adult-class-lineup.png");
      lineup.enemies[0].hurt=.05f;lineup.enemies[0].flinch=.15f;
      lineup.enemies[1].flinch=.15f;lineup.enemies[1].hitZone=HitZone::Legs;
      lineup.enemies[2].dead=true;lineup.enemies[2].death=.6f;lineup.enemies[2].deathDuration=1.2f;
      lineup.syncPresentation();renderer.render(lineup,view);
      RendererAudit::infantryState(renderer);
    }
    for(int weapon=0;weapon<6;++weapon) {
      Game scene;scene.load(0,false,450);scene.player.weapon=scene.player.firedWeapon=weapon;scene.player.inv=0;
      scene.player.fireAge=.01f;scene.player.recoil=.1f;scene.syncPresentation();view.input.shoot=true;
      renderer.render(scene,view);
      if(argc>2)renderer.screenshot(std::string(argv[2])+"/weapon-"+std::to_string(weapon)+".png");
    }
    RendererAudit::surfaceResponse(renderer);
    Game framed=original;framed.cameraZoom=framed.prevCameraZoom=1.025f;
    float sx,sy,afterX,afterY;SDL_RenderGetScale(device,&sx,&sy);
    renderer.render(framed,view);SDL_RenderGetScale(device,&afterX,&afterY);
    if(sx!=afterX || sy!=afterY)throw std::runtime_error("Cinematic framing leaked scale into HUD or next frame");
    view.input = Input{};
    view.screen = Screen::Pause;
    renderer.render(original, view);
    auto paused = pixels();
    view.clock += 100;
    renderer.render(original, view);
    if (pixels() != paused) throw std::runtime_error("Lighting advanced while paused");
    std::cout << auditedFrames << " atlas cells in both directions, six scenes, actors, lights and pause passed\n";
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    result = 1;
  }
  SDL_DestroyRenderer(device);
  SDL_FreeSurface(surface);
  SDL_Quit();
  return result;
}
