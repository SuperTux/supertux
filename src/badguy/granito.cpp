#include "granito.hpp"

#include "math/random.hpp"
#include "supertux/sector.hpp"
#include "object/player.hpp"

Granito::Granito(const ReaderMapping& reader):
  WalkingBadguy(reader, "images/creatures/granito/granito.sprite", "left", "right"),
  m_state(STATE_STAND),
  m_has_waved(false),
  m_stepped_on(false)
{
  parse_type(reader);

  walk_speed = 0;
  max_drop_height = 600;

  m_countMe = false;

}

void Granito::active_update(float dt_sec)
{
  if (!m_stepped_on && m_state == STATE_LOOKUP)
  {
    m_state = STATE_STAND;
    set_action("stand", m_dir);
  }

  if (m_type == SIT || m_state == STATE_LOOKUP)
  {
    // Sit type and look up state do nothing
    WalkingBadguy::active_update(dt_sec);
    m_stepped_on = false;
    return;
  }

  if (!m_has_waved)
  {
    if (m_state == STATE_WAVE)
    {
      if (!m_sprite->animation_done())
      {
        // Still waving
        WalkingBadguy::active_update(dt_sec);
        m_stepped_on = false;
        return;
      }
      else
      {
        // Finished waving
        m_state = STATE_STAND;
        set_action("stand", m_dir);
        m_has_waved = true;
      }
    }
    else
    {
      try_wave();
    }
  }

  if (!m_walk_interval.started() && !m_walk_interval.check())
  {
    m_walk_interval.start(gameRandom.randf(1.f, 4.f));

    switch (m_type)
    {
      case STAND:
        if (gameRandom.rand(100) > 50)
        {
          // turn around
          m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
          set_action("stand", m_dir);
        }

        break;

      case WALK:
      {
        if (gameRandom.rand(100) > 50 && walk_speed == 0)
        {
          // turn around
          m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
          set_action("stand", m_dir);
        }
        else
        {
          // walk/stop
          if (walk_speed > 0)
          {
            walk_speed = 0;
            m_state = STATE_STAND;
            m_physic.set_velocity_x(0);
            set_action("stand", m_dir);
          }
          else
          {
            // FIXME: Why do I need to add 1??? Grumbel, you...
            m_dir = (gameRandom.rand(1 + 1) == 0 ? Direction::LEFT : Direction::RIGHT);
            walk_speed = 80;
            m_state = STATE_WALK;
            m_physic.set_velocity_x(80 * (m_dir == Direction::LEFT ? -1 : 1));
            set_action(m_dir);
          }
        }

        break;
      }

      default:
        break;
    }
  }

  WalkingBadguy::active_update(dt_sec);

  m_stepped_on = false;
}

HitResponse Granito::collision_player(Player& player, const CollisionHit &hit)
{
  if (m_type == SIT) return FORCE_MOVE;

  if (hit.top)
  {
    m_stepped_on = true;
    //m_col.propagate_movement(m_col.get_movement());
    if (m_state != STATE_LOOKUP)
    {
      m_state = STATE_LOOKUP;
      set_action("lookup", m_dir);

      // Don't wave again because we've
      // already spotted the player
      m_has_waved = true;
    }
  }

  return FORCE_MOVE;
}

void Granito::kill_fall()
{
  return;
}

void Granito::activate()
{
  WalkingBadguy::activate();
  m_has_waved = false;
}

GameObjectTypes Granito::get_types() const
{
  return {
    {"walking", _("Walking")},
    {"sitting", _("Sitting")},
    {"standing", _("Standing")}
  };
}

void Granito::initialize()
{
  WalkingBadguy::initialize();

  switch (m_type)
  {
    case WALK:
      set_action(m_dir);
      break;

    case SIT:
      // TODO: sit animation
      set_action("stand", m_dir);
      break;

    case STAND:
      set_action("stand", m_dir);
      break;
  }

  set_colgroup_active(COLGROUP_MOVING_STATIC);
}

void Granito::on_type_change(__attribute__((unused)) int old_type)
{
  // FIXME: change action for type in editor
}

bool Granito::try_wave()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  Player* plr = get_nearest_player();
  RaycastResult result = Sector::get().get_first_line_intersection(get_bbox().get_middle(),
                                                                   plr->get_bbox().get_middle(),
                                                                   false,
                                                                   get_collision_object());

  if (result.hit.object != nullptr && result.hit.object == plr->get_collision_object())
  {
    float xdist = get_bbox().get_middle().x - result.box.get_middle().x;
    if (std::abs(xdist) < 32.f*4.f)
    {
      // Only wave if facing player.
      if (xdist == std::abs(xdist) * (m_dir == Direction::LEFT ? -1 : 1))
        return false;

      wave();
      return true;
    }
  }

  return false;
}

void Granito::wave()
{
  walk_speed = 0;
  m_physic.set_velocity_x(0);

  m_state = STATE_WAVE;

  set_action("wave", m_dir, 1);
}
