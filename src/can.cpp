#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <MidiParser/Parser.hpp>
#include <algorithm>
#include <cmath>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <utility>

// https://github.com/openframeworks/openFrameworks/blob/0.12.0/libs/openFrameworks/math/ofMath.cpp#L78
float map(float value, float inputMin, float inputMax, float outputMin,
          float outputMax, bool clamp = true) {
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

template <class... Ts>
struct overload : Ts... {
  using Ts::operator()...;
};

struct QuantizedNote {
  uint8_t key;
  uint32_t timeStart, timeEnd;
};

std::vector<QuantizedNote> quantizeMIDI(const MidiParser::MidiFile& data) {
  using namespace MidiParser;

  std::unordered_map<uint8_t, uint32_t> noteOnTimes;
  std::vector<QuantizedNote> notes;

  for (size_t i = 0; i < data.tracks.size(); i++) {
    uint32_t currentTime = 0;

    auto visitMidi = [&currentTime, &notes,
                      &noteOnTimes](MIDIEvent& e) mutable {
      currentTime += e.deltaTime;
      const bool hasNoteOnStatus = (e.status & 0b11110000) == 0b10010000;
      const bool hasNoteOffStatus = (e.status & 0b11110000) == 0b10000000;

      if (!(hasNoteOnStatus || hasNoteOffStatus)) {
        return;
      }

      const uint8_t key = e.data.at(0);
      const uint8_t velocity = e.data.at(1);
      const bool isNoteOn = hasNoteOnStatus && velocity > 0;
      const bool isNoteOff =
          (hasNoteOnStatus && velocity == 0) || hasNoteOffStatus;

      if (isNoteOn) {
        noteOnTimes.insert_or_assign(e.data.at(0), currentTime);
      }
      if (isNoteOff) {
        notes.push_back({.key = key,
                         .timeStart = noteOnTimes.at(key),
                         .timeEnd = currentTime});
      }
    };

    auto visitor = overload{
        visitMidi,
        [&currentTime](MetaEvent& e) mutable { currentTime += e.deltaTime; },
        [](SysExEvent&) { /* ignore */ }};

    for (TrackEvent e : data.tracks.at(i).events) {
      std::visit(visitor, e);
    }
  }

  return notes;
}

using Window = std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>>;
using Renderer =
    std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>>;

class App {

 private:
  void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error(SDL_GetError());
    }
    SDL_GetCurrentDisplayMode(0, &m_mode);
  }

  SDL_DisplayMode m_mode;

 public:
  App() { initSDL(); }

  ~App() { SDL_Quit(); }

  void run(std::string filepath) {
    int windowWidth = m_mode.w * 0.38;
    int windowHeight = m_mode.h * 0.38;

    // PREPARE

    MidiParser::Parser parser;
    auto data = parser.parse(filepath);
    auto qNotes = quantizeMIDI(data);

    auto cmpy = [](const QuantizedNote& a, const QuantizedNote& b) {
      return a.key < b.key;
    };
    auto cmpx = [](const QuantizedNote& a, const QuantizedNote& b) {
      return a.timeStart > b.timeStart;
    };

    auto highestNote = *std::max_element(qNotes.begin(), qNotes.end(), cmpy);
    auto lowestNote = *std::min_element(qNotes.begin(), qNotes.end(), cmpy);
    auto leftMostNote = *std::max_element(qNotes.begin(), qNotes.end(), cmpx);
    auto rightMostNote = *std::min_element(qNotes.begin(), qNotes.end(), cmpx);

    uint8_t gridSizeY = highestNote.key - lowestNote.key;
    uint32_t barSize =
        static_cast<uint32_t>(data.tickDivision) * 4 * 4;  // 4 bars
    int gridHeight = windowHeight / (int)gridSizeY;
    float xStart = 0;

    // RENDER

    Window w(SDL_CreateWindow("can", 0, 0, windowWidth, windowHeight,
                              SDL_WINDOW_UTILITY),
             SDL_DestroyWindow);

    Renderer r(SDL_CreateRenderer(w.get(), -1, SDL_RENDERER_ACCELERATED),
               SDL_DestroyRenderer);

    int padding = 0;
    std::vector<SDL_Rect> rects;
    std::vector<SDL_Rect> rectsOffset;

    for (auto note : qNotes) {
      SDL_Rect r{
          .x = (int)map(note.timeStart, 0, barSize, 0, windowWidth, false),
          .y = (int)map(note.key, lowestNote.key, highestNote.key,
                        windowHeight - gridHeight, 0) +
               padding,
          .w = (int)map(note.timeEnd - note.timeStart, 0, barSize, 0,
                        windowWidth),
          .h = gridHeight - (padding * 2)};
      rects.push_back(r);
      rectsOffset.push_back(r);
    }

    SDL_Event e;
    while (true) {

      SDL_SetRenderDrawColor(r.get(), 0x0, 0x0, 0x0, 0xFF);
      SDL_RenderClear(r.get());
      SDL_SetRenderDrawColor(r.get(), 0xFF, 0xFF, 0xFF, 0xFF);
      SDL_RenderFillRects(r.get(), rectsOffset.data(), rectsOffset.size());
      SDL_RenderPresent(r.get());

      SDL_PollEvent(&e);
      if (e.type == SDL_MOUSEWHEEL) {
        xStart += e.wheel.y * 5;
        // clamp beginning
        xStart = std::min({xStart + e.wheel.y * 5, 0.0f});
        // clamp end
        xStart = std::max(
            {xStart,
             -(((float)rightMostNote.timeEnd / (float)barSize) * windowWidth -
               windowWidth)});
        // update rects to be drawn
        for (std::tuple<SDL_Rect&, SDL_Rect&> rects :
             std::views::zip(rects, rectsOffset)) {
          std::get<1>(rects).x = std::get<0>(rects).x + (int)xStart;
        }
      }
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
};

int main(int argc, char* argv[]) {
  App app;
  app.run(argv[1]);
}
