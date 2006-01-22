/* Default functions for the whole levelset */

function intro()
{
  Tux.deactivate();
  Tux.set_visible(false);
  DisplayEffect.sixteen_to_nine(0);
  DisplayEffect.fade_in(2);
  Camera.scroll_to(0, 945, 15);
  // Sound.play_music("music/tux_intro.ogg");
  wait(2);

  wait(20);
  
  Level.finish();
}

