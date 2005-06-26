#include <config.h>

#include <string>
#include <stdio.h>
#include "level.hpp"
#include "game_session.hpp"
#include "flip_level_transformer.hpp"

namespace Scripting
{
  Level::Level()
  {}

  Level::~Level()
  {}

  void
  Level::finish()
  {
    GameSession::current()->finish();
  }

  void
  Level::spawn(const std::string& sector, const std::string& spawnpoint)
  {
    GameSession::current()->respawn(sector, spawnpoint);
  }

  void
  Level::flip_vertically()
  {
    FlipLevelTransformer flip_transformer;
    flip_transformer.transform(GameSession::current()->get_current_level());
  }
}
