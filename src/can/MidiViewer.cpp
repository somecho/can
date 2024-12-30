#include <algorithm>
#include <cmath>

#include "MidiViewer.hpp"
#include "helper.hpp"

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

  // calculate bounds of viewer based on midi data
  for (size_t i = 0; i < allNotes_.key.size(); i++) {
    auto k = allNotes_.key[i];
    if (k < lowestKey_) {
      lowestKey_ = k;
    }
    if (k > highestKey_) {
      highestKey_ = k;
    }
    if (allNotes_.end[i] > totalMillis_) {
      totalMillis_ = allNotes_.end[i];
    }
  }
  inclusiveNoteRange_ = highestKey_ - lowestKey_ + 1;
  noteHeight_ =
      static_cast<float>(height_) / static_cast<float>(inclusiveNoteRange_);

  pageSize_ = static_cast<float>(width_) * 10.f;

  // Length of MIDI Track in terms of number of bars(pages) it can fit in
  /* float trackLengthNormalized = rightMostNote_.end / pageSize_; */
  float trackLengthNormalized = totalMillis_ / pageSize_;

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
                          heightf_ - noteHeight_, -noteHeight_),
         .w = widthf_,
         .h = noteHeight_});
  }

  // populate gridTicks_
  float interval = 1.f * 1000.f;  // 10 seconds
  unsigned int numTicks =
      static_cast<unsigned int>(std::floor(totalMillis_ / interval));
  numTicks = std::max(
      {numTicks, static_cast<unsigned int>(std::floor(pageSize_ / interval))});
  for (auto i = 1u; i <= numTicks; i++) {
    float x = helper::map(static_cast<float>(i) * interval, 0, pageSize_, 0,
                          widthf_, false);
    gridTicks_.emplace_back(x);
  }
}

void MidiViewer::update() {
  // update scroll physics
  mouseAccel_ += (0 - mouseAccel_) * mouseAccelDamping_;
  xOffset_ += mouseAccel_ * mouseAccelScaling_;
  xOffset_ = std::clamp(xOffset_, xOffsetMin_, xOffsetMax_);

  size_t writeBuffer = 1 - currentBuffer;
  drawnRects_[writeBuffer].clear();
  for (const auto& rectColPair : referenceRects_) {
    auto r = std::get<0>(rectColPair);
    float xpos = r.x + xOffset_;
    float xposEnd = xpos + r.w;
    auto col = std::get<1>(rectColPair);
    if ((xpos > 0 && xpos < widthf_) || (xposEnd > 0 && xposEnd < widthf_)) {
      drawnRects_[writeBuffer].emplace_back(
          std::pair{SDL_FRect{.x = xpos, .y = r.y, .w = r.w, .h = r.h}, col});
    }
  }
  currentBuffer = writeBuffer;
}

void MidiViewer::render(SDL_Renderer* renderer) {
  SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
  SDL_RenderClear(renderer);
  drawPianoRoll(renderer);
  drawTimeTicks(renderer);
  drawMIDINotes(renderer);
};

void MidiViewer::drawPianoRoll(SDL_Renderer* renderer) {
  for (auto i = 0u; i < inclusiveNoteRange_; i++) {
    int id = (i + lowestKey_) % 12;
    if (id == 1 || id == 3 || id == 6 || id == 8 || id == 10) {
      SDL_SetRenderDrawColor(renderer, 5, 5, 5, 255);
    } else {
      SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    }
    SDL_RenderFillRect(renderer, &gridRects_.at(i));
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_RenderLine(renderer, 0, gridRects_.at(i).y, widthf_,
                   gridRects_.at(i).y);
  }
}

void MidiViewer::drawTimeTicks(SDL_Renderer* renderer) {
  for (size_t i = 0; i < gridTicks_.size(); i++) {
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    if ((i + 6) % 5 == 0) {  // Accent every 5th interval
      SDL_SetRenderDrawColor(renderer, 70, 70, 80, 255);
    }
    auto xPos = gridTicks_.at(i) + xOffset_;
    SDL_RenderLine(renderer, xPos, 0, xPos, heightf_);
  }
}

