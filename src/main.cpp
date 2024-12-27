#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <MidiParser/Parser.hpp>
#include <cmath>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>

struct note {
  uint8_t key;
  uint32_t left, right;
};

// https://github.com/openframeworks/openFrameworks/blob/0.12.0/libs/openFrameworks/math/ofMath.cpp#L78
float map(float value, float inputMin, float inputMax, float outputMin,
          float outputMax, bool clamp) {
  if (std::abs(inputMin - inputMax) < std::numeric_limits<float>::epsilon()) {
    return outputMin;
  } else {
    float outVal =
        ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) +
         outputMin);

    if (clamp) {
      if (outputMax < outputMin) {
        if (outVal < outputMax)
          outVal = outputMax;
        else if (outVal > outputMin)
          outVal = outputMin;
      } else {
        if (outVal > outputMax)
          outVal = outputMax;
        else if (outVal < outputMin)
          outVal = outputMin;
      }
    }
    return outVal;
  }
}

int main() {
  MidiParser::Parser parser;
  auto MIDIData = parser.parse("tmp/cmaj.mid");
  uint32_t currentTime = 0;
  std::unordered_map<uint8_t, uint32_t> keymap;
  std::vector<note> notes;
  uint8_t highestNote = 0;
  uint8_t lowestNote = UINT8_MAX;

  for (size_t i = 0; i < MIDIData.tracks.size(); i++) {
    for (const auto& e : MIDIData.tracks.at(i).events) {
      if (std::holds_alternative<MidiParser::MIDIEvent>(e)) {
        auto m = std::get<MidiParser::MIDIEvent>(e);
        if ((m.status & 0b11110000) == 0b10010000) {
          currentTime += m.deltaTime;
          auto key = m.data[0];
          auto vel = m.data[1];
          if (vel > 0) {
            keymap.insert_or_assign(key, currentTime);
          } else {
            notes.push_back(
                note{.key = key, .left = keymap.at(key), .right = currentTime});
          }
          if (key < lowestNote) {
            lowestNote = key;
          }
          if (key > highestNote) {
            highestNote = key;
          }
        }
      }
    }
  }

  std::cout << "Highest note = " << (int)highestNote << std::endl;
  std::cout << "Lowest note = " << (int)lowestNote << std::endl;
  auto left = notes.front().left;
  auto right = notes.back().right;

  {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error(SDL_GetError());
    }

    SDL_DisplayMode mode{};
    SDL_GetCurrentDisplayMode(0, &mode);
    std::cout << mode.w << std::endl;

    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> win(
        SDL_CreateWindow(
            "SDL Window", 0, 0, 600, 800,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_UTILITY | SDL_WINDOW_BORDERLESS),
        SDL_DestroyWindow);

    std::unique_ptr<SDL_Surface, std::function<void(SDL_Surface*)>> surface(
        SDL_GetWindowSurface(win.get()), SDL_FreeSurface);

    std::cout << notes.size() << std::endl;

    const auto noteHeight = (int)floor(800.0 / (highestNote - lowestNote));

    for (const auto n : notes) {
      SDL_Rect r{
          .x = static_cast<int>(map(n.left, 0.0, right, 0.0, 600.0, false)),
          .y = (int)map(n.key, lowestNote, highestNote, 800 - noteHeight, 0,
                        false),
          .w = (int)map(n.right - n.left, 0.0, right, 0.0, 600.0, false),
          .h = noteHeight};
      SDL_FillRect(surface.get(), &r,
                   SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
    }

    SDL_UpdateWindowSurface(win.get());

    SDL_Event e;
    while (true) {
      SDL_PollEvent(&e);
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDL_KeyCode::SDLK_ESCAPE ||
            e.key.keysym.sym == SDL_KeyCode::SDLK_q) {
          break;
        }
      }
      if (e.type == SDL_QUIT) {
        break;
      }
    }
  }
  SDL_Quit();
}
