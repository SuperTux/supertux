#include "yeti_stalactite.h"

static const float SHAKE_TIME = .8;

YetiStalactite::YetiStalactite(const lisp::Lisp& lisp)
  : Stalactite(lisp)
{
}

YetiStalactite::~YetiStalactite()
{
}

void
YetiStalactite::start_shaking()
{
  timer.start(SHAKE_TIME);
  state = STALACTITE_SHAKING;
}

bool
YetiStalactite::is_hanging()
{
  return state == STALACTITE_HANGING;
}

void
YetiStalactite::active_action(float elapsed_time)
{
  if(state == STALACTITE_HANGING)
    return;

  Stalactite::active_action(elapsed_time);
}

IMPLEMENT_FACTORY(YetiStalactite, "yeti_stalactite")
