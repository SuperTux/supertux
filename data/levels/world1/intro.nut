function intro()
{  
  logo <- FloatingImage("images/objects/logo/logo.sprite");
  Tux.deactivate();
  Tux.set_visible(false);
  Effect.sixteen_to_nine(0);
  Effect.fade_in(2);
  Camera.scroll_to(0, 945, 15);
  wait(3);
  Text.set_text("Somewhere at the shores\nof Antarctica...");
  Text.fade_in(2);
  wait(3);
  Text.fade_out(2);
  wait(2);
  logo.set_anchor_point(ANCHOR_TOP);
  logo.set_pos(0, 90);
  logo.set_visible(true);
  wait(6);
  Effect.fade_out(2);
  wait(2);
  Level.finish(true);
}


