// ambient_sound.h   basti_
// 
//  SuperTux
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

/**
 *  Ambient Sound Source, beta version. Features:
 *
 *  - "disc" like structure. Full volume up to some distance
 *    (distance_bias) to the source, then fading proportional to
 *    inverse square distance
 *  
 *  - parameters for point source:
 *    x, y               position
 *    distance_factor    high = steep fallofff
 *    distance_bias      high = big "100% disc"
 *    silence_distance   defaults reasonably.
 * 
 *      basti_ 
 */

#ifndef __AMBIENT_SOUND_H__
#define __AMBIENT_SOUND_H__


#include "game_object.h"
#include "resources.h"
#include "player.h"
#include "SDL_mixer.h"

class AmbientSound : public GameObject
{
public:
  AmbientSound(const lisp::Lisp& lisp);
  ~AmbientSound();
protected:
  virtual void hit(Player& player);
  virtual void action(float time);
  virtual void draw(DrawingContext&);
  virtual void start_playing();
  virtual void stop_playing();
private:
  Vector position;

  std::string sample;
  int playing;
  int latency;

  float distance_factor;  /// distance scaling
  float distance_bias;    /// 100% volume disc radius
  float silence_distance; /// not implemented yet 

  float maximumvolume; /// maximum volume
  float targetvolume;  /// how loud we want to be
  float currentvolume; /// how loud we are

  float * volume_ptr; /// this will be used by the volume adjustment effect.
};

#endif

