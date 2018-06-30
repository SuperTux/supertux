 #ifndef SUPERTUX_MATH_BROADPHASE_HPP_INCLUDED
 #define SUPERTUX_MATH_BROADPHASE_HPP_INCLUDED
#include <vector>
#include <functional>
#include "math/rectf.hpp"
#include <set>
#include <vector>
#include "supertux/moving_object.hpp"
/**
 *  This class is an interface for any class implementing the a collision Broadphase.
 *  All methods are pure virtual and have to be implemented.
 *
 */

class collision_broadphase {
public:
  /**
   *  Insert an object into the data structure.
   *  @param aabb The rectangle in which obj lies.
   *  @param obj The object (e.g. Tux, a snowball, etc.)
   */
  virtual void insert(const Rectf& aabb,MovingObject* obj){};
  /**
   *  Lists all objects o which might collide with r and for which collision_ok(o) == true.
   *  @param r The rectangle to query.
   *  @param collision_ok A lambda function to filter out some collisions.
   *  @param fill The set to fill with the possibly colliding objects.
   */
  virtual void search(const Rectf& r, std::function<void()> collision_ok, std::set< MovingObject* >& fill){};
  /**
   *  TODO Define method for raycast
   *
   */
   //virtual void raycast();
   /**
    * Checks if any objects collide with r.
    * @param r The rectangle to check.
    * If insert runs in time O(f) then collides should also run in O(f).
    */
    //virtual bool collides(Rectf r);
   /**
    * Allows for fast insertion of all objects
    * @param object_list List of moving objects to insert.
    * Performace: If insert runs in O(f) then calling bulk_insert with n Elements should run in
    * O(f*n). Exception: Faster query time can be guaranteed.
    */
    void bulk_insert(std::vector<Rectf>& aabbs, std::vector<MovingObject*>& object_list){

      for(size_t i = 0; i < object_list.size(); i++)
      {
        insert(aabbs[i],object_list[i]);
      }
    }
    /**
     *  Deletes obj from the tree.
     *  @param obj The object to insert
     *  @returns Boolean indicating wether obj was found an deleted or not.
     */
    virtual bool remove(MovingObject* obj){return false;}
    /**
     *  Clears the data structure and removes all items.
     *  Performance guarentee: Not worse than remove for every object.
     */
     virtual void clear(){};

     virtual ~collision_broadphase() = default;
};
#endif
