#include <iostream>
#include <cassert>
#include <cmath>
#include "util/colorspace_oklab.hpp"
#include "video/color.hpp"

// Approximate comparison helper
bool approx_equal(float a, float b, float epsilon = 1e-4f) {
  return std::fabs(a - b) < epsilon;
}

void test_conversion_to_oklch() {
  Color c(1.0f, 0.0f, 0.0f);  // bright red
  ColorOKLCh oklch(c);

  std::cout << "Converted OKLCh values:\n";
  std::cout << "L: " << oklch.L << " C: " << oklch.C << " h: " << oklch.h << "\n";

  assert(oklch.L > 0.0f && oklch.L <= 1.0f);
  assert(oklch.C > 0.0f);
}

void test_modified_lightness() {
  Color c(0.5f, 0.5f, 0.5f);  // neutral gray
  ColorOKLCh oklch(c);

  float modL = oklch.get_modified_lightness();
  std::cout << "Modified Lightness: " << modL << "\n";

  assert(modL >= 0.0f && modL <= 1.0f);
}

void test_max_chroma() {
  Color c(0.8f, 0.2f, 0.4f);
  ColorOKLCh oklch(c);
  float max_c = oklch.get_max_chroma(10);
  std::cout << "Max Chroma (10 steps): " << max_c << "\n";

  assert(max_c >= 0.0f && max_c <= 1.0f);
}

int main() {
  test_conversion_to_oklch();
  test_modified_lightness();
  test_max_chroma();
  std::cout << "All tests passed.\n";
  return 0;
}