#ifndef __CAMERA_H__
#define __CAMERA_H__

namespace Scripting
{

class Camera
{
public:
#ifndef SCRIPTING_API
    Camera();
    ~Camera();
#endif

    /** Shake the camera */
    void shake(float speed, float x, float y);
    /** Set camera to a specific coordinate */
    void set_pos(float x, float y);
    /** Set camera to a specific mode, can be "normal", "manual" */
    void set_mode(const std::string& mode);
};

}

#endif

