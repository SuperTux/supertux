// Browser touch is a separate source: releasing it must not release a key.
#include "control/controller.hpp"
#include <cassert>
#include <bitset>

int main()
{
  Controller controller;
  controller.set_control(Control::LEFT, true);
  assert(controller.hold(Control::LEFT) && controller.pressed(Control::LEFT));
  controller.update();
  assert(controller.hold(Control::LEFT) && !controller.pressed(Control::LEFT));
  controller.set_control(Control::LEFT, false);
  assert(controller.released(Control::LEFT));
  controller.reset();
#ifdef __EMSCRIPTEN__
  std::bitset<static_cast<size_t>(Control::CONTROLCOUNT)> touch;
  const auto set = [&](Control control, bool held) { touch.set(static_cast<size_t>(control), held); };
  set(Control::RIGHT, true);
  set(Control::JUMP, true);
  set(Control::ACTION, true);
  controller.set_touch_controls(touch);
  assert(controller.pressed(Control::RIGHT) && controller.pressed(Control::JUMP) && controller.pressed(Control::ACTION));
  controller.update();
  set(Control::JUMP, false);
  controller.set_touch_controls(touch);
  assert(controller.released(Control::JUMP) && controller.hold(Control::RIGHT) && controller.hold(Control::ACTION));
  controller.set_control(Control::RIGHT, true);
  controller.update();
  set(Control::RIGHT, false);
  controller.set_touch_controls(touch);
  assert(controller.hold(Control::RIGHT) && !controller.released(Control::RIGHT));
  controller.set_control(Control::RIGHT, false);
  assert(controller.released(Control::RIGHT));
  controller.set_control(Control::JUMP, true);
  controller.set_touch_controls({});
  assert(controller.hold(Control::JUMP));
  controller.set_jump_key_with_up(false);
  assert(controller.hold(Control::JUMP));
  controller.reset();
#endif
  for (size_t i = 0; i < static_cast<size_t>(Control::CONTROLCOUNT); ++i)
    assert(!controller.hold(static_cast<Control>(i)) && !controller.pressed(static_cast<Control>(i)));
}
