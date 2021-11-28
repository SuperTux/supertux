//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
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

#ifndef HEADER_SUPERTUX_OBJECT_TEXT_AREA_HPP
#define HEADER_SUPERTUX_OBJECT_TEXT_AREA_HPP

#include "supertux/moving_object.hpp"

class TextArea final : public MovingObject
{
public:
  TextArea(const ReaderMapping& mapping);
  TextArea(const Vector& pos);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  virtual ObjectSettings get_settings() override;
  virtual std::string get_class() const override { return "text-area"; }
  virtual std::string get_display_name() const override { return _("Text Area"); }
  virtual bool has_variable_size() const override { return true; }
private:
  bool m_started;
private:
  TextArea(const TextArea&) = delete;
  TextArea& operator=(const TextArea&) = delete;
};

#endif
/* EOF */
