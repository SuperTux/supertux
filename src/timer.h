#ifndef __SUPERTUX_TIMER_H__
#define __SUPERTUX_TIMER_H__

extern float global_time;

/**
 * new simpler timer designed to be used in the update functions of objects
 */
class Timer2 // TODO rename later
{
public:
  Timer2();
  ~Timer2();

  /** start the timer with the given period. If cyclic=true then the timer willl
   * be reset after each period.
   * Set period to zero if you want to disable the timer.
   */
  void start(float period, bool cyclic = false);
  /** returns true if a period (or more) passed */
  bool check();

  /** returns the period of the timer or 0 if it isn't started */
  float get_period() const
  { return period; }
  float get_timeleft() const
  { return period - (global_time - cycle_start); }
  float get_timegone() const
  { return global_time - cycle_start; }
  bool started() const
  { return period != 0 && get_timeleft() > 0; }

private:
  float period;
  float cycle_start;
  bool cyclic;
};

#endif

