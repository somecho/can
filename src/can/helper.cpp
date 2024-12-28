#include <cmath>

#include "helper.hpp"

namespace Can {
namespace helper {

std::array<float, 3> heatmap(float value) {
  using rgb = std::array<float, 3>;
  const int NUM_COLORS = 4;
  static std::array<rgb, 4> color = {rgb{0, 0, 1}, rgb{0, 1, 0}, rgb{1, 1, 0},
                                     rgb{1, 0, 0}};
  size_t idx1;
  size_t idx2;
  float fractBetween = 0;
  if (value <= 0) {
    idx1 = idx2 = 0;
  } else if (value >= 1) {
    idx1 = idx2 = NUM_COLORS - 1;
  } else {
    value = value * (NUM_COLORS - 1);
    idx1 = static_cast<size_t>(floor(value));
    idx2 = idx1 + 1;
    fractBetween = value - float(idx1);
  }

  return rgb{(color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0],
             (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1],
             (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2]};
}

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

}  // namespace helper
}  // namespace Can
