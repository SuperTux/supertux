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

#include "math/vector.hpp"
#include "supertux/moving_object.hpp"
#include "scripting/ambient_sound.hpp"
#include "squirrel/exposed_object.hpp"
#include "video/layer.hpp"

class GameObject;
class ReaderMapping;
class SoundSource;

class AmbientSound final : public MovingObject,
                           public ExposedObject<AmbientSound, scripting::AmbientSound>
{
public:
  AmbientSound(const ReaderMapping& mapping);
  AmbientSound(const Vector& pos, float radius, float vol, const std::string& file);
  ~AmbientSound() override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit_) override;

  static std::string class_name() { return "ambient-sound"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Ambient Sound"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual bool has_variable_size() const override { return true; }

  /** @name Scriptable Methods
      @{ */
#ifndef SCRIPTING_API
  virtual void set_pos(const Vector& pos) override;
#endif
  void set_pos(float x, float y);
  float get_pos_x() const;
  float get_pos_y() const;
  /** @} */

  virtual void draw(DrawingContext& context) override;

  virtual ObjectSettings get_settings() override;

  virtual int get_layer() const override { return LAYER_OBJECTS; }

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

protected:
  virtual void update(float dt_sec) override;

private:
  std::string m_sample;
  std::unique_ptr<SoundSource> m_sound_source;

  float m_radius;
  float m_radius_in_px;
  float m_volume;
  bool m_first_update;

private:
  void prepare_sound_source();

private:
  AmbientSound(const AmbientSound&) = delete;
  AmbientSound& operator=(const AmbientSound&) = delete;
};

#endif

/* EOF */
