#include <algorithm>
#include <cmath>
#include <map>
#include <ranges>
#include <unordered_map>

#include "MidiViewer.hpp"
#include "helper.hpp"

template <class... Ts>
struct overload : Ts... {
  using Ts::operator()...;
};

namespace Can {
namespace Viewers {

MidiViewer::MidiViewer(std::string fileToView, int width, int height)
    : Viewer(fileToView, width, height),
      prevMouseWheel_(0.f),
      padding_(0.5f),
      xOffset_(0.f),
      xOffsetMax_(0.f),
      mouseAccel_(0.f) {
  populateNotes();

  auto cmpy = [](const Viewable::MIDINote& a, const Viewable::MIDINote& b) {
    return a.key < b.key;
  };
  auto cmpx = [](const Viewable::MIDINote& a, const Viewable::MIDINote& b) {
    return a.start > b.start;
  };

  // calculate bounds of viewer based on midi data
  highestNote_ = *std::max_element(notes_.begin(), notes_.end(), cmpy);
  lowestNote_ = *std::min_element(notes_.begin(), notes_.end(), cmpy);
  leftMostNote_ = *std::max_element(notes_.begin(), notes_.end(), cmpx);
  rightMostNote_ = *std::min_element(notes_.begin(), notes_.end(), cmpx);
  inclusiveNoteRange_ = highestNote_.key - lowestNote_.key + 1;
  noteHeight_ =
      static_cast<float>(height_) / static_cast<float>(inclusiveNoteRange_);

  // Page size
  pageSize_ = static_cast<float>(width_) * 10.f;

  // Length of MIDI Track in terms of number of bars(pages) it can fit in
  float trackLengthNormalized = rightMostNote_.end / pageSize_;

  // calculate how far the track can be scrolled
  xOffsetMin_ = -(trackLengthNormalized * static_cast<float>(width_) -
                  static_cast<float>(width_));

  // Initialize scroll physics values
  mouseAccelScaling_ = trackLengthNormalized * 0.005f;
  mouseAccelDamping_ = trackLengthNormalized / 10000.f;

  populateNoteRects();

  // generate rects for drawing grid
  for (auto i = 0u; i < inclusiveNoteRange_; i++) {
    gridRects_.push_back(
        {.x = 0,
         .y = helper::map(static_cast<float>(i), 0,
                          static_cast<float>(inclusiveNoteRange_),
                          static_cast<float>(height_), 0),
         .w = (float)width_,
         .h = noteHeight_});
  }

  // populate gridTicks_
  float interval = 1.f * 1000.f;  // 10 seconds
  unsigned int numTicks =
      static_cast<unsigned int>(std::floor(rightMostNote_.end / interval));
  numTicks = std::max(
      {numTicks, static_cast<unsigned int>(std::floor(pageSize_ / interval))});
  for (auto i = 1u; i <= numTicks; i++) {
    float x = helper::map(static_cast<float>(i) * interval, 0, pageSize_, 0,
                          static_cast<float>(width_), false);
    gridTicks_.emplace_back(x);
  }
}

void MidiViewer::update() {
  // update scroll physics
  mouseAccel_ += (0 - mouseAccel_) * mouseAccelDamping_;
  xOffset_ += mouseAccel_ * mouseAccelScaling_;
  xOffset_ = std::clamp(xOffset_, xOffsetMin_, xOffsetMax_);

  // update note rect positions with offset data
  for (std::tuple<SDL_FRect&, SDL_FRect&> rects :
       std::views::zip(noteRects_, offsetNoteRects_)) {
    std::get<1>(rects).x = std::get<0>(rects).x + xOffset_;
  }
}

void MidiViewer::render(SDL_Renderer* renderer) {
  SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
  SDL_RenderClear(renderer);

  // Draw Grid
  for (auto i = 0u; i < inclusiveNoteRange_; i++) {
    int id = (i + lowestNote_.key) % 12;
    if (id == 1 || id == 3 || id == 6 || id == 8 || id == 10) {
      SDL_SetRenderDrawColor(renderer, 5, 5, 5, 255);
    } else {
      SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    }
    SDL_RenderFillRect(renderer, &gridRects_.at(i));
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_RenderLine(renderer, 0, gridRects_.at(i).y, static_cast<float>(width_),
                   gridRects_.at(i).y);
  }

  /* for (const auto& tick : gridTicks_) { */
  for (size_t i = 0; i < gridTicks_.size(); i++) {
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    if ((i + 6) % 5 == 0) {
      SDL_SetRenderDrawColor(renderer, 70, 70, 80, 255);
    }
    SDL_RenderLine(renderer, gridTicks_[i] + xOffset_, 0,
                   gridTicks_[i] + xOffset_, static_cast<float>(height_));
  }

  // Draw Notes
  for (auto rects : std::views::zip(notes_, offsetNoteRects_)) {
    float t = helper::map(std::get<0>(rects).velocity, 0.f, 127.f, 0.f, 1.f);
    auto [rc, gc, bc] = helper::heatmap(t);
    SDL_SetRenderDrawColor(renderer, static_cast<Uint8>(rc * 255),
                           static_cast<Uint8>(gc * 255),
                           static_cast<Uint8>(bc * 255), 0xFF);
    SDL_RenderFillRect(renderer, &std::get<1>(rects));
  }
};

void MidiViewer::onMouseWheel(const SDL_Event& event) {
  const bool directionChanged = prevMouseWheel_ != event.wheel.y;
  if (directionChanged) {
    mouseAccel_ = 0.f;
  }
  prevMouseWheel_ = event.wheel.y;
  mouseAccel_ += event.wheel.y;
};

void MidiViewer::onMouseDown(const SDL_Event& event) {
  mouseAccel_ = 0.f;
};

void MidiViewer::populateNotes() {
  using namespace MidiParser;

  Parser parser;
  MidiFile parsed = parser.parse(fileToView_);

  std::unordered_map<uint8_t, uint32_t> noteOnTimes;
  std::unordered_map<uint8_t, uint8_t> noteVelocity;
  // tempo in milliseconds
  std::map<uint32_t, float> tempoMap;

  //populate tempoMap
  for (size_t i = 0; i < parsed.tracks.size(); i++) {
    uint32_t currentTime = 0;
    for (TrackEvent e : parsed.tracks.at(i).events) {
      if (std::holds_alternative<MetaEvent>(e)) {
        auto meta = std::get<MetaEvent>(e);
        currentTime += meta.deltaTime;
        if (meta.status == 0x51) {  // Set Tempo Event
          uint32_t tempo =
              0u | meta.data[0] << 16 | meta.data[1] << 8 | meta.data[2];
          float tempof = static_cast<float>(tempo) / 1000.f;
          tempoMap.insert_or_assign(currentTime, tempof);
        }
      }
    }
  }

  for (size_t i = 0; i < parsed.tracks.size(); i++) {
    uint32_t currentTime = 0;
    float currentTempo;

    auto visitMidi = [&currentTime, &noteOnTimes, &noteVelocity, &currentTempo,
                      tempoMap, this, parsed](MIDIEvent& e) mutable {
      currentTime += e.deltaTime;
      // get currentTempo;
      for (auto i = tempoMap.begin(); i != tempoMap.end(); i++) {
        if (currentTime >= (*i).first) {
          currentTempo = (*i).second;
        }
      }

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
        noteOnTimes.insert_or_assign(key, currentTime);
        noteVelocity.insert_or_assign(key, velocity);
      }
      float start = static_cast<float>(noteOnTimes.at(key)) /
                    static_cast<float>(parsed.tickDivision) * currentTempo;
      float end = static_cast<float>(currentTime) /
                  static_cast<float>(parsed.tickDivision) * currentTempo;
      if (isNoteOff) {
        notes_.push_back({.key = key,
                          .velocity = noteVelocity.at(key),
                          .start = start,
                          .end = end});
      }
    };

    auto visitor = overload{
        visitMidi,
        [&currentTime](MetaEvent& e) mutable { currentTime += e.deltaTime; },
        [](SysExEvent&) { /* ignore */ }};

    for (TrackEvent e : parsed.tracks.at(i).events) {
      std::visit(visitor, e);
    }
  }
}

void MidiViewer::populateNoteRects() {
  for (const auto& note : notes_) {
    SDL_FRect r{
        .x = helper::map(note.start, 0.f, pageSize_, 0.f,
                         static_cast<float>(width_), false) +
             padding_,
        .y = helper::map(static_cast<float>(note.key),
                         static_cast<float>(lowestNote_.key),
                         static_cast<float>(highestNote_.key),
                         static_cast<float>(height_) - noteHeight_, 0.f) +
             padding_,
        .w = helper::map(note.end - note.start, 0.f, pageSize_, 0,
                         static_cast<float>(width_)) -
             padding_ * 2.f,
        .h = noteHeight_ - padding_ * 2.f};
    noteRects_.push_back(r);
    offsetNoteRects_.push_back(r);
  }
};

}  // namespace Viewers
}  // namespace Can
