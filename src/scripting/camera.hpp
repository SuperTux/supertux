#ifndef __CAMERA_H__
#define __CAMERA_H__

#ifndef SCRIPTING_API
class Camera;
typedef Camera _Camera;
#endif

namespace Scripting
{

class Camera
{
public:
#ifndef SCRIPTING_API
  Camera(_Camera* camera);
  ~Camera();
#endif

  /** Shake the camera */
  void shake(float speed, float x, float y);
  /** Set camera to a specific coordinate */
  void set_pos(float x, float y);
  /** Set camera to a specific mode, can be "normal", "manual" */
  void set_mode(const std::string& mode);
  /** Scroll camera to position x,y in scrolltime seconds */
  void scroll_to(float x, float y, float scrolltime);

#ifndef SCRIPTING_API
  _Camera* camera;
#endif
};

}

#endif

