#ifndef __OBJECT_REMOVE_LISTENER_H__
#define __OBJECT_REMOVE_LISTENER_H__

namespace SuperTux
{

class GameObject;

class ObjectRemoveListener
{
public:
  virtual ~ObjectRemoveListener()
  { }

  virtual void object_removed(GameObject* object) = 0;
};

}

#endif

