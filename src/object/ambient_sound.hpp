//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2023 mrkubax10 <mrkubax10@onet.pl>
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

#ifndef HEADER_SUPERTUX_OBJECT_AMBIENT_SOUND_HPP
#define HEADER_SUPERTUX_OBJECT_AMBIENT_SOUND_HPP

#include "supertux/game_object.hpp"
#include "scripting/ambient_sound.hpp"
#include "squirrel/exposed_object.hpp"

class ReaderMapping;
class SoundSource;

class AmbientSound final : public GameObject,
                     public ExposedObject<AmbientSound, scripting::AmbientSound>
{
public:
  AmbientSound(const ReaderMapping& mapping);
  AmbientSound(float vol, float play_interval, const std::string& file);
  ~AmbientSound() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override {}

  static std::string class_name() { return "ambient-sound"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Ambient Sound"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual const std::string get_icon_path() const override { return "images/engine/editor/ambientsound.png"; }

  virtual ObjectSettings get_settings() override;

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

private:
  std::string m_sample;
  std::unique_ptr<SoundSource> m_sound_source;
  float m_volume;
  float m_play_interval;
  float m_delay;
  bool m_playing;
  bool m_scheduled_for_removal;

private:
  void prepare_sound_source();

private:
  AmbientSound(const AmbientSound&) = delete;
  AmbientSound& operator=(const AmbientSound&) = delete;
};

#endif

/* EOF */
