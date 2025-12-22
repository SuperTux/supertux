//  SuperTux - Ghost Tree Attacks
//  Copyright (C) 2025 Hypernoot <teratux.mail@gmail.com>
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

#include "badguy/badguy.hpp"

#include "supertux/timer.hpp"

// This implementation is very Java but it is the only way how not to get
// mad of it. I could split it into more files but that would be unnecessary.

// PART 1: Abstract Classes
// ----------------------------------------------------------------------------

class GhostTreeAttack
{
public:
  GhostTreeAttack();
  virtual ~GhostTreeAttack();

  virtual void active_update(float dtime) = 0;
  virtual bool is_done() const = 0;
  virtual void root_died() {}
};

class GhostTreeRoot : public BadGuy
{
public:
  GhostTreeRoot(const Vector& pos, Direction dir, const std::string& sprite);
  virtual~GhostTreeRoot();
  
  virtual HitResponse collision_badguy(BadGuy& other, const CollisionHit& hit) override;
  virtual void kill_fall() override;

  virtual bool is_flammable() const override { return false; }
  virtual bool is_freezable() const override { return false; }
  virtual bool is_snipable() const override { return false; }
};

// PART 2: Roots
// ----------------------------------------------------------------------------

class GhostTreeRootMain final : public GhostTreeRoot
{
public:
  GhostTreeRootMain(const Vector& pos, GhostTreeAttack* parent);
  ~GhostTreeRootMain();

  virtual void active_update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

private:
  enum State {
    STATE_HATCHING,
    STATE_RISING,
    STATE_FALLING,
    STATE_FADE_OUT
  };

  State m_state;
  Timer m_state_timer;
  float m_maxheight;

  GhostTreeAttack* m_parent;

  SurfacePtr m_hill;
};

class GhostTreeRootRed final : public GhostTreeRoot
{
public:
  GhostTreeRootRed(const Vector& pos, GhostTreeAttack* parent);
  ~GhostTreeRootRed();

  virtual void active_update(float dt_sec) override;

private:
  enum State {
    STATE_RISING,
    STATE_FALLING
  };

  float m_level_bottom;
  float m_level_top;
  State m_state;
  GhostTreeAttack* m_parent;
};

class GhostTreeRootGreen final : public GhostTreeRoot
{
public:
  GhostTreeRootGreen(const Vector& pos, GhostTreeAttack* parent);
  ~GhostTreeRootGreen();

  virtual void active_update(float dt_sec) override;

private:
  float m_level_top;
  GhostTreeAttack* m_parent;
};

class GhostTreeRootBlue final : public GhostTreeRoot
{
public:
  GhostTreeRootBlue(const Vector& pos, GhostTreeAttack* parent);
  ~GhostTreeRootBlue();

  virtual void active_update(float dt_sec) override;

private:
  enum State {
    STATE_RISING,
    STATE_FIRE_DELAY,
    STATE_FALL_DELAY,
    STATE_FALLING
  };

  float m_level_bottom;
  float m_level_top;
  State m_state;
  Timer m_state_timer;
  int m_variant;
  GhostTreeAttack* m_parent;
  
  void fire();
};

class GhostTreeRootPinch final : public GhostTreeRoot
{
public:
  GhostTreeRootPinch(const Vector& pos, GhostTreeAttack* parent);
  ~GhostTreeRootPinch();

  virtual void active_update(float dt_sec) override;

private:
  enum State {
    STATE_RISING,
    STATE_FIRE_DELAY,
    STATE_EXPLOSION_DELAY
  };

  float m_level_top;
  State m_state;
  Timer m_state_timer;
  GhostTreeAttack* m_parent;
  
   void fire();
};

// PART 3: Root Attacks
// ----------------------------------------------------------------------------

class GhostTreeAttackMain final : public GhostTreeAttack
{
public:
  GhostTreeAttackMain(Vector pos);
  ~GhostTreeAttackMain();

  virtual void active_update(float dtime) override;
  virtual bool is_done() const override;
  virtual void root_died() override;

private:
  Timer m_spawn_timer;
  Vector m_pos;
  int m_remaining_roots;
};

class GhostTreeAttackRed final : public GhostTreeAttack
{
public:
  GhostTreeAttackRed(float y, float x_start, float x_end);
  ~GhostTreeAttackRed();

  virtual void active_update(float dtime) override;
  virtual bool is_done() const override;
  virtual void root_died() override;

private:
  Timer m_spawn_timer;
  float m_pos_y;
  float m_start_x;
  float m_end_x;
  float m_current_x;
  int m_remaining_roots;
  bool m_ended;
};

class GhostTreeAttackGreen final : public GhostTreeAttack
{
public:
  GhostTreeAttackGreen(const Vector& pos);
  ~GhostTreeAttackGreen();

  virtual void active_update(float dtime) override;
  virtual bool is_done() const override;
  virtual void root_died() override;

private:
  bool m_ended;
};

class GhostTreeAttackBlue final : public GhostTreeAttack
{
public:
  GhostTreeAttackBlue(const Vector& pos);
  ~GhostTreeAttackBlue();

  virtual void active_update(float dtime) override;
  virtual bool is_done() const override;
  virtual void root_died() override;

private:
  bool m_ended;
};

class GhostTreeAttackPinch final : public GhostTreeAttack
{
public:
  GhostTreeAttackPinch(const Vector& pos, float x_left, float x_right);
  ~GhostTreeAttackPinch();

  virtual void active_update(float dtime) override;
  virtual bool is_done() const override;
  virtual void root_died() override;

private:
  bool m_root_ended;
  GhostTreeAttackRed m_left_trail;
  GhostTreeAttackRed m_right_trail;
};

