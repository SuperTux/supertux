#ifndef __SECRETAREA_TRIGGER_H__
#define __SECRETAREA_TRIGGER_H__

#include "trigger_base.h"
#include "serializable.h"
#include "resources.h"
#include "video/drawing_context.h"
#include "app/globals.h"
#include "timer.h"

#define MESSAGE_TIME 3

class SecretAreaTrigger : public TriggerBase, public Serializable
{
public:
  SecretAreaTrigger(LispReader& reader);
  SecretAreaTrigger(const Vector& pos);
  ~SecretAreaTrigger();
 
  void write(LispWriter& writer);
  void event(Player& player, EventType type);
  void draw(DrawingContext& context);
  
private:
  std::string message;
  Timer2 message_timer;
};

#endif

