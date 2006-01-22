/* Default functions for the whole levelset */

function intro()
{
  SUPERTUX.set_action("stand-right");  
  RADIO.set_action("quiet");  
  PENNY.set_action("stand-left");
  Tux.deactivate();
  Tux.set_visible(false);
  DisplayEffect.sixteen_to_nine(0);
  DisplayEffect.fade_in(2);
  Camera.scroll_to(0, 945, 15);
  // Sound.play_music("music/intro.ogg");
  wait(16);
  SUPERTUX.set_velocity(50,0);
  Camera.scroll_to(3100, 945, 18);
  wait(10);
  Text.set_text("SuperTux\n(Replace this with the title logo)");
  Text.fade_in(2);
  wait(5);
  Text.fade_out(2);
  wait(6);
  SUPERTUX.set_velocity(0,0);
  RADIO.set_action("loud");
  wait(10);
  Level.finish();
}

