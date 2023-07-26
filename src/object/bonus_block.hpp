//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_BONUS_BLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_BONUS_BLOCK_HPP

#include "object/block.hpp"
#include "supertux/direction.hpp"
#include "supertux/player_status.hpp"

class Player;

class BonusBlock final : public Block
{
public:
  enum class Content {
    COIN,
    FIREGROW,
    ICEGROW,
    AIRGROW,
    EARTHGROW,
    STAR,
    ONEUP,
    CUSTOM,
    SCRIPT,
    LIGHT,
    LIGHT_ON,
    TRAMPOLINE,
    PORTABLE_TRAMPOLINE,
    RAIN,
    EXPLODE,
    ROCK,
    POTION
  };

public:
  BonusBlock(const Vector& pos, int tile_data);
  BonusBlock(const ReaderMapping& mapping);
  ~BonusBlock() override;

  virtual void hit(Player& player) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void draw(DrawingContext& context) override;

  static std::string class_name() { return "bonusblock"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Bonus Block"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;
  GameObjectTypes get_types() const override;
  std::string get_default_sprite_name() const override;

  Content get_contents() const { return m_contents; }
  int get_hit_counter() const { return m_hit_counter; }

  void try_open(Player* player);

private:
  void on_type_change(int old_type) override;

  int get_default_hit_counter() const;

  void try_drop(Player* player);

  void preload_contents(int d);
  void raise_growup_bonus(Player* player, const BonusType& bonus, const Direction& dir);
  void drop_growup_bonus(Player* player, int type, const Direction& dir, bool& countdown);

  BonusBlock::Content get_content_by_data(int tile_data) const;
  BonusBlock::Content get_content_from_string(const std::string& contentstring) const;
  std::string contents_to_string(const BonusBlock::Content& content) const;

private:
  enum Type {
    BLUE,
    ORANGE,
    PURPLE,
    RETRO
  };

private:
  Content m_contents;
  std::unique_ptr<MovingObject> m_object;
  int m_hit_counter;
  std::string m_script;
  SurfacePtr m_lightsprite;
  sexp::Value m_custom_sx;
  std::string m_coin_sprite;

private:
  BonusBlock(const BonusBlock&) = delete;
  BonusBlock& operator=(const BonusBlock&) = delete;
};

#endif

/* EOF */
