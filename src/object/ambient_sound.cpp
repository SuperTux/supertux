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

  dimension.x=0;
  dimension.y=0;

  distance_factor=0;
  distance_bias=0;
  maximumvolume=1;
  sample="";

  if (!(lisp.get("x", position.x)&&lisp.get("y", position.y))) {
    std::cerr << "No Position in ambient_sound"  << std::endl;
  }

  lisp.get("width" , dimension.x);
  lisp.get("height", dimension.y);

  lisp.get("distance_factor",distance_factor);
  lisp.get("distance_bias"  ,distance_bias  );
  lisp.get("sample"         ,sample         );
  lisp.get("volume"         ,maximumvolume  );

  // set dimension to zero if smaller than 64, which is default size in flexlay
  
  if ((dimension.x <= 64) || (dimension.y <= 64)) {
    dimension.x = 0;
    dimension.y = 0;
  }
  
  // square all distances (saves us a sqrt later)

  distance_bias*=distance_bias;
  distance_factor*=distance_factor;

  // set default silence_distance

  if (distance_factor == 0)
    silence_distance = 10e99;
  else
    silence_distance = 1/distance_factor;
  
  lisp.get("silence_distance",silence_distance);

  playing=-1; // not playing at the beginning
  latency=0;
}

AmbientSound::AmbientSound(Vector pos, float factor, float bias, float vol, std::string file)
{

  position.x=pos.x;
  position.y=pos.y;

  dimension.x=0;
  dimension.y=0;

  distance_factor=factor*factor;
  distance_bias=bias*bias;
  maximumvolume=vol;
  sample=file;
  
  // set default silence_distance

  if (distance_factor == 0)
    silence_distance = 10e99;
  else
    silence_distance = 1/distance_factor;

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

    float px,py;
    float rx,ry;

    // Player position

    px=Sector::current()->player->get_pos().x;
    py=Sector::current()->player->get_pos().y;

    // Relate to which point in the area

    rx=px<position.x?position.x:
      (px<position.x+dimension.x?px:position.x+dimension.x);
    ry=py<position.y?position.y:
      (py<position.y+dimension.y?py:position.y+dimension.y);

    // calculate square of distance

    float sqrdistance=(px-rx)*(px-rx)+(py-ry)*(py-ry);
    
    sqrdistance-=distance_bias;

    // inside the bias: full volume (distance 0)
    
    if (sqrdistance<0)
      sqrdistance=0;
    
    // calculate target volume - will never become 0

    targetvolume=1/(1+sqrdistance*distance_factor);

    float rise=targetvolume/currentvolume;

    // rise/fall half life?

    currentvolume*=pow(rise,deltat*10);
    currentvolume += 1e-6; // volume is at least 1e-6 (0 would never rise)

    if (playing>=0) {

      // set the volume
      Mix_Volume(playing,(int)(currentvolume*maximumvolume*MIX_MAX_VOLUME));

      if (sqrdistance>=silence_distance && currentvolume<1e-3)
	stop_playing();
      latency=0;
    } else {
      if (sqrdistance<silence_distance) {
	start_playing();
	latency=0;
      }
      else // set a reasonable latency
	latency=(int)(0.001/distance_factor);
      //(int)(10*((sqrdistance-silence_distance)/silence_distance));
    }
  } 

  // heuristically measured "good" latency maximum

  //  if (latency>0.001/distance_factor)
  // latency=
}

void
AmbientSound::draw(DrawingContext &) 
{
}

IMPLEMENT_FACTORY(AmbientSound, "ambient_sound");
