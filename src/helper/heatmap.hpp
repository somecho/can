#include <array>
#include <cmath>

namespace Can {

namespace helper {

// https://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
inline std::array<float, 3> heatmap(float value) {
  using rgb = std::array<float, 3>;
  const int NUM_COLORS = 4;
  static std::array<rgb, 4> color = {rgb{0, 0, 1}, rgb{0, 1, 0}, rgb{1, 1, 0},
                                     rgb{1, 0, 0}};
  int idx1;
  int idx2;
  float fractBetween = 0;
  if (value <= 0) {
    idx1 = idx2 = 0;
  } else if (value >= 1) {
    idx1 = idx2 = NUM_COLORS - 1;
  } else {
    value = value * (NUM_COLORS - 1);
    idx1 = floor(value);
    idx2 = idx1 + 1;
    fractBetween = value - float(idx1);
  }

  return rgb{(color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0],
             (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1],
             (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2]};
}

}  // namespace helper

}  // namespace Can
