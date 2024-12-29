#pragma once

#include <cstdint>

namespace Can {
namespace Viewable {

struct MIDINote {
  uint8_t key;
  uint8_t velocity;
  float start, end;
};

}  // namespace Viewable
}  // namespace Can
