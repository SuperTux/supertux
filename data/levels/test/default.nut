/* Default functions for the whole levelset */

function intro()
{  
  //initialize
  SUPERTUX.set_action("stand-right");
  RADIO.set_action("quiet");  
  PENNY.set_action("stand-left");
  NOLOK.set_visible(false);
  local logo = FloatingImage("images/objects/logo/logo.sprite");
  Tux.deactivate();
  Tux.set_visible(false);
  DisplayEffect.sixteen_to_nine(0);
  
  //begin scrolling sequence
  DisplayEffect.fade_in(2);
  Camera.scroll_to(0, 945, 15);
  Sound.play("music/intro.ogg");
  wait(3);
  Text.set_text("Somewhere at the shores\nof Antarctica...");
  Text.fade_in(2);
  wait(3);
  Text.fade_out(2);
  wait(10);
  SUPERTUX.set_velocity(50,0);
  Camera.scroll_to(3100, 945, 18);
  wait(10);
  logo.set_anchor_point(ANCHOR_TOP);
  logo.set_pos(0, 50);
  logo.set_visible(true);
  wait(5);
  logo.set_visible(false);
  wait(6);
  
  //begin conversation and Tux rap
  SUPERTUX.set_velocity(0,0);
  Sound.play("speech/tux_hello.ogg");
  wait(3);
  Sound.play("speech/penny_runt_01.ogg");
  wait(1);
  Sound.play("speech/tux_murp_01.ogg");
  wait(1);
  RADIO.set_action("loud");
  Sound.play("speech/tux_rap.ogg");
  wait(15);
  shake_bush();
  wait(2);
  shake_bush();
  wait(2);
  shake_bush();
  wait(1.3);
  
  //enter Nolok
  NOLOK.set_velocity(-220, 600);
  NOLOK.set_visible(true);
  DisplayEffect.fade_out(1.3);
  wait(3);

  //darkness
  NOLOK.set_visible(false);
  PENNY.set_visible(false);
  RADIO.set_action("quiet");
  SUPERTUX.set_pos(3550, SUPERTUX.get_pos_y());

  //wake up, Tux...
  DisplayEffect.fade_in(4);
  wait(4);
  Sound.play("speech/tux_upset.ogg");
  wait(3);
  tux_upset();
  wait(1);
  tux_upset();
  wait(4);
  SUPERTUX.set_action("stand-right");
  SUPERTUX.set_velocity(300,0);
  wait(2);
  
  //end intro sequence
  DisplayEffect.fade_out(2);
  wait(3);
  Level.finish();
}

function shake_bush()
{
  //Sound.play("sounds/rustle.wav");
  local bushx = BUSH.get_pos_x();
  local bushy = BUSH.get_pos_y();
  for(local i = 0; i < 20; ++i) {
    BUSH.set_pos(bushx + rand() % 6 - 3, bushy);
    wait(0.05);
  }
}

function tux_upset()
{
  SUPERTUX.set_action("stand-right");
  SUPERTUX.set_velocity(200,0);
  wait(0.3);
  SUPERTUX.set_velocity(0,0);
  wait(0.4);
  SUPERTUX.set_action("stand-left");
  SUPERTUX.set_velocity(-200,0);
  wait(0.3);
}

