#ifndef __GAMEOBJECT_HPP__
#define __GAMEOBJECT_HPP__

#include <string>

class DisplayManager;

/**
 * Base class for all game objects. This contains functions for:
 *  -querying the actual type of the object
 *  -a flag that indicates if the object wants to be removed. Objects with this
 *   flag will be removed at the end of each frame. This is alot safer than
 *   having some uncontrollable "delete this" in the code.
 *  -an action function that is called once per frame and allows the object to
 *   update it's state.
 * 
 * Most GameObjects will also implement the DrawableObject interface so that
 * they can actually be drawn on screen.
 */
class _GameObject // TODO rename this once the game has been converted
{
public:
  _GameObject();
  virtual ~_GameObject();

  /** returns the name of the objecttype, this is mainly usefull for the editor.
   * For the coding part you should use C++ RTTI (ie. typeid and dynamic_cast)
   * instead.
   */
  virtual std::string type() const = 0;
  /** This function is called once per frame and allows the object to update
   * it's state. The elapsed_time is the time since the last frame and should be
   * the base for all timed things.
   */
  virtual void action(float elapsed_time) = 0;

  /** returns true if the object is not scheduled to be removed yet */
  bool is_valid() const
  { return !wants_to_die; }
  /** schedules this object to be removed at the end of the frame */
  void remove_me()
  { wants_to_die = true; }
  
private:
  /** this flag indicates if the object should be removed at the end of the
   * frame
   */
  bool wants_to_die;
};

#endif