void MidiViewer::drawMIDINotes(SDL_Renderer* renderer) {
  for (const auto& p : drawnRects_[currentBuffer]) {
    auto col = std::get<1>(p);
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    SDL_RenderFillRect(renderer, &std::get<0>(p));
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
  float tickf = static_cast<float>(parsed.tickDivision);

  struct {
    std::vector<uint32_t> time;
    // tempo in milliseconds
    std::vector<float> tempo;
  } tempoMap;

  //populate tempoMap
  for (size_t i = 0; i < parsed.tracks.size(); i++) {
    uint32_t currentTime = 0;
    for (TrackEvent e : parsed.tracks.at(i).events) {
      if (const MetaEvent* meta = std::get_if<MetaEvent>(&e)) {
        currentTime += meta->deltaTime;
        if (meta->status == 0x51) {  // Set Tempo Event
          uint32_t tempo =
              0u | meta->data[0] << 16 | meta->data[1] << 8 | meta->data[2];
          float tempof = static_cast<float>(tempo) / 1000.f;
          tempoMap.time.push_back(currentTime);
          tempoMap.tempo.push_back(tempof);
        }
      }
    }
  }

  std::vector<std::thread> threads;
  struct SOA {
    std::vector<uint8_t> key;
    std::vector<uint8_t> vel;
    std::vector<float> start;
    std::vector<float> end;
  };
  std::vector<SOA> tempNotes;
  tempNotes.resize(parsed.tracks.size());
  for (size_t i = 0; i < parsed.tracks.size(); i++) {
    threads.push_back(std::thread([i, parsed, tempoMap, tickf,
                                   &tempNotes]() mutable {
      std::array<uint32_t, 127> noteOn;
      std::array<uint8_t, 127> noteVel;
      uint32_t currentTime = 0;
      float currentTempo = 0;
      for (const TrackEvent& e : parsed.tracks.at(i).events) {
        if (const MIDIEvent* event = std::get_if<MIDIEvent>(&e)) {
          currentTime += event->deltaTime;
          // get currentTempo;
          for (size_t i = 0; i < tempoMap.time.size(); i++) {
            if (currentTime >= tempoMap.time[i]) {
              currentTempo = tempoMap.tempo[i];
            }
          }
          const uint8_t masked = event->status & 0b11110000;
          const bool hasNoteOnStatus = masked == 0b10010000;
          const bool hasNoteOffStatus = masked == 0b10000000;
          if (!(hasNoteOnStatus || hasNoteOffStatus)) {
            continue;
          }
          const uint8_t key = event->data.at(0);
          const uint8_t velocity = event->data.at(1);
          const bool isNoteOn = hasNoteOnStatus && velocity > 0;
          const bool isNoteOff =
              (hasNoteOnStatus && velocity == 0) || hasNoteOffStatus;
          if (isNoteOn) {
            noteOn[key] = currentTime;
            noteVel[key] = velocity;
          }
          float start = static_cast<float>(noteOn[key]) / tickf * currentTempo;
          float end = static_cast<float>(currentTime) / tickf * currentTempo;
          if (isNoteOff) {
            tempNotes[i].key.push_back(key);
            tempNotes[i].vel.push_back(noteVel[key]);
            tempNotes[i].start.push_back(start);
            tempNotes[i].end.push_back(end);
          }
        }
        if (const MetaEvent* event = std::get_if<MetaEvent>(&e)) {
          currentTime += event->deltaTime;
        }
      }
    }));
  }  // end for loop
  for (auto& t : threads) {
    t.join();
  }
  for (auto& noteCol : tempNotes) {
    allNotes_.key.insert(allNotes_.key.end(), noteCol.key.begin(),
                         noteCol.key.end());
    allNotes_.vel.insert(allNotes_.vel.end(), noteCol.vel.begin(),
                         noteCol.vel.end());
    allNotes_.start.insert(allNotes_.start.end(), noteCol.start.begin(),
                           noteCol.start.end());
    allNotes_.end.insert(allNotes_.end.end(), noteCol.end.begin(),
                         noteCol.end.end());
  }
}

void MidiViewer::populateNoteRects() {
  for (size_t i = 0; i < allNotes_.key.size(); i++) {
    SDL_FRect r{.x = helper::map(allNotes_.start[i], 0.f, pageSize_, 0.f,
                                 widthf_, false) +
                     padding_,
                .y = helper::map(static_cast<float>(allNotes_.key[i]),
                                 static_cast<float>(lowestKey_),
                                 static_cast<float>(highestKey_),
                                 heightf_ - noteHeight_, 0.f) +
                     padding_,
                .w = helper::map(allNotes_.end[i] - allNotes_.start[i], 0.f,
                                 pageSize_, 0, widthf_) -
                     padding_ * 2.f,
                .h = noteHeight_ - padding_ * 2.f};
    auto [rc, gc, bc] =
        helper::heatmap(static_cast<float>(allNotes_.vel[i]) / 127.f);
    SDL_Color col{.r = static_cast<uint8_t>(rc * 255.f),
                  .g = static_cast<uint8_t>(gc * 255.f),
                  .b = static_cast<uint8_t>(bc * 255.f),
                  .a = 255};
    referenceRects_.push_back({r, col});
  }
};

}  // namespace Viewers
}  // namespace Can
