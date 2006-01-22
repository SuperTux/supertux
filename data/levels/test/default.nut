/* Default functions for the whole levelset */

function intro()
{
  Tux.deactivate();
  Tux.set_visible(false);
  DisplayEffect.sixteen_to_nine(0);
  DisplayEffect.fade_in(2);
  Camera.scroll_to(0, 945, 15);
  // Sound.play_music("music/intro.ogg");
  wait(16);
  Camera.scroll_to(3200, 945, 19);
  wait(10);
  Text.set_text("SuperTux\n(Replace this with the title logo)");
  Text.fade_in(2);
  wait(5);
  Text.fade_out(2);	 
  wait(4);

  wait(20);
  Level.finish();
}

