#ifndef __FIREWORKS_H__
#define __FIREWORKS_H__

#include "video/drawing_context.h"
#include "special/game_object.h"
#include "timer.h"

class Fireworks : public SuperTux::GameObject
{
public:
    Fireworks();
    ~Fireworks();
    
    virtual void action(float elapsed_time);
    virtual void draw(SuperTux::DrawingContext& context);

private:
    Timer2 timer;
};

#endif

