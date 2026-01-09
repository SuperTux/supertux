function initialize()
{
  start_cutscene();
  
  TUX.set_action("big-stand-right");
  pier.set_solid(false);
  
  Tux.deactivate();
  Tux.set_visible(false);
  
  
  ///Player triggers state: "textbox"
	Tux.set_pos(96, 480);
  
  Camera.set_pos(50, 200);
  Camera.set_scale_anchor(1.2, 4);
  
  Effect.sixteen_to_nine(0);
  Effect.fade_in(2);
  
  Camera.scroll_to(200, 500, 10)
  
  wait(2);
  
  TUX.set_velocity(100, -400);
  TUX.set_action("big-jump-right");
  wait(0.5);
  pier.set_solid(true);
  TUX.set_action("big-fall-right");
  wait(0.2);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  wait(0.5);
  TUX.set_action("big-stand-left");
  wait(1);
  TUX.set_action("bend-left");
  wait(1);
  TUX.set_action("big-stand-left");
  wait(0.5);
  
  TUX.set_velocity(100, 0);
  TUX.set_action("big-walk-right");
  
  wait(4);
  Camera.scroll_to(900, Camera.get_y(), 5);
  
  wait(5);
  Camera.scroll_to(1000, Camera.get_y(), 15.8);
  wait(0.8);
  
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  wait(1);
  wait(2);
  TUX.set_action("big-scratch-right");
  wait(1.1);
  TUX.set_action("big-idle-right");
  wait(9.9);
  TUX.set_velocity(200, 0);
  TUX.set_action("big-walk-right");
  wait(1);
  
  Camera.scroll_to(3000, Camera.get_y(), 10);
  
  TUX.set_action("big-run-right");
  TUX.set_velocity(220, 0);
  wait(8);
  
  trigger_state("end_level");
  
  end_cutscene();
}

function textbox()
{
  wait(4);
  
  Text.set_text(_("After quite the long trip, Tux arrived at the new biome."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(4);
  
  Text.set_text(_("Tux was astonished by the scale of this colorful land which was quite diferent from his home."));
  Text.fade_in(0.5);
  wait(5);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("But how was he ever going to find Nolok in this vast forest?"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(3);
  
  Text.set_text(_("Tux thought for a moment and decided to find a place high up to get a better view."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("Maybe he could find some kind of landmark to help him locate Nolok, wherever he may be."));
  Text.fade_in(0.5);
  wait(5);
  Text.fade_out(0.5);
}

function end_level()
{
  //end sequence
  Effect.fade_out(2);
  wait(4);
  stop_music(1);
  wait(1);
  Level.finish(true);
}

state_idx <- 0;
states <- { init=0, start=1, textbox=2, end_level=3};
function trigger_state(state) {
  local idx = states[state];
  if(!idx || idx <= state_idx)
    return;
  state_idx = idx;
  switch(state) {
    case "start":
      initialize();
      break;
	case "textbox":
      textbox();
      break;
	case "end_level":
      end_level();
      break;
  }
}
