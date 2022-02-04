//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_TRIGGER_TRIGGER_BASE_HPP
#define HEADER_SUPERTUX_TRIGGER_TRIGGER_BASE_HPP

#include <list>

#include "supertux/moving_object.hpp"
#include "supertux/object_remove_listener.hpp"

#include "sprite/sprite_ptr.hpp"
#include "video/layer.hpp"

class Player;

/** This class is the base class for all objects you can interact with
    in some way. There are several interaction types defined like
    touch and activate */
class TriggerBase : public MovingObject,
                    public ObjectRemoveListener
{
public:
  enum EventType {
    EVENT_TOUCH,     /**< Object came into contact */
    EVENT_LOSETOUCH, /**< Lost contact with object */
    EVENT_ACTIVATE   /**< Action button pressed    */
  };

public:
  TriggerBase(const ReaderMapping& mapping);
  TriggerBase();
  ~TriggerBase() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  /** Receive trigger events */
  virtual void event(Player& player, EventType type) = 0;

  /** Called by GameObject destructor of an object in losetouch_listeners */
  virtual void object_removed(GameObject* object) override;

  virtual int get_layer() const override { return LAYER_TILES + 1; }

private:
  SpritePtr m_sprite;
  bool m_lasthit;
  bool m_hit;

  /** Players that will be informed when we lose touch with them */
  std::vector<Player*> m_losetouch_listeners;

private:
  TriggerBase(const TriggerBase&) = delete;
  TriggerBase& operator=(const TriggerBase&) = delete;
};

#endif

/* EOF */
