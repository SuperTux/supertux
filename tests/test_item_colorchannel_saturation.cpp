#include <cassert>
#include <iostream>
#include "gui/item_colorchannel_saturation.hpp"
#include "video/color.hpp"

bool approx_equal(float a, float b, float epsilon = 0.01f) {
  return std::fabs(a - b) < epsilon;
}

void test_initialization() {
  Color c(0.5f, 0.5f, 0.5f);
  ColorOKLCh oklch(c);

  ItemColorChannelSaturation item(&c, &oklch, 1);
  assert(approx_equal(*item.m_sat, oklch.C));
  std::cout << "Initialization test passed.\n";
}

void test_add_char_and_clamp() {
  Color c(0.3f, 0.4f, 0.5f);
  ColorOKLCh oklch(c);

  ItemColorChannelSaturation item(&c, &oklch, 1);
  item.add_char('0');
  item.add_char('.');
  item.add_char('9');

  assert(approx_equal(*item.m_sat, 0.9f));
  std::cout << "Char input test passed.\n";
}

void test_left_right_actions() {
  Color c(0.6f, 0.2f, 0.2f);
  ColorOKLCh oklch(c);
  float original = oklch.C;

  ItemColorChannelSaturation item(&c, &oklch, 1);
  item.process_action(MenuAction::RIGHT);
  assert(*item.m_sat >= original);  // could be clamped but shouldn't go down

  item.process_action(MenuAction::LEFT);
  assert(*item.m_sat <= original + 0.01f);  // should go back down or stay near original
  std::cout << "Left/right action test passed.\n";
}

void test_edit_mode_toggle() {
  Color c(0.1f, 0.2f, 0.3f);
  ColorOKLCh oklch(c);
  ItemColorChannelSaturation item(&c, &oklch, 1);

  item.enable_edit_mode();
  item.add_char('0');
  assert(item.get_text().find('0') != std::string::npos);
  std::cout << "Edit mode toggle test passed.\n";
}

int main() {
  test_initialization();
  test_add_char_and_clamp();
  test_left_right_actions();
  test_edit_mode_toggle();
  std::cout << "All ItemColorChannelSaturation tests passed.\n";
  return 0;
}