#ifndef __SCRIPTING_AMBIENT_SOUND_H__
#define __SCRIPTING_AMBIENT_SOUND_H__

namespace Scripting
{

class AmbientSound
{
public:
#ifndef SCRIPTING_API
  virtual ~AmbientSound()
  {}
#endif

  virtual void set_pos(float x, float y) = 0;
  virtual float get_pos_x() = 0;
  virtual float get_pos_y() = 0;
};

}

#endif

