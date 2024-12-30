#pragma once

#include <SDL3/SDL.h>
#include <MidiParser/Parser.hpp>
#include <string>
#include <vector>

#include "Viewer.hpp"

namespace Can {
namespace Viewers {

class MidiViewer : public Viewer {

 public:
  MidiViewer(std::string fileToView, int width, int height);

  void update() override;
  void render(SDL_Renderer* renderer) override;
  void onMouseWheel(const SDL_Event& event) override;
  void onMouseDown(const SDL_Event& event) override;

 private:
  struct {
    std::vector<uint8_t> key;
    std::vector<uint8_t> vel;
    std::vector<float> start;
    std::vector<float> end;
    size_t size = 0;
  } allNotes_;
  uint8_t highestKey_ = 0;
  uint8_t lowestKey_ = UINT8_MAX;
  float totalMillis_ = 0;

  // The number of discrete notes from the lowest to the highest note including
  // both. E.g. The inclusive note range of an octave from C1 - C2 is 13.
  unsigned int inclusiveNoteRange_;
  uint32_t microsecondsPerQuarter_;
  float prevMouseWheel_;
  float padding_;
  float xOffset_, xOffsetMax_, xOffsetMin_;
  float mouseAccel_, mouseAccelDamping_, mouseAccelScaling_;
  float pageSize_;
  float noteHeight_;

  // The rects representing the horizontal piano roll grid.
  std::vector<SDL_FRect> gridRects_;

  // The collection of all midi notes represented as rects.
  struct {
    std::vector<SDL_FRect> rect;
    std::vector<SDL_Color> col;
    size_t size = 0;
  } refs_;

  // Updated every time `update()` is called. Filters `referenceRects_` out
  // to only rects that are visible. Double buffered.
  std::vector<std::pair<SDL_FRect, SDL_Color>> drawnRects_[2];
  size_t currentBuffer = 0;
  std::vector<float> gridTicks_;

  void populateNotes();
  void populateNoteRects();

  // Renders `gridRects_`
  void drawPianoRoll(SDL_Renderer* renderer);

  // Renders the vertical lines representing time
  void drawTimeTicks(SDL_Renderer* renderer);

  // Renders `drawnRects_`
  void drawMIDINotes(SDL_Renderer* renderer);
};

}  // namespace Viewers
}  // namespace Can
