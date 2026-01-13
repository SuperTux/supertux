function initialize()
{
  start_cutscene();

  trigger_state("textbox");
  
  // TUX.set_action("big-duck-right");
  ///GHOSTTREE.set_action("decayed");
  
  Tux.deactivate();
  wait(0.05);
  Tux.do_duck();
  Tux.disable_fancy_idling();
  
  Camera.set_pos(420, 135);
  
  Effect.sixteen_to_nine(0);
  Effect.fade_in(2);
  
  Camera.set_scale_anchor(3, 4);
  Camera.scale_anchor(2, 36, 4);
  
  corrupted_solid.fade(0, 20);
  wait(4);
  Tux.do_standup();
  // TUX.set_action("big-stand-right");
  wait(2);
  Camera.scroll_to(250, Camera.get_y(), 0.5);
  Tux.set_dir(false);
  // TUX.set_action("big-stand-left");
  wait(0.5);
  Camera.scroll_to(240, Camera.get_y(), 3);
  wait(3);
  Camera.scroll_to(580, Camera.get_y(), 0.5);
  Tux.set_dir(true);
  // TUX.set_action("big-stand-right");
  wait(0.5);
  Camera.scroll_to(590, Camera.get_y(), 3);
  wait(3);
  Camera.scroll_to(480, Camera.get_y(), 22);
  wait(2);
  Tux.walk(120);
  // TUX.set_action("big-walk-right");
  // TUX.set_velocity(120, 0);
  wait(2);
  Tux.walk(0);
  // TUX.set_action("big-stand-right");
  // TUX.set_velocity(0, 0);
  wait(3);
  Tux.set_dir(false);
  // TUX.set_action("big-stand-left");
  wait(1);
  Tux.set_dir(true);
  // TUX.set_action("big-stand-right");
  wait(2);
  Tux.set_dir(false);
  Tux.walk(-100);
  // TUX.set_action("big-walk-left");
  // TUX.set_velocity(-100, 0);
  wait(1.2);
  Tux.walk(0);
  // TUX.set_action("big-stand-left");
  // TUX.set_velocity(0, 0);
  wait(3);
  Tux.set_dir(true);
  // TUX.set_action("big-stand-right");
  wait(2);
  Tux.set_dir(false);
  // TUX.set_action("big-stand-left");
  wait(5);
  
  Effect.fade_out(1);
  wait(1.5);
  Tux.set_pos(2150, 480);
  // TUX.set_pos(2250, 480);
  Camera.set_pos(1980, 135);
  Camera.scroll_to(4460, 90, 32);
  Tux.set_dir(true);
  // TUX.set_action("big-walk-right");
  // TUX.set_velocity(120, 0);
  Tux.walk(120);
  wait(0.5);
  Effect.fade_in(1);
  
  wait(12);
  play_sound("sounds/jump.wav");
  Tux.do_jump(-450);
  // TUX.set_action("big-jump-right");
  // TUX.set_velocity(120, -450);
  wait(0.5);
  // TUX.set_action("big-fall-right");
  wait(0.5);
  // TUX.set_action("big-walk-right");
  wait(5);
  play_sound("sounds/jump.wav");
  // TUX.set_action("big-jump-right");
  // TUX.set_velocity(120, -450);
  Tux.do_jump(-450);
  wait(0.5);
  // TUX.set_action("big-fall-right");
  wait(0.2);
  // TUX.set_action("big-walk-right");
  wait(2.3);
  play_sound("sounds/jump.wav");
  // TUX.set_action("big-jump-right");
  // TUX.set_velocity(120, -500);
  Tux.do_jump(-450);
  wait(0.6);
  // TUX.set_action("big-fall-right");
  wait(0.2);
  // TUX.set_action("big-walk-right");
  
  trigger_state("end_level");
  
  end_cutscene();
}

function textbox()
{
  wait(5.5);
  Text.set_text(_("Was it over? Was the nightmare finally over?"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(4);
  
  Text.set_text(_("Tux could not believe his eyes, but it seemed so! The roots began retreating. The ground looked more vital again."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(5);
  
  Text.set_text(_("Though as much as he was ecstatic about seeing the land restored, beginning to heal... where to now?"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2.2);
  
  Text.set_text(_("The forest, the tower... those were the only leads Tux had. He had no clue where Nolok could be hiding, if he even was here to begin with. Or rather anymore."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(6);
  
  Text.set_text(_("There was no one around Tux could ask for help. All but the remains of corrupted husks littering the ground as moved along in thought. No living creature in sight."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2.2);
  
  Text.set_text(_("Hoping to discover some kind of clue - anything! - Tux's best bet was to continue exploring this side of the forest, as destroyed as it may be."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2.2);
  Text.set_text(_("There must be something, somewhere, able to help him to find Nolok! Right...?"));
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
states <- { init=0, start=1, textbox=2, end_level=3 };
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
