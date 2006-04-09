#include <config.h>

#include <stdexcept>
#include <iostream>
#include "spawn_point.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "msg.hpp"

SpawnPoint::SpawnPoint()
{}

SpawnPoint::SpawnPoint(const SpawnPoint& other)
    : name(other.name), pos(other.pos)
{}

SpawnPoint::SpawnPoint(const lisp::Lisp* slisp)
{
    pos.x = -1;
    pos.y = -1;
    lisp::ListIterator iter(slisp);
    while(iter.next()) {
        const std::string& token = iter.item();
        if(token == "name") {
            iter.value()->get(name);
        } else if(token == "x") {
            iter.value()->get(pos.x);
        } else if(token == "y") {
            iter.value()->get(pos.y);
        } else {
            msg_warning << "unknown token '" << token << "' in SpawnPoint" << std::endl;
        }
    }

    if(name == "")
        throw std::runtime_error("No name specified for spawnpoint");
    if(pos.x < 0 || pos.y < 0)
        throw std::runtime_error("Invalid coordinates for spawnpoint");
}
