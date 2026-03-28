//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#include "st_assert.hpp"
#include "object/bounce_jump_state.hpp"

int main(void)
{
  PlayerBounceState state{true, true, true, 3.0f};

  clear_player_bounce_state(state);

  ST_ASSERT("jumping is cleared", state.jumping == false);
  ST_ASSERT("slidejump falling is cleared", state.slidejump_falling == false);
  ST_ASSERT("early apex is cleared", state.jump_early_apex == false);
  ST_ASSERT("gravity modifier is reset", state.gravity_modifier == 1.0f);

  return 0;
}
