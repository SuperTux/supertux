#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include "sprite/sprite.hpp"

enum CounterType {
  COUNTER_STAR = 0, /*!< @description Invencible Star counter. */
  /* Space for future clock types */

  COUNTER_COUNT
};

struct CounterDef {
  float total_time;
  int num_parts; // number of segments in counter sprite
  std::string center_sprite_path;
  std::string counter_sprite_path;
  std::string effect_sprite_path;
};

class Counter
{
public:
  Counter(CounterType type);
  void update_counter(bool counter, float remaining_time = 0.f);
  
  CounterType get_type() const;
  bool is_counter_on() const;
  int counter_stage() const;
  float effect_angle() const;

  static const CounterDef& get_def(CounterType type);

public:
  bool m_counter_on;

  float m_total_time;
  float m_remaining_time;

private:
  CounterType m_type;

  size_t m_num_parts;
  int m_counter_idx;
  float m_effect_angle;
};
