#ifndef __PLAYER_H__
#define __PLAYER_H__

namespace Scripting
{

class Player
{
public:
#ifndef SCRIPTING_API
  virtual ~Player()
  {}
#endif

#if 0
  /**
   * Set tux bonus.
   * This can be "grow", "fireflower" or "iceflower" at the moment
   */
  virtual void set_bonus(const std::string& bonus) = 0;
  /**
   * Give tux another life
   */
  virtual void add_life() = 0;
  /**
   * Give tux more coins
   */
  virtual void add_coins(int count) = 0;
#endif
  /**
   * Make tux invicible for a short amount of time
   */
  virtual void make_invincible() = 0;
  /**
   * Deactivate user input for Tux
   */
  virtual void deactivate() = 0;
  /**
   * Give control back to user
   */
  virtual void activate() = 0;
  /**
   * Make Tux walk
   */
  virtual void walk(float speed) = 0;
};

}

#endif

