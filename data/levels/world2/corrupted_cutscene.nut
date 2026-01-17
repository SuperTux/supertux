function initialize()
{
  start_cutscene();
  
  TUX.set_action("big-walk-right");
  VICIOUS_IVY.set_action("pant-left");
  VICIOUS_IVY.set_visible(false);
  
  Tux.deactivate();
  Tux.set_visible(false);
  
  ///Player triggers state: "textbox"
	Tux.set_pos(960, 33);
  
  Camera.set_pos(0, 360);
  
  Effect.sixteen_to_nine(0);
  Effect.fade_in(2);
  
  Camera.scroll_to(300, Camera.get_y(), 8);
  Camera.set_scale_anchor(1.5, 4);
  TUX.set_velocity(180, 0);
  wait(3.1);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  
  wait(2.8);
  TUX.set_action("big-scratch-right");
  wait(1.2);
  
  TUX.set_velocity(200, 0);
  TUX.set_action("big-walk-right");
  wait(1);
  Camera.scroll_to(920, Camera.get_y(), 2.6);
  wait(2.6);
  Camera.scroll_to(1000, Camera.get_y(), 12.4);
  wait(0.2);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  wait(12.2);
  
  Camera.scale_anchor(1.8, 4.8, 4);
  Camera.scroll_to(1050, Camera.get_y(), 4.8);
  TUX.set_velocity(50, 0);
  TUX.set_action("big-walk-right");
  wait(2.8);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-idle-right");
  wait(2);
  
  Camera.scale_anchor(1.5, 0.2, 4);
  
  VICIOUS_IVY.set_visible(true);
  VICIOUS_IVY.set_velocity(-120, -200);
  play_sound("sounds/hop.ogg");
  TUX.set_velocity(-80, -300);
  TUX.set_action("big-fall-right");
  play_sound("sounds/jump.wav");
  wait(0.3);
  
  VICIOUS_IVY.set_velocity(0, 0);
  wait(0.3);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  // stop_music(10);
  wait(5.7);
  TUX.set_velocity(80, 0);
  TUX.set_action("big-walk-right");
  wait(1.5);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  wait(4);
  
  ///Player triggers state: "root_ambush"
  Tux.set_pos(1056, 33);
  
  // play_music("music/forest/wisphunt.music");
  wait(2);
  Camera.start_earthquake(5, 0.01);
  wait(2.2);
  TUX.set_velocity(-80, -300);
  TUX.set_action("big-fall-right");
  play_sound("sounds/jump.wav");
  wait(0.6);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  wait(0.4);
  TUX.set_velocity(80, -300);
  TUX.set_action("big-fall-left");
  play_sound("sounds/jump.wav");
  wait(0.6);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-left");
  wait(0.6);
  TUX.set_action("big-stand-right");
  
  wait(1.4);
  
  Camera.scroll_to(1400, Camera.get_y(), 1);
  Effect.fade_out(1);
  wait(1.5);
  
  Camera.set_pos(8970, Camera.get_y());
  Effect.fade_in(1);
  
  Camera.set_scale_anchor(1.8, 4);
  Camera.set_pos(8970, Camera.get_y());
  Camera.scroll_to(6870, Camera.get_y(), 8);
  root_rush1.start_moving();
  wait(0.4);
  root_rush2.start_moving();
  wait(0.4);
  root_rush3.start_moving();
  wait(0.4);
  root_rush4.start_moving();
  wait(0.4);
  root_rush5.start_moving();
  wait(4.4);
  
  Effect.fade_out(0.5);
  wait(0.5);

  Camera.set_scale_anchor(1.5, 4);
  Camera.set_pos(1000, Camera.get_y());
  
  Effect.fade_in(0.5);
  
  root_blockade1.fade(1, 0);
  root_blockade2.fade(1, 0);
  root_blockade3.fade(1, 0);
  
  TUX.set_velocity(0, -450);
  TUX.set_action("big-jump-right");
  play_sound("sounds/jump.wav");
  wait(0.4);
  TUX.set_action("big-fall-right");
  wait(0.5);
  TUX.set_velocity(-300, 0);
  TUX.set_action("big-run-left");
  Camera.scroll_to(60, Camera.get_y(), 2.5);
  wait(1);
  
  ///Player triggers state: "root_blockade"
	Tux.set_pos(1152, 33);
  
  wait(2.2);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-left");
  
  TUX.set_velocity(100, -300);
  TUX.set_action("big-fall-left");
  play_sound("sounds/jump.wav");
  wait(0.6);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-duck-left");
  TUX.move(0, 32);
  wait(0.3);
  TUX.set_action("big-stand-left");
  TUX.move(0, -32);
  wait(0.2);
  TUX.set_action("big-stand-right");
  wait(0.2);
  TUX.set_action("big-stand-left");
  wait(0.2);
  Camera.scroll_to(4600, Camera.get_y(), 18);
  TUX.set_action("big-stand-right");
  wait(0.2);
  TUX.set_velocity(300, 0);
  TUX.set_action("big-run-right")
  
  root_obstacle1.fade(1, 0);
  VICIOUS_IVY.set_velocity(-60, 0);
  VICIOUS_IVY.set_action("left");
  
  wait(2.2);
  TUX.set_velocity(300, -450);
  TUX.set_action("big-jump-right");
  play_sound("sounds/jump.wav");
  wait(0.4);
  TUX.set_action("big-fall-right");
  wait(0.5);
  TUX.set_velocity(300, 0);
  TUX.set_action("big-run-right");
  wait(0.8);
  
  ///Player triggers state: "root_obstacle"
	Tux.set_pos(1248, 33);
  
  wait(0.8);
  
  ///Tux bounces due to impact of first root obstacle.
  
  TUX.set_velocity(300, -300);
  TUX.set_action("big-fall-right");
  play_sound("sounds/jump.wav");
  wait(0.6);
  TUX.set_velocity(300, 0);
  TUX.set_action("big-run-right");
  wait(1.2);
  
  ///Tux jumps over second root obstacle.
  
  TUX.set_velocity(300, -550);
  TUX.set_action("big-jump-right");
  play_sound("sounds/jump.wav");
  wait(0.6);
  TUX.set_action("big-fall-right");
  wait(0.5);
  TUX.set_velocity(300, 0);
  TUX.set_action("big-run-right");
  wait(0.6);
  
  ///Tux jumps over third root obstacle.
  
  TUX.set_velocity(300, -500);
  TUX.set_action("big-jump-right");
  play_sound("sounds/jump.wav");
  wait(0.45);
  TUX.set_action("big-fall-right");
  wait(0.55);
  TUX.set_velocity(300, 0);
  TUX.set_action("big-run-right");
  wait(2);
  
  ///Tux jumps over fourth root obstacle.
  
  fog.fade_color(0.6, 0.5, 0.8, 1, 2.85);
  
  TUX.set_velocity(300, -500);
  TUX.set_action("big-jump-right");
  play_sound("sounds/jump.wav");
  wait(0.45);
  TUX.set_action("big-fall-right");
  wait(0.5);
  TUX.set_velocity(300, 0);
  TUX.set_action("big-run-right");
  wait(0.8);
  
  ///Tux jumps over fifth root obstacle.
  
  TUX.set_velocity(300, -450);
  TUX.set_action("big-jump-right");
  play_sound("sounds/jump.wav");
  wait(0.4);
  TUX.set_action("big-fall-right");
  wait(0.5);
  TUX.set_velocity(300, 0);
  TUX.set_action("big-run-right");  
  
  trigger_state("end_level");
  end_cutscene();
}

