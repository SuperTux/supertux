function initialize()
{
  Tux.add_bonus("grow");
  RADIO.set_action("quiet");
  PENNY.set_action("stand-left");
  NOLOK.set_visible(false);
  LOGO.set_visible(false);
  LOGO.enable_gravity(false);
  SIGN.set_visible(false);
}

function logo_in()
{
  local x = LOGO.get_pos_x();
  local y = LOGO.get_pos_y();
  local time = 1.5;
  LOGO.set_pos(x,-120);
  LOGO.set_visible(true);
  LOGO.set_velocity(0,(120+y)/time);
  LOGO.enable_gravity(false);
  wait(time);
  LOGO.set_velocity(0,0);
  wait(3);
  LOGO.set_visible(false);
}

function intro_text()
{
  Text.set_text(_("Somewhere at the shores\nof Antarctica..."));
  Text.fade_in(1);
  wait(1);
  Text.fade_out(1);
}

function rap_scene()
{
  Tux.deactivate();
  //begin conversation and Tux rap
  play_sound("speech/tux_hello.ogg"); // 3.1 seconds
  Text.set_centered(false);
  Text.set_text(_("Tux: Hello Penny"));
  Text.fade_in(1);
  wait(2.5);
  Text.fade_out(1);
  wait(1);
  play_sound("speech/penny_runt_01.ogg"); // 1.2 seconds
  Text.set_text(_("Penny: Hey Tux"));
  Text.fade_in(0.5);
  wait(1);
  Text.fade_out(0.5);
  wait(0.5);
  play_sound("speech/tux_murp_01.ogg"); // 1.5 seconds
  Text.set_text(_("Tux: Check out my dance moves."));
  Text.fade_in(0.5);
  wait(1);
  Text.fade_out(0.5);
  wait(0.5);
  RADIO.set_action("loud");
  play_sound("speech/tux_rap.ogg"); // 24.6 seconds
  local t = ::newthread(shake_bush_thread);
  t.call(this); // Nolok's waiting...
  // meanwhile... Tux dances!
  // TODO: add some more dance moves besides jumps, ducks, & flips
  // and fill in the parts where he just stands still
  wait(6); // music intro - 6 seconds
  Tux.do_duck();
  Tux.set_dir(false);
  wait(0.40625); // all times rounded to nearest 1/64 of a second
  Tux.do_standup();
  Tux.kick(); // 0.3 seconds
  wait(0.40625);
  Tux.set_dir(true);
  wait(0.203125);
  Tux.set_dir(false);
  wait(0.203125);
  Tux.set_dir(true);
  wait(0.09375);
  Tux.do_duck(); // t=7.3
  wait(0.5);
  Tux.set_dir(false);
  wait(0.203125);
  Tux.set_dir(true);
  wait(0.203125);
  Tux.set_dir(false);
  wait(0.203125);
  Tux.set_dir(true);
  wait(0.203125);
  Tux.set_dir(false);
  wait(0.203125);
  Tux.set_dir(true);
  Tux.do_standup(); // t=8.8
  wait(0.203125);
  Tux.set_dir(false);
  wait(0.203125);
  Tux.kick();
  wait(0.40625);
  Tux.set_dir(true);
  Tux.do_jump(-300);
  wait(0.703125);
  Tux.do_duck(); // t=10.7
  wait(0.296875);
  Tux.set_dir(false);
  wait(0.59375);
  Tux.set_dir(true);
  wait(0.703125);
  Tux.do_standup(); // t=12.3
  Tux.kick();
  wait(0.5);
  Tux.do_duck();
  wait(0.5);
  Tux.do_backflip(); // t=13.3
  wait(79.0/64);
  Tux.walk(200);
  wait(38.0/64);
  Tux.walk(0);
  wait(184.0/64);
  Tux.set_dir(false); //t=18.0
  wait(0.3)
  Tux.kick();
  wait(0.40625);
  Tux.do_duck();
  Tux.set_dir(true);
  wait(0.296875);
  Tux.set_dir(false); // t=19.0
  wait(0.5);
  Tux.do_standup();
  Tux.do_jump(-800);
  wait(1.5);
  Tux.kick(); // t=21
  wait(0.5);
  Tux.do_duck();
  wait(0.5);
  Tux.do_jump(-400);
  wait(1.0);
  Tux.do_standup();
  wait(0.796875);
  Tux.set_dir(true); //t=23.8 - Tux sees Nolok!
  Tux.do_jump(-520);
  wait(151.0/64);
  // we have to activate Tux to hurt him
  Tux.activate();
  Tux.kill(false);
  Tux.deactivate();
  wait(1.5);
  // song is done
  // darkness
  NOLOK.set_visible(false);
  NOLOK.set_solid(false);
  PENNY.set_visible(false);
  PENNY.set_solid(false);
  RADIO.set_visible(false);
  RADIO.set_solid(false);
  SIGN.set_visible(true);
  //wake up, Tux...
  Effect.fade_in(3);
  play_sound("speech/tux_upset.ogg"); // 11 seconds
  Text.set_text(_("Tux: Oww... my head..."));
  Text.fade_in(1);
  wait(1.9);
  Tux.activate();
  Text.fade_out(0.3);
  wait(0.5);
  Text.set_text(_("Tux: Wait!")); // t=2.4
  Text.fade_in(0.5);
  wait(1);
  Text.fade_out(0.5);
  wait(0.5);
  Text.set_text(_("Tux: Penny!")); // t=3.9
  Text.fade_in(0.5);
  wait(1);
  Text.fade_out(0.5);
  wait(0.5);
  Text.set_text(_("Tux: Where are you, Penny?!")); // t=4.4
  Text.fade_in(0.5);
  wait(2);
  Text.fade_out(0.5);
  wait(0.5);
  Text.set_text(_("Tux: Oh no...")); // t=6.9
  Text.fade_in(0.5);
  wait(1);
  Text.fade_out(0.5);
  wait(0.5);
  Text.set_text(_("Tux: Don't worry, Penny, I'll rescue you!")); // t=8.4
  Text.fade_in(0.5);
  wait(2);
  Text.fade_out(0.5);
  wait(0.5);
}

function shake_bush()
{
  //play_sound("sounds/rustle.wav");
  local bushx = BUSH.get_pos_x();
  local bushy = BUSH.get_pos_y();
  for(local i = 0; i < 21; ++i) {
    BUSH.set_pos(bushx + ( rand() % 6 ) - 3, bushy);
    wait(0.046875);
  }
  wait(0.015625);
}

function shake_bush_thread(table)
{
  table.wait(8);
  table.shake_bush(); // each bush shake lasts 1 second
  table.wait(5);
  table.shake_bush();
  table.wait(3);
  table.shake_bush();
  table.wait(2);
  table.shake_bush();
  table.shake_bush(); // total 23 seconds
  //enter Nolok
  table.NOLOK.set_velocity(-220, 600);
  table.NOLOK.set_visible(true);
  table.Effect.fade_out(1.3);
}

function end_level()
{
  //end intro sequence
  Effect.fade_out(2);
  wait(2.1);
  Level.finish(true);
}

state_idx <- 0;
states <- { init=0, start=1, logo_in=2, intro_text=3, rap_scene=4, end_level=5};
function trigger_state(state) {
  local idx = states[state];
  if(!idx || idx <= state_idx)
    return;
  state_idx = idx;
  switch(state) {
    case "start":
      initialize();
      break;
    case "logo_in":
      logo_in();
      break;
    case "intro_text":
      intro_text();
      break;
    case "rap_scene":
      rap_scene();
      break;
    case "end_level":
      end_level();
      break;
  }
}