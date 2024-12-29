#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <string>

namespace Can {

class Viewer {
 public:
  Viewer(std::string fileToView, int width, int height)
      : fileToView_(fileToView), width_(width), height_(height) {};
  virtual ~Viewer() = default;

  virtual void update() {};
  virtual void render(SDL_Renderer* renderer) = 0;
  virtual void onMouseWheel(const SDL_Event& event) {};
  virtual void onMouseDown(const SDL_Event& event) {};

 protected:
  std::string fileToView_;
  int width_, height_;
};

}  // namespace Can
