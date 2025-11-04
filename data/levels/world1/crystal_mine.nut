function level_intro()
{
  start_cutscene();
  
  Tux.use_scripting_controller(true);
  Camera.set_mode("manual");
  Effect.sixteen_to_nine(1);
  Tux.do_scripting_controller("right", true);
  Camera.scroll_to(3800, 340, 15.2);
  wait(2.75);
  //Tux.do_scripting_controller("right", false);
  wait(6.5);
  Tux.do_scripting_controller("right", true);
  wait(1.2);
  //Tux.do_scripting_controller("jump", true);
  wait(0.5);
  //Tux.do_scripting_controller("jump", false);
  wait(3);
  Tux.do_scripting_controller("right", false);
  Tux.set_pos(5088, 608);
  Effect.fade_out(1);
  wait(1.5);
  Camera.set_pos(5120, 320);
  Camera.set_scale(1.2);
  settings.set_ambient_light(0.5, 0.4, 0.55);
  vignette.set_image("images/background/misc/vignette.png");
  wait(0.5);
  Effect.fade_in(1);
  Tux.do_scripting_controller("right", true);
  wait(1.8);
  //Tux.do_scripting_controller("right", false);
  wait(2);
  SWITCH.set_action("turnon");
  play_sound("sounds/switch.ogg");
  wait(0.2);
  SWITCH.set_action("turnoff");
  lift_top.goto_node(1);
  lift_bottom.goto_node(1);
  wait(1);
  SWITCH.set_action("off");
  Camera.scroll_to(Camera.get_x(), 736, 10);
  wait(5.5);
  Effect.fade_out(1);
  wait(3);
  Tux.use_scripting_controller(false);
  Level.spawn("crystal_mine", "start");
  
  end_cutscene();
}

function level_outro()
{
  start_cutscene();
  
  Effect.sixteen_to_nine(0);
  Camera.move(0, -150);
  Camera.set_scale(1.2);
  Tux.use_scripting_controller(true);
  Effect.fade_in(1);
  Camera.scroll_to(Camera.get_x(), 380, 6);
  lift_top.start_moving();
  lift_bottom.start_moving();
  wait(6);
  Tux.do_scripting_controller("right", true);
  wait(0.5);
  Camera.scroll_to(600, 250, 5);
  wait(1);
  //Tux.do_scripting_controller("jump", true);
  wait(0.5);
  //Tux.do_scripting_controller("jump", false);
  wait(1);
  Effect.fade_out(1);
  wait(1.5);
  vignette.set_image("images/background/misc/transparent_up.png");
  Tux.set_pos(3456, 576);
  Camera.set_pos(3550, 300);
  Camera.scale_anchor(1, 8, 4);
  Tux.do_scripting_controller("right", false);
  Camera.scroll_to(5400, 450, 20);
  wait(0.5);
  Effect.fade_in(1);
  wait(1.5);
  Tux.do_scripting_controller("right", true);
  wait(4.5);
  //Tux.do_scripting_controller("right", false);
  wait(4);
  Tux.do_scripting_controller("right", true);
  wait(3.3);
  //Tux.do_scripting_controller("jump", true);
  wait(0.2);
  //Tux.do_scripting_controller("jump", false);
  //Tux.do_scripting_controller("right", false);
  wait(10);
  Camera.scroll_to(7250, Camera.get_y(), 15);
  wait(10);
  
  trigger_state("end_level");
  
  end_cutscene();
  
}

function textbox_intro()
{
  wait(3.5);
  
  Text.set_text(_("After a long swim across the antarctic waters, Tux arrived at the Glacier Isle mentioned in Nolok's note."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("A massive shaft lay before him, leading down into the aforementioned Crystal Mine."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(5.5);
  
  Text.set_text(_("Still carrying a spark of doubt that this was not a trap, Tux slowly descended into the depths once more..."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
}

function textbox_outro()
{
  wait(4);
  
  Text.set_text(_("At last, Tux made it through the crystal mine and found himself on the opposite end of the Glacier Isle."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(4.5);
  
  Text.set_text(_("As he was about to exit the mine, Tux was internally hoping that all Nolok wrote in the note was true."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("To his relief, he did find a boat still docked. With it, he could sail up north, to find Nolok and save Penny."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(5);
  
  Text.set_text(_("It seemed the map in the note wasn't a trap after all."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(8);
  
  Text.set_text(_("The more he thought about it, the less he was sure about Nolok's motivations. Why would he leave clues behind?"));
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
states <- { init=0, intro=1, outro=2, textbox_intro=3, textbox_outro=4, end_level=5 };
function trigger_state(state) {
  local idx = states[state];
  if(!idx || idx <= state_idx)
    return;
  state_idx = idx;
  switch(state) {
    case "intro":
      level_intro();
      break;
	case "outro":
      level_outro();
      break;
	case "textbox_intro":
      textbox_intro();
      break;
	case "textbox_outro":
      textbox_outro();
      break;
	case "end_level":
      end_level();
      break;
  }
}
