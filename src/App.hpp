#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>

#include "can/MidiViewer.hpp"

#ifdef DEBUG
#include <SDL3_ttf/SDL_ttf.h>
#endif

namespace Can {

class App {
 public:
  App(std::string fileToOpen);
  ~App();

  void run();

 private:
  void initSDL();
  void handleEvent();

  SDL_Window* w;
  SDL_Renderer* r;
  SDL_Event e;

  std::unique_ptr<Viewer> viewer;
  int width_, height_;
  bool shouldQuit_ = false;
  uint64_t prevTime_ = 0;
  float fps_ = 0.f;

#ifdef DEBUG
  void initTTF();
  void drawFps();
  TTF_Font* font;
#endif
};

}  // namespace Can
