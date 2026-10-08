// SPDX-License-Identifier: GPL-3.0-or-later
#include "control/remote_controller.hpp"

RemoteController::RemoteController() = default;

void
RemoteController::reset()
{
  Controller::reset();
  m_queue.clear();
  m_sequence = 0;
  m_latest_mask = 0;
  m_received = Time{};
  if (++m_generation == 0) m_generation = 1;
}

void
RemoteController::set_enabled(bool enabled)
{
  if (m_enabled != enabled)
  {
    m_enabled = enabled;
    reset();
  }
}

bool
RemoteController::submit(uint32_t generation, uint32_t sequence, uint32_t mask, Time now)
{
  if (!m_enabled || generation != m_generation || !sequence || sequence <= m_sequence || (mask & ~GAMEPLAY_MASK))
    return false;
  // Coalesce identical refreshes, never transitions: a down/up tap must get
  // two simulation steps even if both packets arrive before the next frame.
  if (mask != m_latest_mask)
  {
    if (m_queue.size() == QUEUE_LIMIT)
    {
      reset(); // fail neutral; invalidate even the packet causing overflow
      return false;
    }
    m_queue.push_back({mask, now});
  }
  m_latest_mask = mask;
  m_sequence = sequence;
  m_received = now;
  return true;
}

void
RemoteController::advance(Time now)
{
  Controller::update(); // capture last frame before applying this frame
  if (!m_enabled) return;
  if ((m_received != Time{} && now - m_received > std::chrono::milliseconds(750)) ||
      (!m_queue.empty() && now - m_queue.front().received > std::chrono::milliseconds(250)))
  {
    reset();
    return;
  }
  if (!m_queue.empty())
  {
    const uint32_t mask = m_queue.front().mask;
    m_queue.pop_front();
    for (int i = 0; i < 7; ++i)
      Controller::set_control(static_cast<Control>(i), (mask & (1u << i)) != 0);
  }
}

void
RemoteController::update()
{
  advance(Clock::now());
}
