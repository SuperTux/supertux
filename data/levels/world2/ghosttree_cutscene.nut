function initialize()
{
  start_cutscene();
  
  TUX.set_action("big-walk-right");
  
  Camera.set_pos(2100, 120);
  
  Effect.sixteen_to_nine(0);
  Effect.fade_in(2);
  
  Camera.set_scale(1.5);
  Camera.scroll_to(2400, 100, 20);
  
  TUX.set_velocity(100, 0);
  wait(3);
  Text.set_text(_("High on the very top of the dilapidated tower, once again Tux found... nothing."));
  Text.fade_in(0.5);
  wait(1);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  
  wait(3);
  Text.fade_out(0.5);
  wait(1);

  wait(2.2);
  
  Text.set_text(_("He could not believe it. He was shocked. Confused. Was this not where he needed to go?"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2.2);
  
  Text.set_text(_("Did Nolok's hints trick him afterall? Did he walk into a trap?"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2.2);
    
  Camera.start_earthquake(10, 0.05);
  
  Text.set_text(_("Tux had not long to think about all this, the tower began shaking violetly. His assumption prove correct instantly."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);

  wait(1);
  
  trigger_state("outside");
}

function ghosttree()
{
  Camera.set_pos(6145, 3420);
  Camera.set_scale(3.5);
  TUX1.set_action("big-duck-right");
  Effect.fade_in(3);
  Camera.scroll_to(Camera.get_x(), 3400, 12);
  Camera.scale(2.5, 12);
  TUX1.set_pos(6768, 3760);
  TUX1.set_velocity(0, 0);

  Camera.stop_earthquake();
    
  wait(2.2);
  
  Text.set_text(_("He made it! He escaped the collapsing tower just in time as the structure now fully gave in. But the roots were not done yet!"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1.1);

  TUX1.set_pos(TUX1.get_x(), TUX1.get_y() - 32);
  TUX1.set_action("big-stand-right");

  wait(1.1);
  
  Text.set_text(_("This was only the beginning, as he had found the root of the problem... towering over the small penguin."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2.2);

  trigger_state("end_level");
}

function outside()
{
  Effect.sixteen_to_nine(0);
  
  Camera.set_pos(2900, 1900);
  Camera.set_scale(2);
  Effect.fade_in(0.5);
  Camera.scroll_to(Camera.get_x(), 950, 6.5);
  Camera.start_earthquake(8, 0.05);
  wait(0.2);
  root_attack1.start_moving();
  wait(0.6);
  root_attack2.start_moving();
  wait(0.4);
  Text.set_text(_("Tux watched in shock as the roots dug through the tower's walls, faster and faster. He had to get out of there, fast!"));
  Text.fade_in(0.5);
  root_attack3.start_moving();
  wait(4.5);
  Text.fade_out(0.5);
  wait(0.6);
  Effect.fade_out(0.5);
  wait(0.6);
  Camera.set_scale(1.5);
  Camera.set_pos(2400, 100);
  Effect.fade_in(0.5);

  trigger_state("inside");
}

function inside()
{
  invisibleh.set_pos(0, 0);
  thunder.stop();
  rain_sfx.set_volume(0.5);
  castle_bg_wall.set_color(1, 1, 1, 1);
  castle_bg_pillar.set_color(1, 1, 1, 1);
  Effect.sixteen_to_nine(0);
  TUX1.set_action("big-run-left");
  TUX1.set_velocity(-320, 0);
  
  Camera.set_pos(2000, 2450);
  Camera.set_scale(1.5);
  Effect.fade_in(0.5);
  Camera.start_earthquake(6, 0.05);
  Camera.scroll_to(600, Camera.get_y(), 3.5);
  wait(0.2);
  root_ambush1.goto_node(2);
  wait(0.3);
    
  Text.set_text(_("The roots were now everywhere. The tower become more unstable by the second. Slowly, it collapsed in on itself."));
  Text.fade_in(0.5);

  wait(0.7);
  root_ambush2.start_moving();
  wait(0.6);
  TUX1.set_action("big-jump-left");
  play_sound("sounds/jump.wav");
  TUX1.set_velocity(-320, -500);
  wait(0.5);
  TUX1.set_action("big-fall-left");
  wait(0.5);
  TUX1.set_action("big-run-left");
  room_blockade.start_moving();
  
  wait(0.4);
  
  TUX1.set_velocity(-200, 0);
  TUX1.set_action("big-fall-left");
  play_sound("sounds/skid.wav");
  wait(0.4);
  TUX1.set_velocity(-120, 0);
  wait(0.2);
  TUX1.set_velocity(0, 0);
  TUX1.set_action("big-stand-left");
  wait(0.5);
  TUX1.set_action("big-stand-right");
  wait(0.2);
  Text.fade_out(0.5);
  
  TUX1.set_action("big-stand-left");
  wait(0.3);
  TUX1.set_action("big-stand-right");
  root_ambush1.set_node(4);
  wait(1);
  root_ambush1.goto_node(6);
  wait(0.35);
  Camera.scroll_to(700, Camera.get_y(), 0.8);
  TUX1.set_action("big-fall-left");
  play_sound("sounds/jump.wav");
  TUX1.set_velocity(200, -400);
  wait(0.8);

  Text.set_text(_("Tux ran as fast as he could, but the roots would not let him leave as they blocked off the only path to the main entrance."));
  Text.fade_in(0.5);

  Camera.scroll_to(750, Camera.get_y(), 4);
  play_sound("sounds/skid.wav");
  TUX1.set_action("big-skid-right");
  TUX1.set_velocity(-100, 0);
  wait(0.5);
  TUX1.set_action("big-run-right");
  TUX1.set_velocity(320, 0);
  wait(0.4);
  TUX1.set_action("big-jump-right");
  play_sound("sounds/jump.wav");
  TUX1.set_velocity(320, -550);
  wait(0.5);
  TUX1.set_action("big-fall-right");
  wait(0.5);
  TUX1.set_action("big-run-right");
  wait(1);
  Effect.fade_out(0.5);
  wait(1);

  Text.fade_out(0.5);
  
  TUX1.set_velocity(0, 0);
  castle_bg_wall.set_color(1, 1, 1, 0);
  castle_bg_pillar.set_color(1, 1, 1, 0);
  TUX1.set_pos(2752, 2816);
  Camera.set_scale(2);
  Camera.set_pos(2500, 2400);
  Camera.scroll_to(3000, Camera.get_y(), 5);
  Camera.scale_anchor(1.5, 5, 5);
  Effect.fade_in(0.5);
  wait(1.2);
  TUX1.set_velocity(320, 0);
  wait(1.5);

  Text.set_text(_("Across the hallway, on the other side, there was a huge crack in the wall. Tux saw himself forced to take the leap of faith, if he wished to not be crushed."));
  Text.fade_in(0.5);

  TUX1.set_action("big-jump-right");
  play_sound("sounds/jump.wav");
  TUX1.set_velocity(320, -575);
  wait(0.5);
  TUX1.set_action("big-fall-right");

  wait(3.5);

  Text.fade_out(0.5);

  wait(2.2);

  Effect.fade_out(0.5);

  wait(1);

  trigger_state("ghosttree");
  
}

function textbox()
{
  
}

function end_level()
{
  //end sequence
  Effect.fade_out(2);
  wait(2.1);
  end_cutscene();
  Level.finish(true);
}

state_idx <- 0;
states <- { init=0, start=1, ghosttree=2, outside=3, inside=4, end_level=5 };
function trigger_state(state) {
  local idx = states[state];
  if(!idx)
    return;
  state_idx = idx;
  switch(state) {
    case "start":
      initialize();
      break;
	case "ghosttree":
      ghosttree();
      break;
	case "outside":
      outside();
      break;
	case "inside":
      inside();
      break;
    case "end_level":
      end_level();
      break;
  }
}
