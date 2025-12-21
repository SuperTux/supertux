//  SuperTux - Boss "GhostTree"
//  Copyright (C) 2007 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "badguy/boss.hpp"

class GhostTreeAttack;
class TreeWillOWisp;

class GhostTree final : public Boss
{
public:
  GhostTree(const ReaderMapping& mapping);
  ~GhostTree();

  virtual void kill_fall() override { }

  virtual void activate() override;
  virtual void active_update(float dt_sec) override;

  virtual bool collides(MovingObject& other, const CollisionHit& hit) const override;
  virtual HitResponse collision(MovingObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "ghosttree"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Ghost Tree"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return Boss::get_class_types().add(typeid(GhostTree)); }

  virtual void on_flip(float height) override;

  void willowisp_suck_finished(TreeWillOWisp* willowisp);

protected:
  virtual std::vector<Direction> get_allowed_directions() const override;
  virtual bool collision_squished(MovingObject& object) override;

private:
  enum MyState {
    STATE_INIT,
    STATE_SCREAM,
    STATE_IDLE,
    STATE_SUCKING,
    STATE_SPITTING,
    STATE_ATTACKING,
    STATE_RECHARGING,
    STATE_DEAD,
    STATE_MUSIC_FADE_OUT,
    STATE_WISP_FLY_AWAY,
  };
  
  enum AttackType {
    ATTACK_NORMAL,

    ATTACK_RED,
    ATTACK_GREEN,
    ATTACK_BLUE,

    ATTACK_PINCH,

    ATTACK_FIRST_SPECIAL = ATTACK_RED,
  };

private:
  void set_state(MyState new_state);
  bool should_suck(const Color& color) const;

private:
  MyState m_state;
  AttackType m_attack;
  AttackType m_willo;
  Timer m_state_timer;
  float m_willo_spawn_y;
  float m_willo_radius;
  float m_willo_speed;
  int m_willo_to_spawn;
  Vector m_attack_pos;

  std::vector<TreeWillOWisp*> m_willowisps;
  std::unique_ptr<GhostTreeAttack> m_root_attack;
  void spawn_willowisp(AttackType color);
  void rotate_willo_color();

  Vector get_attack_pos() const;
  void start_attack();

private:
  GhostTree(const GhostTree&) = delete;
  GhostTree& operator=(const GhostTree&) = delete;
};
