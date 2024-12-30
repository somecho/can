#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <format>
#include <iostream>
#include <stdexcept>

#include "can/MidiViewer.hpp"
#include "can/SDLptr.hpp"

#ifdef DEBUG
#include "arial.h"
#endif

class App {
 private:
  void initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      throw std::runtime_error(SDL_GetError());
    }

#ifdef DEBUG
    if (!TTF_Init()) {
      throw std::runtime_error("Unable to initialize sdl ttf");
    }
#endif
  }

  std::unique_ptr<Can::Viewer> viewer;
  int width_, height_;
  bool shouldQuit_ = false;
  uint64_t prevTime_ = 0;
  float fps_ = 0.f;

 public:
  App(std::string fileToOpen) {
    initSDL();

    const SDL_DisplayMode* m_mode =
        SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
    if (!m_mode) {
      throw std::runtime_error(SDL_GetError());
    }
    width_ = static_cast<int>(m_mode->w * 0.38);
    height_ = static_cast<int>(m_mode->h * 0.38);
    // If MIDI File
    viewer = std::make_unique<Can::Viewers::MidiViewer>(
        Can::Viewers::MidiViewer(fileToOpen, width_, height_));
#ifdef DEBUG
    std::cout << "Startup time: " << SDL_GetTicks() << "ms" << std::endl;
#endif
  }

  ~App() { SDL_Quit(); }

  void drawFps(SDL_Renderer* r, TTF_Font* font) {
    uint64_t interval = 10;
    if (viewer->frameNum % interval == 0) {
      uint64_t currT = SDL_GetTicks();
      fps_ = 1000.f / static_cast<float>(currT - prevTime_) *
             static_cast<float>(interval);
      prevTime_ = currT;
    }
    std::string txt = std::format("{:.2f}", fps_);
    auto textSurface =
        TTF_RenderText_Solid(font, txt.data(), txt.size(),
                             SDL_Color{.r = 255, .g = 255, .b = 255, .a = 255});
    auto tex = SDL_CreateTextureFromSurface(r, textSurface);
    auto textRect = SDL_FRect{.w = static_cast<float>(tex->w),
                              .h = static_cast<float>(tex->h)};
    SDL_RenderTexture(r, tex, nullptr, &textRect);
    SDL_DestroyTexture(tex);
    SDL_DestroySurface(textSurface);
  }

  void run() {
    Can::SDLptr::Window w(
        SDL_CreateWindow("can", width_, height_, SDL_WINDOW_UTILITY),
        SDL_DestroyWindow);

    Can::SDLptr::Renderer r(SDL_CreateRenderer(w.get(), nullptr),
                            SDL_DestroyRenderer);

    SDL_Event e;

#ifdef DEBUG
    auto fontstream = SDL_IOFromMem(
        &arialFontData, sizeof(arialFontData) / sizeof(unsigned char));
    auto font = TTF_OpenFontIO(fontstream, true, 12);
    if (!font) {
      throw std::runtime_error(SDL_GetError());
    }
#endif

    auto t = std::thread([this]() {
      while (!shouldQuit_) {
        uint64_t start = SDL_GetTicks();
        viewer->update();
        int64_t end = static_cast<int64_t>(SDL_GetTicks() - start);
        int64_t rem = 9 - end;
        if (rem > 0) {
          SDL_Delay(static_cast<uint32_t>(rem));
        }
      }
    });

    while (!shouldQuit_) {
      uint64_t start = SDL_GetTicks();
      SDL_PollEvent(&e);
      handleEvent(e);
      viewer->render(r.get());

#ifdef DEBUG
      drawFps(r.get(), font);
#endif

      if (!SDL_RenderPresent(r.get())) {
        throw std::runtime_error(SDL_GetError());
      };
      ++viewer->frameNum;
      int64_t end = static_cast<int64_t>(SDL_GetTicks() - start);
      int64_t rem = 9 - end;
      if (rem > 0) {
        SDL_Delay(static_cast<uint32_t>(rem));
      }
    }
    t.join();
  }

  void handleEvent(const SDL_Event& e) {
    switch (e.type) {
      case SDL_EVENT_MOUSE_WHEEL:
        viewer->onMouseWheel(e);
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        viewer->onMouseDown(e);
        break;
      case SDL_EVENT_KEY_DOWN: {
        if (e.key.key == SDLK_ESCAPE || e.key.key == SDLK_Q) {
          shouldQuit_ = true;
          break;
        } else {
          break;
        }
      }
      case SDL_EVENT_QUIT:
        shouldQuit_ = true;
        break;
    }
  }
};

int main(int argc, char* argv[]) {
  App app(argv[1]);
#ifndef PROFILE_STARTUP
  app.run();
#endif
}
