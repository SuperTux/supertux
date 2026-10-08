// SPDX-License-Identifier: GPL-3.0-or-later
#ifdef NDEBUG
#undef NDEBUG // meaningful assertions also in native Release validation
#endif
#include "control/remote_controller.hpp"
#include <cassert>
#include <chrono>

int main()
{
  using namespace std::chrono_literals;
  RemoteController remote;
  auto time = RemoteController::Time{} + 1s;
  assert(!remote.submit(remote.generation(), 1, 2, time));
  remote.set_enabled(true);
  auto generation = remote.generation();
  Controller& local = remote;
  local.set_control(Control::RIGHT, true);
  assert(!remote.hold(Control::RIGHT));
  assert(!remote.submit(generation, 1, 1u << static_cast<int>(Control::ESCAPE), time));
  assert(remote.submit(generation, 1, 16, time));
  assert(remote.submit(generation, 2, 0, time));
  assert(!remote.submit(generation, 2, 2, time));
  assert(!remote.submit(generation, 1, 2, time));
  remote.advance(time);
  assert(remote.pressed(Control::JUMP) && remote.hold(Control::JUMP));
  remote.advance(time + 16ms);
  assert(remote.released(Control::JUMP) && !remote.hold(Control::JUMP));
  remote.advance(time + 32ms);
  assert(!remote.released(Control::JUMP));
  assert(remote.submit(generation, 3, 2, time + 32ms));
  remote.advance(time + 32ms);
  assert(remote.pressed(Control::RIGHT));
  assert(remote.submit(generation, 4, 2, time + 600ms));
  remote.advance(time + 800ms);
  assert(remote.hold(Control::RIGHT) && !remote.pressed(Control::RIGHT));
  remote.advance(time + 1400ms);
  assert(!remote.hold(Control::RIGHT) && generation != remote.generation());
  assert(!remote.submit(generation, 5, 2, time + 1400ms));
  generation = remote.generation();
  assert(remote.submit(generation, 1, 2, time + 1400ms));
  remote.set_enabled(false);
  remote.set_enabled(true);
  remote.advance(time + 1400ms);
  assert(!remote.hold(Control::RIGHT) && !remote.queued());
  generation = remote.generation();
  for (uint32_t i = 1; i <= RemoteController::QUEUE_LIMIT; ++i)
    assert(remote.submit(generation, i, i % 2 ? 16 : 0, time + 1400ms));
  assert(!remote.submit(generation, 33, 16, time + 1400ms));
  assert(remote.queued() == 0 && remote.generation() != generation);
  generation = remote.generation();
  assert(remote.submit(generation, 1, 2, time + 1400ms));
  remote.advance(time + 1800ms); // obsolete queue cannot replay after a stall
  assert(!remote.hold(Control::RIGHT) && remote.generation() != generation);
  local.reset();
  assert(remote.queued() == 0);
}
