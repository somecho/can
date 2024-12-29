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
  Viewable::MIDINote leftMostNote_;
  Viewable::MIDINote rightMostNote_;

  /** 
   * The number of discrete notes from the lowest to the highest note including
   * both.e.g.The inclusive note range of an octave from C1 - C2 is 13. 
   */
  unsigned int inclusiveNoteRange_;
  uint32_t microsecondsPerQuarter_;
  float prevMouseWheel_;
  float padding_;
  float xOffset_, xOffsetMax_, xOffsetMin_;
  float mouseAccel_, mouseAccelDamping_, mouseAccelScaling_;
  float pageSize_;
  float noteHeight_;

  std::vector<SDL_FRect> gridRects_;
  std::vector<SDL_FRect> noteRects_;
  std::vector<SDL_FRect> offsetNoteRects_;

  void populateNotes();

  /**
   * Fills up `noteRects_` with drawable geometry based on data in `notes_`.
   */
  void populateNoteRects();
};

}  // namespace Viewers
}  // namespace Can
