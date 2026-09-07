#include "audio.h"
#include "game.h"
#include "render.h"
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#ifdef KH_VITA
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#endif
using namespace kh;
struct Save {
  int unlocked = 0, best = 0;
  bool muted = false, shake = true, assist = false, fullscreen = false;
};
static Save readSave(const std::string &path) {
  Save s;
  std::ifstream f(path);
  std::string magic;
  int m = 0, sh = 1, a = 0, fs = 0;
  int u = 0, b = 0;
  if (f >> magic >> u >> b >> m >> sh >> a >> fs && magic == "KH_SAVE_1") {
    s.unlocked = std::clamp(u, 0, 5);
    s.best = std::clamp(b, 0, 99999999);
    s.muted = m == 1;
    s.shake = sh == 1;
    s.assist = a == 1;
    s.fullscreen = fs == 1;
  }
  return s;
}
static bool writeSave(const std::string &path, const ViewState &v) {
  std::string tmp = path + ".tmp";
  std::ofstream f(tmp);
  if (!f)
    return false;
  f << "KH_SAVE_1 " << v.unlocked << ' ' << v.best << ' ' << v.muted << ' ' << v.shake << ' '
    << v.assist << ' ' << v.fullscreen << '\n';
  f.close();
  if (!f)
    return false;
  return std::rename(tmp.c_str(), path.c_str()) == 0;
}
struct Buttons {
  Input input;
  bool confirm = false, back = false, pause = false, left = false, right = false, up = false,
       down = false;
};
static bool newly(bool now, bool prev) { return now && !prev; }
int main(int argc, char **argv) {
  std::string assetPath, savePath, capture, recordDir, initialScreen;
  int frameLimit = 0, stage = -1, showcase = 0, recordEvery = 1, previewPhase = 1;
  bool demo = false, fast = false, bossPreview = false;
  float beginX = 40;
  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    auto next = [&]() { return i + 1 < argc ? std::string(argv[++i]) : std::string(); };
    if (a == "--record")
      recordDir = next();
    else if (a == "--record-every")
      recordEvery = std::max(1, std::stoi(next()));
    else if (a == "--boss-preview")
      bossPreview = true;
    else if (a == "--preview-phase")
      previewPhase = std::clamp(std::stoi(next()), 1, 2);
    else if (a == "--screen")
      initialScreen = next();
    else if (a == "--assets")
      assetPath = next();
    else if (a == "--save")
      savePath = next();
    else if (a == "--capture")
      capture = next();
    else if (a == "--frames")
      frameLimit = std::stoi(next());
    else if (a == "--stage")
      stage = std::stoi(next()) - 1;
    else if (a == "--demo")
      demo = true;
    else if (a == "--fast")
      fast = true;
    else if (a == "--showcase")
      showcase = std::stoi(next());
    else if (a == "--start-x")
      beginX = std::stof(next());
    else if (a == "--help") {
      std::cout << "Iron Coast: Scrap Tide --stage 1..6 --frames N --capture frame.png --demo --fast "
                   "--assets PATH --save PATH --showcase 1..5 --boss-preview --preview-phase 1..2 "
                   "--record DIR --record-every N\n";
      return 0;
    }
  }
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) != 0) {
    std::cerr << SDL_GetError() << '\n';
    return 1;
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#ifdef KH_VITA
  sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
  if (assetPath.empty())
    assetPath = "app0:/assets";
  sceIoMkdir("ux0:data/KiyiHurdasi", 0777);
  if (savePath.empty())
    savePath = "ux0:data/KiyiHurdasi/save.dat";
#else
  if (assetPath.empty()) {
    char *base = SDL_GetBasePath();
    std::string dir = base ? base : "./";
    SDL_free(base);
    std::ifstream test(dir + "assets/hero.png");
    if (test)
      assetPath = dir + "assets";
    else {
      std::ifstream app(dir + "../Resources/assets/hero.png");
      assetPath = app ? dir + "../Resources/assets" : "assets";
    }
  }
  if (savePath.empty()) {
    char *pref = SDL_GetPrefPath("KiyiHurdasi", "KiyiHurdasi");
    savePath = std::string(pref ? pref : "./") + "save.dat";
    SDL_free(pref);
  }
#endif
  auto saved = readSave(savePath);
  ViewState view;
  view.unlocked = saved.unlocked;
  view.best = saved.best;
  view.muted = saved.muted;
  view.shake = saved.shake;
  view.assist = saved.assist;
  view.fullscreen = saved.fullscreen;
  SDL_Window *window = SDL_CreateWindow("Iron Coast: Scrap Tide | Operation Iron Grid", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, 960, 544,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
                                            (saved.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
  if (!window) {
    std::cerr << SDL_GetError() << '\n';
    SDL_Quit();
    return 1;
  }
  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | (fast ? 0 : SDL_RENDERER_PRESENTVSYNC));
  if (!renderer)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    std::cerr << SDL_GetError() << '\n';
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  SDL_RenderSetLogicalSize(renderer, 480, 272);
  SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
  SDL_RendererInfo rendererInfo{};
  SDL_GetRendererInfo(renderer, &rendererInfo);
  const bool hasVsync = rendererInfo.flags & SDL_RENDERER_PRESENTVSYNC;
  int exitCode = 0;
  try {
    Renderer graphics(renderer, assetPath);
    Audio audio;
    Game game;
    game.load(0);
    bool continuing = false;
    if (stage >= 0) {
      view.selected = std::clamp(stage, 0, 5);
      view.screen = Screen::Play;
      game.load(view.selected, false, beginX);
    }
    if (demo) {
      view.assist = true;
      game.debugInvincible = true;
    }
    if (showcase) {
      view.screen = Screen::Play;
      game.load(stage >= 0 ? std::clamp(stage, 0, 5) : 0);
      game.debugInvincible = true;
      view.assist = true;
      game.player.inv = 0;
      if (showcase == 1) {
        game.player.x = 520;
        game.player.y = 232;
        game.camera = 365;
        game.player.weapon = 1;
        game.player.ammo = 180;
      } else if (showcase == 2) {
        game.player.x = game.level().width - 390;
        game.player.y = 232;
        game.camera = game.level().width - W;
        game.boss.active = true;
        game.boss.timer = .2f;
        game.player.weapon = 3;
        game.player.ammo = 30;
      } else if (showcase == 3) {
        game.player.x = game.vehicleX;
        game.player.y = 232;
        game.player.vehicleHP = 3;
        game.vehicleAvailable = false;
        game.camera = game.player.x - 150;
      } else {
        game.player.x = 500;
        game.camera = 345;
        game.player.y = showcase == 5 ? 150 : 232;
      }
    }
    if (bossPreview) {
      int index = std::clamp(stage, 0, 5);
      game.load(index, false, campaign()[index].width - 395);
      game.boss.active = true;
      game.camera = game.level().width - W;
      game.enemies.clear();
      game.props.clear();
      game.player.inv = 0;
      game.player.weapon = 1;
      game.player.ammo = 999;
      game.debugInvincible = true;
      if (previewPhase == 2)
        game.boss.hp = game.boss.maxhp * .4f;
      view.screen = Screen::Play;
      view.assist = true;
    }
    game.syncPresentation();
    if (initialScreen == "map")
      view.screen = Screen::Map;
    else if (initialScreen == "brief")
      view.screen = Screen::Brief;
    else if (initialScreen == "controls")
      view.screen = Screen::Controls;
    SDL_GameController *pad = nullptr;
    auto openPad = [&]() {
      if (pad)
        return;
      for (int i = 0; i < SDL_NumJoysticks(); i++)
        if (SDL_IsGameController(i)) {
          pad = SDL_GameControllerOpen(i);
          if (pad)
            break;
        }
    };
    openPad();
    Buttons previous{};
    Input queued{};
    bool running = true;
    int frames = 0;
    double accumulator = 0;
    uint64_t last = SDL_GetPerformanceCounter();
    float footTimer = 0;
    while (running) {
      uint64_t now = SDL_GetPerformanceCounter();
      double delta = std::min(.2, double(now - last) / SDL_GetPerformanceFrequency());
      last = now;
      if (fast)
        delta = DT;
      view.clock += float(delta);
      bool quitEvent = false, jumpPress = false, grenadePress = false, interactPress = false;
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
          quitEvent = true;
        if (event.type == SDL_CONTROLLERDEVICEADDED)
          openPad();
        if (event.type == SDL_CONTROLLERDEVICEREMOVED && pad &&
            !SDL_GameControllerGetAttached(pad)) {
          SDL_GameControllerClose(pad);
          pad = nullptr;
          if (view.screen == Screen::Play)
            view.screen = Screen::Pause;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST &&
            !fast) {
          if (view.screen == Screen::Play)
            view.screen = Screen::Pause;
          accumulator = 0;
          previous = {};
        }
        if (event.type == SDL_KEYDOWN && !event.key.repeat) {
          auto key = event.key.keysym.sym;
          jumpPress |= key == SDLK_z || key == SDLK_SPACE;
          grenadePress |= key == SDLK_c || key == SDLK_k;
          interactPress |= key == SDLK_e;
          if (event.key.keysym.sym == SDLK_m) {
            view.muted = !view.muted;
            writeSave(savePath, view);
          }
          if (event.key.keysym.sym == SDLK_F12)
            graphics.screenshot("capture.png");
        }
      }
      if (quitEvent)
        running = false;
      const Uint8 *k = SDL_GetKeyboardState(nullptr);
      Buttons buttons;
      buttons.left = k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A];
      buttons.right = k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D];
      buttons.up = k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_W];
      buttons.down = k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S];
      buttons.input.jump = k[SDL_SCANCODE_Z] || k[SDL_SCANCODE_SPACE];
      buttons.input.shoot = k[SDL_SCANCODE_X] || k[SDL_SCANCODE_J];
      buttons.input.grenade = k[SDL_SCANCODE_C] || k[SDL_SCANCODE_K];
      buttons.input.interact = k[SDL_SCANCODE_E];
      buttons.confirm = k[SDL_SCANCODE_RETURN] || k[SDL_SCANCODE_Z];
      buttons.back = k[SDL_SCANCODE_ESCAPE];
      buttons.pause = k[SDL_SCANCODE_ESCAPE] || k[SDL_SCANCODE_P];
      if (pad) {
        auto b = [&](SDL_GameControllerButton x) {
          return SDL_GameControllerGetButton(pad, x) != 0;
        };
        int axis = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX),
            ay = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);
        buttons.left |= axis < -11000 || b(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        buttons.right |= axis > 11000 || b(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        buttons.up |= ay < -11000 || b(SDL_CONTROLLER_BUTTON_DPAD_UP);
        buttons.down |= ay > 11000 || b(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        buttons.input.jump |= b(SDL_CONTROLLER_BUTTON_A);
        buttons.input.shoot |= b(SDL_CONTROLLER_BUTTON_X);
        buttons.input.grenade |=
            b(SDL_CONTROLLER_BUTTON_B) || b(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        buttons.input.interact |= b(SDL_CONTROLLER_BUTTON_Y);
        buttons.confirm |= b(SDL_CONTROLLER_BUTTON_A) || b(SDL_CONTROLLER_BUTTON_START);
        buttons.back |= b(SDL_CONTROLLER_BUTTON_B);
        buttons.pause |= b(SDL_CONTROLLER_BUTTON_START);
      }
#ifdef KH_VITA
      SceCtrlData vp{};
      sceCtrlPeekBufferPositive(0, &vp, 1);
      auto vb = [&](uint32_t x) { return (vp.buttons & x) != 0; };
      buttons.left |= vb(SCE_CTRL_LEFT) || vp.lx < 80;
      buttons.right |= vb(SCE_CTRL_RIGHT) || vp.lx > 175;
      buttons.up |= vb(SCE_CTRL_UP) || vp.ly < 80;
      buttons.down |= vb(SCE_CTRL_DOWN) || vp.ly > 175;
      buttons.input.jump |= vb(SCE_CTRL_CROSS);
      buttons.input.shoot |= vb(SCE_CTRL_SQUARE);
      buttons.input.grenade |= vb(SCE_CTRL_CIRCLE) || vb(SCE_CTRL_RTRIGGER);
      buttons.input.interact |= vb(SCE_CTRL_TRIANGLE);
      buttons.confirm |= vb(SCE_CTRL_CROSS);
      buttons.back |= vb(SCE_CTRL_CIRCLE);
      buttons.pause |= vb(SCE_CTRL_START);
#endif
      bool confirm = newly(buttons.confirm, previous.confirm),
           back = newly(buttons.back, previous.back), pause = newly(buttons.pause, previous.pause),
           left = newly(buttons.left, previous.left), right = newly(buttons.right, previous.right),
           up = newly(buttons.up, previous.up), down = newly(buttons.down, previous.down);
      buttons.input.move = float(buttons.right) - float(buttons.left);
      buttons.input.up = buttons.up;
      buttons.input.down = buttons.down;
      Input input = buttons.input;
      input.jump = jumpPress || newly(input.jump, previous.input.jump);
      input.grenade = grenadePress || newly(input.grenade, previous.input.grenade);
      input.interact = interactPress || newly(input.interact, previous.input.interact);
      view.input = buttons.input;
      if (view.screen == Screen::Title) {
        if (up)
          view.menu = (view.menu + 4) % 5;
        if (down)
          view.menu = (view.menu + 1) % 5;
        if (confirm) {
          audio.play(Sound::Pickup);
          if (view.menu == 0) {
            view.selected = 0;
            continuing = false;
            view.screen = Screen::Brief;
          } else if (view.menu == 1) {
            view.selected = view.unlocked;
            view.screen = Screen::Map;
          } else if (view.menu == 2) {
            view.screen = Screen::Options;
            view.menu = 0;
          } else if (view.menu == 3)
            view.screen = Screen::Controls;
          else
            running = false;
        }
      } else if (view.screen == Screen::Map) {
        if (left)
          view.selected = (view.selected + 5) % 6;
        if (right)
          view.selected = (view.selected + 1) % 6;
        if (confirm && (view.selected <= view.unlocked || view.assist)) {
          continuing = false;
          view.screen = Screen::Brief;
        }
        if (back) {
          view.screen = Screen::Title;
          view.menu = 1;
        }
      } else if (view.screen == Screen::Brief) {
        if (confirm) {
          int lives = game.player.lives;
          game.load(view.selected, continuing);
          if (continuing)
            game.player.lives = std::min(5, lives + 1);
          continuing = false;
          game.debugInvincible = view.assist;
          view.screen = Screen::Play;
          accumulator = 0;
        }
        if (back)
          view.screen = Screen::Map;
      } else if (view.screen == Screen::Options) {
        if (up)
          view.menu = (view.menu + 4) % 5;
        if (down)
          view.menu = (view.menu + 1) % 5;
        if (confirm || left || right) {
          if (view.menu == 0)
            view.muted = !view.muted;
          if (view.menu == 1)
            view.shake = !view.shake;
          if (view.menu == 2)
            view.assist = !view.assist;
          if (view.menu == 3) {
            view.fullscreen = !view.fullscreen;
            SDL_SetWindowFullscreen(window, view.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
          }
          if (view.menu == 4) {
            view.screen = Screen::Title;
            view.menu = 2;
          }
          writeSave(savePath, view);
        }
        if (back) {
          view.screen = Screen::Title;
          view.menu = 2;
        }
      } else if (view.screen == Screen::Controls) {
        if (confirm || back) {
          view.screen = Screen::Title;
          view.menu = 3;
        }
      } else if (view.screen == Screen::Pause) {
        if (confirm || (pause && !back)) {
          view.screen = Screen::Play;
          accumulator = 0;
        } else if (back) {
          view.screen = Screen::Title;
          view.menu = 0;
        }
      } else if (view.screen == Screen::Debrief) {
        if (confirm) {
          if (game.levelIndex == 5)
            view.screen = Screen::Ending;
          else {
            view.selected = game.levelIndex + 1;
            continuing = true;
            view.screen = Screen::Brief;
          }
        }
      } else if (view.screen == Screen::Ending) {
        if (confirm || back) {
          view.screen = Screen::Title;
          view.menu = 0;
        }
      } else if (view.screen == Screen::Play) {
        if (game.status == Status::GameOver) {
          if (confirm) {
            game.retry();
            game.debugInvincible = view.assist;
          } else if (back) {
            view.screen = Screen::Title;
            view.menu = 0;
          }
        } else if (pause) {
          view.screen = Screen::Pause;
          accumulator = 0;
        } else {
          if (demo) {
            input = {};
            input.shoot = true;
            input.move = game.boss.active ? 0 : 1;
            float look = game.floorAt(game.player.x + 48, game.player.y - 2);
            input.jump =
                game.player.grounded && (look > game.player.y + 18 ||
                                         std::fmod(game.time, .55f) < .05f);
            input.grenade = game.boss.active && int(game.time * 2) != int((game.time + DT) * 2);
            input.interact = game.vehicleAvailable && std::fabs(game.player.x - game.vehicleX) < 32;
            for (auto &e : game.enemies)
              if (!e.dead && e.kind == 3 && std::fabs(e.x - game.player.x) < 65)
                input.up = true;
          }
          if (showcase) {
            input = {};
            input.shoot = true;
            if (showcase == 1)
              input.move = .3f;
            if (showcase == 4)
              input.up = true;
            if (showcase == 5)
              input.down = true;
          }
          view.input = input;
          queued.jump |= input.jump;
          queued.grenade |= input.grenade;
          queued.interact |= input.interact;
          accumulator += delta;
          bool first = true;
          while (accumulator >= DT) {
            Input tick = input;
            tick.jump = first && queued.jump;
            tick.grenade = first && queued.grenade;
            tick.interact = first && queued.interact;
            game.update(tick);
            queued = {};
            for (auto s : game.sounds)
              audio.play(s);
            accumulator -= DT;
            first = false;
          }
          if (game.player.grounded && std::fabs(game.player.vx) > 20) {
            footTimer += float(delta);
            if (footTimer > .21f) {
              audio.play(Sound::Step);
              footTimer = 0;
            }
          }
          if (game.status == Status::Clear) {
            if (!view.assist) {
              view.unlocked = std::max(view.unlocked, std::min(5, game.levelIndex + 1));
              view.best = std::max(view.best, game.score);
              writeSave(savePath, view);
            }
            view.screen = Screen::Debrief;
          }
        }
      }
      audio.settings(game.levelIndex, view.muted, view.screen == Screen::Pause,
                     game.boss.active && !game.boss.dead);
      view.interpolation = fast || view.screen != Screen::Play || game.status == Status::GameOver
                               ? 1.0f
                               : float(std::clamp(accumulator / double(DT), 0.0, 1.0));
      graphics.render(game, view);
      frames++;
      if (!recordDir.empty() && frames % recordEvery == 0) {
        char filename[48];
        std::snprintf(filename, sizeof(filename), "/frame%05d.png", frames / recordEvery);
        graphics.screenshot(recordDir + filename);
      }
      if (!capture.empty() && frameLimit > 0 && frames >= frameLimit)
        graphics.screenshot(capture);
      SDL_RenderPresent(renderer);
      previous = buttons;
      if (frameLimit > 0 && frames >= frameLimit)
        running = false;
      // Present-vsync already paces the accelerated renderer. Sleeping after
      // every present can miss the next refresh on a busy desktop/Vita frame.
      if (!fast && !hasVsync)
        SDL_Delay(1);
    }
    if (pad)
      SDL_GameControllerClose(pad);
    std::cout << "session frames=" << frames << " stage=" << game.levelIndex + 1
              << " x=" << game.player.x << " score=" << game.score << " status=" << int(game.status)
              << " boss_hp=" << game.boss.hp << " lives=" << game.player.lives << "\n";
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Iron Coast: Scrap Tide", e.what(), window);
    exitCode = 1;
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return exitCode;
}
