function intro()
{  
  //initialize
  SUPERTUX.set_action("walk-right");
  logo <- FloatingImage("images/objects/logo/logo.sprite");
  Tux.deactivate();
  Tux.set_visible(false);
  Effect.sixteen_to_nine(0);
  
  //begin scrolling sequence
  Effect.fade_in(2);
  Camera.scroll_to(0, 945, 15);
  wait(3);
  Text.set_text("Somewhere at the shores\nof Antarctica...");
  Text.fade_in(2);
  wait(3);
  Text.fade_out(2);
  wait(10);
  SUPERTUX.set_velocity(75,0);
  Camera.scroll_to(3300, 945, 18);
  wait(10);
  logo.set_anchor_point(ANCHOR_TOP);
  logo.set_pos(0, 90);
  logo.set_visible(true);
  wait(5);
  logo.set_visible(false);
  wait(5);

  Level.finish(true);
}

