#pragma once

#include <array>

namespace Can {
namespace helper {

std::array<float, 3> heatmap(float value);

float map(float value, float inputMin, float inputMax, float outputMin,
          float outputMax, bool clamp = true);

}  // namespace helper
}  // namespace Can
