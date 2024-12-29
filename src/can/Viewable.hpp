#pragma once

#include <cstdint>

namespace Can {
namespace Viewable {

struct MIDINote {
  uint8_t key;
  uint8_t velocity;
  uint32_t timeStart, timeEnd;
  float startf, endf;
};

}  // namespace Viewable
}  // namespace Can
