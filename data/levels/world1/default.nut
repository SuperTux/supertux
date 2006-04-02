function intro()
{
  SUPERTUX.set_action("stand-right");
  Tux.deactivate();
  Tux.set_visible(false);
  DisplayEffect.sixteen_to_nine(0);
  DisplayEffect.fade_in(2);
  wait(2);

  Text.set_text(translate("Tux and Penny were out having a\n nice picnic on the\nice fields of Antarctica."));
  Text.fade_in(1);
// TODO play some tux sounds...
  wait(1);

  Sound.play("speech/tux_rap.ogg");
  wait(5);
  Text.fade_out(1);
  wait(15);

  Text.set_text(translate("Then suddenly..."));
  Text.fade_in(1);

  // let's shake the bush...
  // Sound.play("sounds/rustle.wav");
  local bushx = BUSH.get_pos_x();
  local bushy = BUSH.get_pos_y();
  for(local i = 0; i < 20; ++i) {
    BUSH.set_pos(bushx + rand() % 6 - 3, bushy);
    wait(0.1);
  }
  Text.fade_out(1);

// NOLOK jumps out of the bush
  wait(0.5);
  print("jump");
  NOLOK.set_velocity(70, 600);

  wait(1)
  NOLOK.set_velocity(-120, 700);
  wait(1.2);
  NOLOK.set_velocity(0, 0);
  wait(1);

// nolok casts his spell...
  NOLOK.set_action("throw");
  // TODO we really need fade to white here and some thunder sound...
  DisplayEffect.fade_out(0.3);
  wait(0.3);
  DisplayEffect.fade_in(0);
  wait(0.3);
  DisplayEffect.fade_out(0.5);
  wait(0.5);
  DisplayEffect.fade_in(0);
  wait(0.4);
  DisplayEffect.fade_out(0.2);
  wait(2.5);
  NOLOK.set_visible(false);
  PENNY.set_visible(false);
  DisplayEffect.fade_in(1);
  wait(1);

  Text.set_text("Oh No!\nPenny has been captured");
  Text.fade_in(1);
  wait(3);
  Text.fade_out(1);
  
  Text.set_text("Tux has to rescue her");
  Text.fade_in(1);
  wait(5);

// fade out
  DisplayEffect.fade_out(2);
  wait(2);
  Level.finish();
}
