function initialize()
{
  wait(0);
  start_cutscene();
  
  TUX.set_action("big-run-right");
  YETI.set_visible(false);
  YETI.set_solid(false);
  YETI.enable_gravity(false);
  
  Tux.deactivate();
  Tux.set_visible(false);
  
  
  Camera.set_pos(0, 50);
  
  Effect.sixteen_to_nine(0);
  Effect.fade_in(2);
  
  Camera.scroll_to(600, Camera.get_y(), 3.5);
  Camera.set_scale_anchor(1.5, 4);  
  
  TUX.set_velocity(300, 0);
  
  wait(1.5);
  
  ///Player triggers state: "textbox"
  Tux.set_pos(1376, 160);
  
  wait(1);
  
  Camera.scroll_to(680, Camera.get_y(), 21.1);
  
  wait(1);
  
  TUX.set_velocity(150, 0);
  TUX.set_action("big-walk-right");
  wait(0.6);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  wait(6);
  TUX.set_velocity(100, 0);
  TUX.set_action("big-walk-right");
  wait(1);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-right");
  wait(3.5);
  TUX.set_action("big-stand-left");
  wait(1.5);
  TUX.set_action("big-stand-right");
  wait(2);
  Camera.scroll_to(320, Camera.get_y(), 0.5);
  TUX.set_action("big-stand-left");
  wait(0.5);
  Camera.scroll_to(260, Camera.get_y(), 2);
  wait(2);
  Camera.scroll_to(800, Camera.get_y(), 0.5);
  TUX.set_action("big-stand-right");
  wait(0.5);
  Camera.scroll_to(880, Camera.get_y(), 1.5);
  wait(1.5);
  Camera.scroll_to(500, Camera.get_y(), 0.5);
  wait(0.5);
  TUX.set_action("big-stand-left");
  Camera.scroll_to(450, Camera.get_y(), 1.2);
  wait(1.2);
  Camera.scroll_to(300, Camera.get_y(), 28.5);
  Camera.scale(1.8, 28.5);
  wait(1);
  TUX.set_velocity(-100, 0);
  TUX.set_action("big-walk-left");
  wait(2.7);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-left");
  wait(24.3);
  stop_music(0.5);
  
  play_sound("sounds/yeti_gna.wav");
  wait(0.5);
  TUX.set_action("big-stand-left");
  play_music("music/antarctic/yetiboss.music");
  Camera.scroll_to(80, Camera.get_y(), 0.5);
  Camera.scale(1.5, 0.5);
  
  ///Player triggers state: "yeti_ambush"
  Tux.set_pos(1312, 160);
  
  
  wait(2.7);
  Camera.scroll_to(500, Camera.get_y(), 0.5);
  
  play_sound("sounds/jump.wav");
  TUX.set_velocity(100, -300);
  TUX.set_action("big-fall-left");
  wait(0.6);
  
  play_sound("sounds/skid.wav");
  TUX.set_action("big-skid-right");
  TUX.set_velocity(-100, 0);
  wait(0.5);
  TUX.set_action("big-run-right");
  TUX.set_velocity(320, 0);
  
  wait(1);
  
  Camera.scroll_to(70, Camera.get_y(), 0.5);
  wait(0.5);
  Camera.scroll_to(60, Camera.get_y(), 4.5);
  wait(4.5);
  Camera.scroll_to(600, Camera.get_y(), 3);
  wait(1);
  
  Effect.fade_out(1);
  wait(1.5);
  
  ///Transition to next scene.
  
  sector.settings.set_ambient_light(1, 1, 1);
  clouds.set_enabled(true);
  snow.set_enabled(true);
  castle_wall.set_color(1, 1, 1, 0);
  castle_pillars.set_color(1, 1, 1, 0);
  
  Camera.set_pos(2050, 0);
  TUX.set_pos(1920, 352);
  TUX.set_velocity(0, 0);
  YETI.set_pos(1760, 320);
  YETI.set_velocity(0, 0);
  
  wait(1);
  
  Effect.fade_in(1);
  
  Camera.scroll_to(2100, Camera.get_y(), 8);
  Camera.scale_anchor(1.2, 4, 4);
  
  wait(1);
  TUX.set_velocity(320, 0);
  
  wait(1.5);
  play_sound("sounds/skid.wav");
  TUX.set_velocity(100, 0);
  TUX.set_action("big-skid-left");
  wait(0.2);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-left");
  wait(0.2);
  play_sound("sounds/yeti_gna.wav");
  play_sound("sounds/jump.wav");
  TUX.set_velocity(100, -300);
  TUX.set_action("big-fall-left");
  wait(0.6);
  TUX.set_velocity(0, 0);
  TUX.set_action("big-stand-left");
  wait(0.2);
  TUX.set_action("big-stand-right");
  wait(0.5);
  TUX.set_velocity(250, -450);
  play_sound("sounds/jump.wav");
  TUX.set_action("big-jump-right");
  wait(0.5);
  TUX.set_action("big-fall-right");
  
  YETI.set_velocity(350, 0);
  wait(2);
  play_sound("sounds/yeti_roar.wav");
  YETI.set_action("jump-right");
  YETI.set_velocity(350, -500);
  wait(2);
  
  Effect.fade_out(1);
  wait(1.5);
  
  ///Transition to next scene.
  
  Camera.set_pos(2450, 350);
  TUX.enable_gravity(false);
  YETI.enable_gravity(false);
  TUX.set_velocity(0, 0);
  YETI.set_velocity(0, 0);
  TUX.set_pos(3000, 200);
  YETI.set_pos(2900, 100);
  
  
  wait(0.5);
  
  Effect.fade_in(1);
  
  Camera.scroll_to(2550, Camera.get_y(), 12);
  
  wait(0.5);
  
  TUX.enable_gravity(true);
  
  TUX.set_velocity(200, 0);
  wait(1.1);
  TUX.set_action("big-duck-right");
  TUX.move(0, 32);
  TUX.set_velocity(0, 0);
  wait(0.1);
  play_sound("sounds/jump.wav");
  TUX.set_action("big-fall-right");
  TUX.move(0, -32);
  TUX.set_velocity(320, -300);
  wait(0.6);
  TUX.set_action("big-run-right");
  TUX.set_velocity(320, 0);
  
  wait(1.2);
  
  YETI.enable_gravity(true);
  YETI.set_action("leap-right");
  
  YETI.set_velocity(200, 0);
  wait(0.6);
  Camera.scale_anchor(1.5, 0.5, 4);
  wait(0.5);
  YETI.set_velocity(0, 0);
  YETI.set_action("stomp-right");
  Camera.start_earthquake(3, 0.1);
  wait(0.6);
  play_sound("sounds/yeti_roar.wav");
  YETI.set_action("walk-right");
  YETI.set_velocity(320, 0);
  wait(0.5);
  Camera.stop_earthquake();
  wait(0.5);
  
  wait(2);
  
  trigger_state("end_level");
  end_cutscene();
}

