//  CoinExplode - several coins are hurled through the air
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_COIN_EXPLODE_HPP
#define HEADER_SUPERTUX_OBJECT_COIN_EXPLODE_HPP

#include "math/vector.hpp"
#include "supertux/game_object.hpp"
#include "object/portable.hpp"
#include "util/reader_mapping.hpp"

class CoinExplode final : public GameObject,
                          public Portable
{
public:
  CoinExplode(const Vector& pos, bool count_stats = true,
              const std::string& sprite_path = "images/objects/coin/coin.sprite");
  CoinExplode(const ReaderMapping& reader);
  virtual GameObjectClasses get_class_types() const override { return GameObject::get_class_types().add(typeid(CoinExplode)); }
  static std::string class_name() { return "coin_explode"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Coin Explode"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual bool is_portable() const override { return true; }
  
  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual bool is_saveable() const override {
    return false;
  }
  
  virtual void grab(MovingObject& object, const Vector& pos, Direction dir) override;
  virtual void ungrab(MovingObject& object, Direction dir) override;

private:
  std::string m_sprite;
  Vector position;
  const bool m_count_stats;
  void explode();
};

#endif

/* EOF */
