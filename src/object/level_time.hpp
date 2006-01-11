#ifndef __LEVELTIME_H__
#define __LEVELTIME_H__

#include "game_object.hpp"
#include "timer.hpp"
#include "lisp/lisp.hpp"

class LevelTime : public GameObject
{
public:
    LevelTime(const lisp::Lisp& reader);
    virtual ~LevelTime();

    void update(float elapsed_time);
    void draw(DrawingContext& context);
    float get_level_time();
    float get_remaining_time();

private:
    Timer time_left;
};

#endif