function textbox()
{
  wait(3);
  Text.set_text(_("Never had Tux been missing sunlight more than ever before."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2.2);
  
  Text.set_text(_("Since the rain had stopped and the sky was clear, Tux hoped to have found a nice, safe place to take a short break."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(6);
  
  Text.set_text(_("Just as he was about to rest, the sudden sound of rustling leaves from a bush nearby startled Tux."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2.5);
  
  Text.set_text(_("He went to investigate and got startled by a Vicious Ivy, tumbling out of the bushes!"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("However, this one looked different. Its usual bright green leaves seemed to have turned an ailing purple."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("Before Tux even had the chance to think about what had happened to the poor thing, the ground began to shake violently..."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1.6);
  
  Text.set_text(_("Massive purple roots burst through the ground, followed by an onslaught of many more coming closer and closer!"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(11);
  
  Text.set_text(_("Tux panicked! Whatever these roots were, they were covering everything. Where could he go?"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1.5);
  
  // l10n: "her" refers to Penny.
  Text.set_text(_("But he cannot give up now! Tux could not leave her behind. Not in a place being overrun by these violent roots."));
  Text.fade_in(0.5);
  wait(3.5);
  Text.fade_out(0.5);
  
  wait(1.5);
  
  Text.set_text(_("He must rescue Penny! He must get to that tower!"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
}

function root_ambush()
{
  wait(2.5);
  root_ambush1.start_moving();
  wait(1.5);
  root_ambush2.start_moving();
  wait(1);
  root_ambush3.start_moving();
}

function root_blockade()
{
  root_blockade3.start_moving();
  wait(0.8);
  root_blockade4.start_moving();
  wait(0.5);
  root_blockade5.start_moving();
}

function root_obstacle()
{
  root_obstacle2.start_moving();
  wait(3.5);
  root_obstacle3.start_moving();
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
states <- { init=0, start=1, textbox=2, root_ambush=3, root_blockade=4, root_obstacle=5, end_level=6 };
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
	case "root_ambush":
      root_ambush();
      break;
	case "root_blockade":
      root_blockade();
      break;
	case "root_obstacle":
      root_obstacle();
      break;
    case "end_level":
      end_level();
      break;
  }
}
