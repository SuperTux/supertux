//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#include "object/bounce_jump_state.hpp"

void
clear_player_bounce_state(PlayerBounceState& state)
{
  state.jumping = false;
  state.slidejump_falling = false;
  state.jump_early_apex = false;
  state.gravity_modifier = 1.0f;
}
