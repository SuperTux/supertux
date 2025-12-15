#include "badguy/pogo_snowball.hpp"

#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"

static const float JUMP_INTERVAL = 1.5f;   // Time between jumps
static const float JUMP_SPEED = -350.0f;    // Upward velocity (negative = up)

PogoSnowball::PogoSnowball(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/snowball/snowball.sprite"),
  m_jump_timer(0.0f)
{
  // Reuse the standard snowball sprite (walking animation gives a nice "boing" feel)
}

void
PogoSnowball::initialize()
{
  m_physic.enable_gravity(true);
  m_physic.set_velocity(0.0f, 0.0f);
  m_dir = Direction::LEFT;  // Just for sprite facing; doesn't move horizontally
  set_action("left");
}

void
PogoSnowball::active_update(float dt_sec)
{
  if (on_ground())
  {
    m_jump_timer += dt_sec;
    if (m_jump_timer >= JUMP_INTERVAL)
    {
      m_jump_timer = 0.0f;
      m_physic.set_velocity_y(JUMP_SPEED);
      // Optional: play a small sound effect here if you want
      // SoundManager::current()->play("sounds/boing.wav", get_pos());
    }
  }
  else
  {
    // Reset timer while in air to sync jumps properly
    m_jump_timer = 0.0f;
  }

  BadGuy::active_update(dt_sec);
}

int ADD_TO_OBJECT_FACTORY(PogoSnowball);
