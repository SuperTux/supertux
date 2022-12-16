//  SuperTux - Badguy "Igel"
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//  Copyright (C) 2022 Daniel Ward <weluvgoatz@gmail.com>
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

#ifndef HEADER_SUPERTUX_BADGUY_IGEL_HPP
#define HEADER_SUPERTUX_BADGUY_IGEL_HPP

#include "badguy/walking_badguy.hpp"

/** Badguy "Igel" - a hedgehog that rolls around ;) */
class Igel final : public WalkingBadguy
{
public:
  Igel(const ReaderMapping& reader);

  virtual void active_update(float dt_sec) override;

  virtual bool is_freezable() const override { return true; }

  virtual std::string get_overlay_size() const override { return "2x1"; }
  static std::string class_name() { return "igel"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Igel"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;
  virtual void collision_solid(const CollisionHit& hit) override;

private:
  enum IgelState {
    WALKING,
    FOUND,
    ROLLING
  };
  IgelState m_state;
  Timer m_found_timer;

private:
  Igel(const Igel&) = delete;
  Igel& operator=(const Igel&) = delete;
};

#endif

/* EOF */
