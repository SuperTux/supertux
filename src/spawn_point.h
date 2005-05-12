#ifndef __SPAWN_POINT_H__
#define __SPAWN_POINT_H__

#include <string>
#include "math/vector.h"
#include "lisp/lisp.h"

class SpawnPoint
{
public:
    SpawnPoint();
    SpawnPoint(const SpawnPoint& other);
    SpawnPoint(const lisp::Lisp* lisp);

    std::string name;
    Vector pos;
};

#endif

