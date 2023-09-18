#include "granito.hpp"

Granito::Granito(const ReaderMapping& reader):
  WalkingBadguy(reader, "images/creatures/granito/granito.sprite", "left", "right")
{
  parse_type(reader);

  walk_speed = 0;
  max_drop_height = 16;

  m_countMe = false;
}

void Granito::active_update(float dt_sec)
{
  WalkingBadguy::active_update(dt_sec);
}

void Granito::kill_fall()
{
  return;
}

GameObjectTypes Granito::get_types() const
{
  return {

  };
}

void Granito::initialize()
{
  WalkingBadguy::initialize();
  set_colgroup_active(COLGROUP_MOVING_ONLY_STATIC);
}
