#ifndef __SEQUENCE_TRIGGER_H__
#define __SEQUENCE_TRIGGER_H__

#include "trigger_base.h"
#include "serializable.h"

class SequenceTrigger : public TriggerBase, public Serializable
{
public:
  SequenceTrigger(const lisp::Lisp& reader);
  SequenceTrigger(const Vector& pos, const std::string& sequence);
  ~SequenceTrigger();
 
  void write(lisp::Writer& writer);
  void event(Player& player, EventType type);
  
private:
  EventType triggerevent;
  std::string sequence_name;
};

#endif

