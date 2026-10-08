// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "control/controller.hpp"

#include <chrono>
#include <cstdint>
#include <deque>

/** A transient gameplay source. Local devices cannot write its controls. */
class RemoteController final : public Controller
{
public:
  using Clock = std::chrono::steady_clock;
  using Time = Clock::time_point;
  static constexpr uint32_t GAMEPLAY_MASK = (1u << 7) - 1;
  static constexpr size_t QUEUE_LIMIT = 32;

  RemoteController();
  void update() override;
  void advance(Time now); // also used by deterministic replay tests
  void reset() override;
  void set_control(Control, bool) override {} // ownership guard, including script callers
  void set_enabled(bool enabled);
  bool submit(uint32_t generation, uint32_t sequence, uint32_t mask, Time now = Clock::now());
  uint32_t generation() const { return m_generation; }
  uint32_t sequence() const { return m_sequence; }
  bool enabled() const { return m_enabled; }
  size_t queued() const { return m_queue.size(); }

private:
  struct State { uint32_t mask; Time received; };
  uint32_t m_generation = 1;
  uint32_t m_sequence = 0;
  uint32_t m_latest_mask = 0;
  bool m_enabled = false;
  Time m_received{};
  std::deque<State> m_queue;
};
