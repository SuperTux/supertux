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
#include "object/player.hpp"

class BonusBlock final : public Block
{
public:
  enum Contents {
    CONTENT_COIN,
    CONTENT_FIREGROW,
    CONTENT_ICEGROW,
    CONTENT_AIRGROW,
    CONTENT_EARTHGROW,
    CONTENT_STAR,
    CONTENT_1UP,
    CONTENT_CUSTOM,
    CONTENT_SCRIPT,
    CONTENT_LIGHT,
    CONTENT_TRAMPOLINE,
    CONTENT_RAIN,
    CONTENT_EXPLODE
  };

public:
  BonusBlock(const Vector& pos, int data);
  BonusBlock(const ReaderMapping& lisp);
  virtual ~BonusBlock();

  virtual void hit(Player& player) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void draw(DrawingContext& context) override;

  Contents get_contents() const { return m_contents; }
  int get_hit_counter() const { return m_hit_counter; }

  virtual void save(Writer& writer) override;

  virtual std::string get_class() const override {
    return "bonusblock";
  }
  virtual std::string get_display_name() const override {
    return _("Bonus block");
  }

  virtual ObjectSettings get_settings() override;

private:
  void try_open(Player* player);
  void try_drop(Player* player);

  void preload_contents(int d);
  void raise_growup_bonus(Player* player, const BonusType& bonus, const Direction& dir);
  void drop_growup_bonus(const std::string& bonus_sprite_name, bool& countdown);

  BonusBlock::Contents get_content_by_data(int d) const;
  BonusBlock::Contents get_content_from_string(const std::string& contentstring) const;
  std::string contents_to_string(const BonusBlock::Contents& content) const;

private:
  Contents m_contents;
  std::shared_ptr<MovingObject> m_object;
  int m_hit_counter;

private:
  std::string m_script;
  SurfacePtr m_lightsprite;

private:
  BonusBlock(const BonusBlock&) = delete;
  BonusBlock& operator=(const BonusBlock&) = delete;
};

#endif

/* EOF */