function yeti_ambush()
{
  YETI.set_visible(true);
  YETI.set_solid(true);
  YETI.enable_gravity(true);
  YETI.set_velocity(500, -500);
  YETI.set_action("jump-right")
  wait(1)
  YETI.set_velocity(0, 0);
  YETI.set_action("stomp-right")
  Camera.start_earthquake(3, 0.1);
  wait(0.5);
  YETI.set_action("stand-right")
  play_sound("sounds/yeti_roar.wav");
  wait(0.5);
  Camera.stop_earthquake();
  
  wait(3.5);
  
  play_sound("sounds/yeti_roar.wav");
  YETI.set_action("rage-right")
  wait(1.7);
  YETI.set_action("stand-right")
  wait(0.5);
  play_sound("sounds/yeti_roar.wav");
  YETI.set_action("walk-right");
  YETI.set_velocity(350, 0);
  wait(2.8);
  
}

function textbox()
{
  Text.set_text(_("At last, Tux managed to rush towards the final room of the icy castle."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(2);
  
  Text.set_text(_("To his suprise, there was no one here. Neither Penny, nor this... strange Nolok guy."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(0.5);
  
  Text.set_text(_("Surely they had to be here somewhere, right? Perhaps a hidden room?"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(3.5);
  
  Text.set_text(_("After a bit of back and forth with himself, Tux noticed a small note attached to one of the pillars."));
  Text.fade_in(0.5);
  wait(5);
  Text.fade_out(0.5);
  
  wait(4.5);
  
  Text.set_text(_("It was yet another note, by that Nolok."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("It read that he had already left Icy Island towards the far north. If he wished to follow him and save his beloved friend, he would have to come to the Crystal Mine."));
  Text.fade_in(0.5);
  wait(8);
  Text.fade_out(0.5);
  
  wait(1);

  Text.set_text(_("Marked below was a rough map of the Glacier Isle, showing a boat and some kind of small pier on the other end."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(3);

  Text.set_text(_("But before Tux could even make sense of any of it, a load roar filled the room!"));
  Text.fade_in(0.5);
  wait(3);
  Text.fade_out(0.5);
  
  wait(3);

  Text.set_text(_("The mighty Yeti had made his appearence, busting his chest in an intimidating fashion!"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(7.5);

  Text.set_text(_("Tux made his way towards a balcony on the other end of the room. With no other option, he jumped!"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(10);

  Text.set_text(_("Unfortunately, the Yeti would not let him go so easily..."));
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
states <- { init=0, start=1, textbox=2, yeti_ambush=3, end_level=4};
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
	case "yeti_ambush":
      yeti_ambush();
      break;
	case "end_level":
      end_level();
      break;
  }
}
