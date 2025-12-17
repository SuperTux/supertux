function initialize()
{
  start_cutscene();
  
  TUX.set_action("big-walk-right");
  YETI.set_action("stand-right");
  YETI.set_visible(false);
  
  Tux.deactivate();
  Tux.set_visible(false);
  
  
  ///Player triggers state: "textbox"
  Tux.set_pos(256, 65);
  
  Camera.set_pos(100, 550);
  Camera.set_scale_anchor(1.5, 4);
  
  Effect.sixteen_to_nine(0);
  Effect.fade_in(2);
  
  Camera.scroll_to(600, Camera.get_y(), 19);
  Camera.scale_anchor(1.2, 19, 4);
  TUX.set_velocity(150, 0);
  wait(7.8);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-idle-right");
  wait(11.2);
  Camera.scroll_to(2000, Camera.get_y(), 6);
  TUX.set_velocity(250, 0);
  TUX.set_action("big-run-right");
  wait(1.7);
  TUX.set_action("big-slide-right");
  TUX.move(0, 32);
  TUX.set_velocity(350, 0);
  wait(4.3);
  Camera.scroll_to(2050, Camera.get_y(), 2.7);
  wait(2.2);
  
  ///Player triggers state: "yeti_reveal"
  Tux.set_pos(320, 65);
  
  wait(0.5);
  Camera.scroll_to(750, Camera.get_y(), 0.5);
  wait(0.5);
  Camera.scroll_to(650, Camera.get_y(), 16);
  
  wait(13);
  
  trigger_state("end_level");
  
  end_cutscene();
}

function textbox()
{
  wait(4);
  
  Text.set_text(_("After an entire day of navigating Icy Island, Tux had now almost reached the supposed castle of Nolok."));
  Text.fade_in(0.5);
  wait(5.5);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("The lack of sleep during the night may not have been ideal but he was determined to keep going!"));
  Text.fade_in(0.5);
  wait(5.5);
  Text.fade_out(0.5);
  
  wait(3);
  
  Text.set_text(_("Not wanting to waste any more time, Tux continued on, sliding down the hill he was on into the icy valley up ahead."));
  Text.fade_in(0.5);
  wait(5.5);
  Text.fade_out(0.5);
  
  wait(3.3);
  
  Text.set_text(_("Unbeknownst to him, a Yeti seemed very keen on his presence as he kept a close eye on Tux as he continued his journey."));
  Text.fade_in(0.5);
  wait(5.5);
  Text.fade_out(0.5);
  
  wait(1.3);
  
  Text.set_text(_("The yeti let out a loud roar and proceeded to follow the little penguin."));
  Text.fade_in(0.5);
  wait(5);
  Text.fade_out(0.5);
}

function yeti_reveal()
{
  play_sound("sounds/yeti_roar.wav");
  wait(0.8);
  YETI.set_visible(true);
  YETI.set_pos(1088, 256);
  YETI.set_velocity(120, 0);
  YETI.set_action("jump-right");
  wait(1.15);
  YETI.set_velocity(0, 0);
  YETI.set_action("stomp-right");
  play_sound("sounds/thud.ogg");
  wait(0.5);
  YETI.set_action("stand-right");
  wait(4.4);
  play_sound("sounds/yeti_gna.wav");
  YETI.set_action("leap-right");
  wait(0.5);
  YETI.set_action("stand-right");
  wait(1.5);
  YETI.set_velocity(320, 0);
  YETI.set_action("walk-right");
  wait(1.5);
  YETI.set_velocity(300, -450);
  YETI.set_action("jump-right");
  play_sound("sounds/yeti_roar.wav");
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
states <- { init=0, start=1, textbox=2, yeti_reveal=3 end_level=4};
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
	case "yeti_reveal":
      yeti_reveal();
      break;
	case "end_level":
      end_level();
      break;
  }
}
