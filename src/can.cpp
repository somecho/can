#include <SDL3/SDL.h>
#include <stdexcept>

#include "can/MidiViewer.hpp"
#include "can/SDLptr.hpp"

class App {
 private:
  void initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      throw std::runtime_error(SDL_GetError());
    }
  }

  std::unique_ptr<Can::Viewer> viewer;
  int width_, height_;
  bool shouldQuit_ = false;

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
  }

  ~App() { SDL_Quit(); }

  void run() {
    Can::SDLptr::Window w(
        SDL_CreateWindow("can", width_, height_, SDL_WINDOW_UTILITY),
        SDL_DestroyWindow);

    Can::SDLptr::Renderer r(SDL_CreateRenderer(w.get(), nullptr),
                            SDL_DestroyRenderer);

    SDL_Event e;
    while (!shouldQuit_) {
      SDL_PollEvent(&e);
      handleEvent(e);
      viewer->update();
      viewer->render(r.get());
      if (!SDL_RenderPresent(r.get())) {
        throw std::runtime_error(SDL_GetError());
      };
    }
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
  app.run();
}
