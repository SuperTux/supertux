#ifndef __YETI_STALACTITE_H__
#define __YETI_STALACTITE_H__

#include "stalactite.h"

class YetiStalactite : public Stalactite
{
public:
  YetiStalactite(const lisp::Lisp& lisp);
  virtual ~YetiStalactite();

  void active_action(float elapsed_time);
  void start_shaking();
  bool is_hanging();
};

#endif

