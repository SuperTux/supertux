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
  maximumvolume=1;
  sample="";

  if (!(lisp.get("x", position.x)&&lisp.get("y", position.y))) {
    std::cerr << "No Position in ambient_sound"  << std::endl;
  }
  lisp.get("distance_factor",distance_factor);
  lisp.get("distance_bias"  ,distance_bias  );
  lisp.get("sample"         ,sample         );
  lisp.get("volume"         ,maximumvolume  );

  distance_bias*=distance_bias;
  distance_factor*=distance_factor;

  if (distance_factor == 0)
    silence_distance = 10e99;
  else
    silence_distance = 1/distance_factor;
  
  lisp.get("silence_distance",silence_distance);

  playing=-1; // not playing at the beginning
  latency=0;

}

AmbientSound::~AmbientSound() {
  stop_playing();
}

void
AmbientSound::hit(Player& )
{
}

void
AmbientSound::stop_playing() {
  if (playing>=0) {
    Mix_HaltChannel(playing);
    playing=-1;
  }
}

void
AmbientSound::start_playing()
{
  playing=sound_manager->play_sound(sample,-1);
  Mix_Volume(playing,0);
  currentvolume=targetvolume=1e-20;
}

void
AmbientSound::update(float deltat) 
{
  if (latency--<=0) {

    float dx=Sector::current()->player->get_pos().x-position.x;
    float dy=Sector::current()->player->get_pos().y-position.y;
    float sqrdistance=dx*dx+dy*dy;
    
    sqrdistance-=distance_bias;
    
    if (sqrdistance<0)
      sqrdistance=0;
    
    targetvolume=1/(1+sqrdistance*distance_factor);
    float rise=targetvolume/currentvolume;
    currentvolume*=pow(rise,deltat*10);
    currentvolume += 1e-30;

    if (playing>=0) {
      Mix_Volume(playing,(int)(currentvolume*maximumvolume*MIX_MAX_VOLUME));
      if (sqrdistance>=silence_distance && currentvolume<0.05)
	stop_playing();
      latency=0;
    } else {
      if (sqrdistance<silence_distance) {
	start_playing();
	latency=0;
      }
      else 
	latency=(int)(10*((sqrdistance-silence_distance)/silence_distance));
    }

  }
  if (latency>0.001/distance_factor)
    latency=(int)(0.001/distance_factor);
}

void
AmbientSound::draw(DrawingContext &) 
{
}

IMPLEMENT_FACTORY(AmbientSound, "ambient_sound");
