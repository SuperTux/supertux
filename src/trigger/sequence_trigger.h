#ifndef __SEQUENCE_TRIGGER_H__
#define __SEQUENCE_TRIGGER_H__

#include "trigger_base.h"
#include "serializable.h"

class SequenceTrigger : public TriggerBase, public Serializable
{
public:
  SequenceTrigger(LispReader& reader, const std::string& sequence);
  SequenceTrigger(const Vector& pos, const std::string& sequence);
  ~SequenceTrigger();
 
  void write(LispWriter& writer);
  void event(Player& player, EventType type);
  
private:
  EventType triggerevent;
  std::string sequence_name;
};

#endif

