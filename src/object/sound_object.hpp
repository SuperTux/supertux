//  SuperTux
//  Copyright (C) 2023 mrkubax10 <mrkubax10@onet.pl>
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

#ifndef HEADER_SUPERTUX_SOUND_OBJECT_HPP
#define HEADER_SUPERTUX_SOUND_OBJECT_HPP

#include "supertux/game_object.hpp"
#include "squirrel/exposed_object.hpp"

#include "scripting/sound_object.hpp"

class ReaderMapping;
class SoundSource;

/** Plays sound at given interval with specified volume hearable in entire Sector */
class SoundObject final : public GameObject,
                          public ExposedObject<SoundObject, scripting::SoundObject>
{
public:
  SoundObject(const ReaderMapping& mapping);
  SoundObject(float vol, const std::string& file);
  ~SoundObject() override;

  virtual void draw(DrawingContext& context) override {}
  virtual void update(float dt_sec) override;

  static std::string class_name() { return "sound-object"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Sound"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual const std::string get_icon_path() const override { return "images/engine/editor/sound.png"; }

  virtual ObjectSettings get_settings() override;

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

  /** @name Scriptable methods
      @{ */
  void set_volume(float volume);
  float get_volume() const { return m_volume; }
  /** @} */

private:
  std::string m_sample;
  std::unique_ptr<SoundSource> m_sound_source;
  float m_volume;
  bool m_started;

private:
  void prepare_sound_source();

private:
  SoundObject(const SoundObject&) = delete;
  SoundObject& operator=(const SoundObject&) = delete;
};

#endif

/* EOF */
