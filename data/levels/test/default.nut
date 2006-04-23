/* Default functions for the whole levelset */
print("default.nut loaded\n");

function intro_scene2()
{
  //initialize
  Camera.scroll_to(0, 945, 0);
  SUPERTUX.set_action("stand-right");
  SUPERTUX.set_visible(false);
  NOLOK.set_visible(false);
  Tux.deactivate();
  Tux.set_visible(false);
  Effect.sixteen_to_nine(0);
  play_sound("music/nolok.ogg");
  Effect.fade_in(5);
  wait(5);
  Camera.scroll_to(3100, 945, 8);
  NOLOK.set_visible(true);  
  NOLOK.set_velocity(500,0);
  wait(8);
  SUPERTUX.set_visible(true);
  SUPERTUX.set_velocity(300,0);
  wait(24);
  Level.finish(true);
}

