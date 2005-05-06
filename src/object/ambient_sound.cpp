// ambient_sound.cpp   basti_
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

#include <config.h>

#include "ambient_sound.h"
#include "object_factory.h"
#include "lisp/lisp.h"
#include "sector.h"

/***
 *  Ambient Sound Source, beta version. Features:
 *
 *  - "disc" like structure. Full volume up to some distance
 *    (distance_bias) to the source, then fading proportional to
 *    inverse square distance
 *  
 *  This is experimental, clicks sometimes and still leaks mem 
 *
 *      basti_ 
 */

AmbientSound::AmbientSound(const lisp::Lisp& lisp)
{

  //  position=pos;
  lisp.get("x", position.x);
  lisp.get("y", position.y);
  lisp.get("distance_factor",distance_factor);
  lisp.get("distance_bias"  ,distance_bias  );
  lisp.get("sample"         ,sample         );

  if (distance_factor == 0)
    silence_distance = 10e99;
  else
    silence_distance = distance_factor*10e2;

  volume_ptr=NULL;
  playing=0;
  startPlaying();
}


void
AmbientSound::hit(Player& )
{
}

void
AmbientSound::startPlaying()
{
  playing=sound_manager->play_sound(sample,-1);
  volume_ptr=new float[2];
  volume_ptr[0]=0;
  volume_ptr[1]=0;
  sound_manager->register_effect(playing,&SoundManager::volume_adjust,
				 NULL,(void *)volume_ptr);
}

void
AmbientSound::action(float) 
{
  float dx=Sector::current()->player->get_pos().x-position.x;
  float dy=Sector::current()->player->get_pos().y-position.y;
  float distance=sqrt(dx*dx+dy*dy);

  distance-=distance_bias;

  if (distance<0)
    distance=0;

  volume_ptr[0]=1/(1+distance*distance_factor); // inverse square of distance

}

void
AmbientSound::draw(DrawingContext &dc) 
{
  return;
  sprite->draw(dc,position,1);

}

IMPLEMENT_FACTORY(AmbientSound, "ambient_sound");
