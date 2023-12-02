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

/**
 *  Ambient Sound Source, gamma version. Features:
 *
 *  - "rounded rectangle" geometry with position, dimension and
 *    "rounding radius" (extending in all directions) of a 100%
 *    volume area, adjustable maximum volume, inverse square
 *    falloff outside area.
 *
 *  - degenerates gracefully to a disc for dimension=0
 *
 *  - parameters:
 *
 *    x, y               position
 *    width, height      dimension
 *    distance_factor    high = steep falloff
 *    distance_bias      high = big "100% disc"
 *    silence_distance   defaults reasonably.
 *    sample             sample to be played back in loop mode
 *
 *      basti_
 */

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
  AmbientSound(const Vector& pos, float factor, float bias, float vol, const std::string& file);
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
  virtual void after_editor_set() override;

  virtual int get_layer() const override { return LAYER_OBJECTS; }

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

protected:
  virtual void update(float dt_sec) override;
  virtual void start_playing();
  virtual void stop_playing();

private:
  std::string sample;
  std::unique_ptr<SoundSource> sound_source;
  int latency;

  float distance_factor;  /// distance scaling
  float distance_bias;    /// 100% volume disc radius
  float silence_distance; /// not implemented yet

  float maximumvolume; /// maximum volume
  float targetvolume;  /// how loud we want to be
  float currentvolume; /// how loud we are

private:
  AmbientSound(const AmbientSound&) = delete;
  AmbientSound& operator=(const AmbientSound&) = delete;
};

#endif

/* EOF */