#ifndef __SECRETAREA_TRIGGER_H__
#define __SECRETAREA_TRIGGER_H__

#include "trigger_base.h"
#include "serializable.h"
#include "resources.h"
#include "video/drawing_context.h"
#include "app/globals.h"
#include "timer.h"

class SecretAreaTrigger : public TriggerBase, public Serializable
{
public:
  SecretAreaTrigger(const lisp::Lisp& reader);
  SecretAreaTrigger(const Vector& pos);
  ~SecretAreaTrigger();
 
  void write(lisp::Writer& writer);
  void event(Player& player, EventType type);
  void draw(DrawingContext& context);
  
private:
  std::string message;
  Timer2 message_timer;
  bool message_displayed;
};

#endif

