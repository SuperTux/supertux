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

#include <math.h>

#include "ambient_sound.h"
#include "object_factory.h"
#include "lisp/lisp.h"
#include "sector.h"

AmbientSound::AmbientSound(const lisp::Lisp& lisp)
{

  position.x=0;
  position.y=0;
  distance_factor=0;
  distance_bias=0;
  sample="";

  if (!(lisp.get("x", position.x)&&lisp.get("y", position.y))) {
    std::cerr << "No Position in ambient_sound"  << std::endl;
  }
  lisp.get("distance_factor",distance_factor);
  lisp.get("distance_bias"  ,distance_bias  );
  lisp.get("sample"         ,sample         );

  if (distance_factor == 0)
    silence_distance = 10e99;
  else
    silence_distance = distance_factor*10e2;

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
  Mix_Volume(playing,0);
  currentvolume=targetvolume=1e-20;
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

  targetvolume=1/(1+distance*distance_factor);
  float rise=targetvolume/currentvolume;
  currentvolume*=pow(rise,.05);
  currentvolume += 1e-30;
  //  std::cout << currentvolume << " " << targetvolume << std::endl;
  Mix_Volume(playing,(int)(currentvolume*MIX_MAX_VOLUME));

}

void
AmbientSound::draw(DrawingContext &) 
{
}

IMPLEMENT_FACTORY(AmbientSound, "ambient_sound");
