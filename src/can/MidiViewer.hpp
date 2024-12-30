#pragma once

#include <SDL3/SDL.h>
#include <MidiParser/Parser.hpp>
#include <string>
#include <vector>

#include "Viewable.hpp"
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
  std::vector<Viewable::MIDINote> notes_;
  Viewable::MIDINote highestNote_;
  Viewable::MIDINote lowestNote_;
  Viewable::MIDINote rightMostNote_;

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
  std::vector<std::pair<SDL_FRect, SDL_Color>> referenceRects_;

  // Updated every time `update()` is called. Filters `referenceRects_` out
  // to only rects that are visible.
  std::vector<std::pair<SDL_FRect, SDL_Color>> drawnRects_;
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
