//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_FLAME_HPP
#define HEADER_SUPERTUX_BADGUY_FLAME_HPP

#include "badguy/badguy.hpp"

#include "audio/sound_source.hpp"

class Flame : public BadGuy
{
public:
  Flame(const ReaderMapping& reader,
        const std::string& sprite = "images/creatures/flame/flame.sprite");

  virtual void activate() override;
  virtual void deactivate() override;

  virtual void active_update(float dt_sec) override;
  virtual void kill_fall() override;

  virtual void freeze() override;
  virtual bool is_freezable() const override;
  virtual bool is_flammable() const override;

  virtual ObjectSettings get_settings() override;
  virtual std::string get_class() const override { return "flame"; }
  virtual std::string get_display_name() const override { return _("Flame"); }

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

protected:
  float angle;
  float radius;
  float speed;

  std::unique_ptr<SoundSource> sound_source;

private:
  Flame(const Flame&) = delete;
  Flame& operator=(const Flame&) = delete;
};

#endif

/* EOF */
