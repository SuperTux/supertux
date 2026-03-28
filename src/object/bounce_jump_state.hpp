//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#pragma once

struct PlayerBounceState
{
  bool jumping;
  bool slidejump_falling;
  bool jump_early_apex;
  float gravity_modifier;
};

void clear_player_bounce_state(PlayerBounceState& state);
