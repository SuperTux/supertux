function initialize()
{
  start_cutscene();
  
  //TUX.set_action("bend-right");
  TUX.set_action("big-idle-right");
  YETI.set_action("busted-left");
  
  Tux.deactivate();
  Tux.set_visible(false);
  
  Camera.set_pos(110, 180);
  
  Effect.sixteen_to_nine(0);
  Effect.fade_in(2);
  
  Camera.scroll_to(50, Camera.get_y(), 30.2);
  Camera.set_scale_anchor(1.5, 4);
  
  wait(2);
  
  ///Player triggers state: "textbox"
  Tux.set_pos(320, 32);
  
  wait(6);
  
  TUX.set_velocity(-100, -300);
  TUX.set_action("big-fall-right");
  play_sound("sounds/jump.wav");
  play_sound("sounds/yeti_roar.wav");
  YETI.set_action("leap-left");
  YETI.enable_gravity(true);
  YETI.set_solid(true);
  YETI.set_velocity(0, -350);
  wait(0.6);
  YETI.set_action("stand-left");
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");  
  wait(4);
  play_sound("sounds/yeti_gna.wav");
  wait(5);
  play_sound("sounds/yeti_roar.wav");
  wait(1.5);
  play_sound("sounds/yeti_roar.wav");
  YETI.set_action("walk-left");
  YETI.set_solid(false);
  YETI.enable_gravity(false);
  YETI.set_velocity(-320, 0);
  wait(0.5);
  TUX.set_velocity(100, -300);
  TUX.set_action("big-fall-left");
  play_sound("sounds/jump.wav");
  wait(0.6);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-left");
  YETI.set_solid(true);
  wait(7);
  TUX.set_action("big-walk-right");
  TUX.set_velocity(200, 0);
  wait(3);
  Camera.scroll_to(600, Camera.get_y(), 3);
  wait(1);
  Effect.fade_out(1);
  wait(1.5);
  
  ///Transition to next scene.
  
  Camera.set_pos(3200, 250);
  
  Effect.sixteen_to_nine(0);
  
  wait(0.5);
  
  Camera.scroll_to(3350, 280, 9.8);
  Camera.set_scale_anchor(1.3, 4);
  
  Effect.fade_in(1);
  TUX.set_pos(3520, 608);
  TUX.set_velocity(100, 0);
  TUX.set_action("big-walk-right");
  wait(3);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-idle-right");
  
  wait(4.8);
  
  TUX.set_velocity(320, 0);
  TUX.set_action("big-run-right");
  wait(0.5);
  
  play_sound("sounds/jump.wav");
  TUX.set_action("big-jump-right");
  TUX.set_velocity(320, -500);
  wait(0.5);  
  TUX.set_action("big-fall-right");
  wait(0.4);
  
  Camera.scroll_to(6000, 270, 12);
  
  TUX.set_action("big-run-right");
  wait(0.3);
  play_sound("sounds/jump.wav");
  TUX.set_action("big-jump-right");
  TUX.set_velocity(320, -500);
  wait(0.5);
  TUX.set_action("big-fall-right");
  wait(0.65);
  play_sound("sounds/splash.wav");
  TUX.set_action("big-boost-right");
  TUX.enable_gravity(false);
  TUX.set_solid(false);
  TUX.set_velocity(500, 0);
  
  wait(7);
  
  trigger_state("end_level");
  end_cutscene();
}

function textbox()
{
  Text.set_text(_("He did it! He actually did it! Tux had beaten the mighty yeti!"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("While it did not take long for the yeti to get up again, to Tux's relief, he did not throw hands again."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("The yeti was furious however! He could not understand how a tiny penguin like Tux could have beaten him in a fight."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("Almost as if he forgot Tux was there to begin with, the yeti rushed off, grumbling to himself."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("Not sure what to make of it, Tux decided to continue moving onwards."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(5);
  
  Text.set_text(_("As he reached the shore, Tux could already see the Glacier Isle on the horizon."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(3);
  
  Text.set_text(_("If he knew Nolok went over there sooner, he could have been there much faster."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1.5);
  
  Text.set_text(_("Perhaps that was Nolok's goal with the note he left behind..."));
  Text.fade_in(0.5);
  wait(4);
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
