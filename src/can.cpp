#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <MidiParser/Parser.hpp>
#include <stdexcept>

#include "can/MidiViewer.hpp"
#include "can/SDLptr.hpp"

class App {
 private:
  void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error(SDL_GetError());
    }
    SDL_GetCurrentDisplayMode(0, &m_mode);
  }

  SDL_DisplayMode m_mode;
  std::unique_ptr<Can::Viewer> viewer;
  int width_, height_;

 public:
  App(std::string fileToOpen) {
    initSDL();

    width_ = static_cast<int>(m_mode.w * 0.38);
    height_ = static_cast<int>(m_mode.h * 0.38);
    // If MIDI File
    viewer = std::make_unique<Can::Viewers::MidiViewer>(
        Can::Viewers::MidiViewer(fileToOpen, width_, height_));
  }

  ~App() { SDL_Quit(); }

  void run() {
    Can::SDLptr::Window w(
        SDL_CreateWindow("can", 0, 0, width_, height_, SDL_WINDOW_UTILITY),
        SDL_DestroyWindow);

    Can::SDLptr::Renderer r(
        SDL_CreateRenderer(w.get(), -1, SDL_RENDERER_ACCELERATED),
        SDL_DestroyRenderer);

    SDL_Event e;
    bool shouldQuit = false;
    while (!shouldQuit) {
      viewer->update();
      viewer->render(r.get());
      SDL_RenderPresent(r.get());
      SDL_PollEvent(&e);
      switch (e.type) {
        case SDL_MOUSEWHEEL:
          viewer->onMouseWheel(e);
          break;
        case SDL_MOUSEBUTTONDOWN:
          viewer->onMouseDown(e);
          break;
        case SDL_KEYDOWN: {
          if (e.key.keysym.sym == SDL_KeyCode::SDLK_ESCAPE ||
              e.key.keysym.sym == SDL_KeyCode::SDLK_q) {
            shouldQuit = true;
            break;
          }
        }
        case SDL_QUIT:
          shouldQuit = true;
          break;
      }
    }
  }
};

int main(int argc, char* argv[]) {
  App app(argv[1]);
  app.run();
}
