function level_intro()
{
  start_cutscene();
  
  Tux.use_scripting_controller(true);
  Camera.set_mode("manual");
  Effect.sixteen_to_nine(1);
  
  end_cutscene();
}

function level_outro()
{
  start_cutscene();
  
  Effect.sixteen_to_nine(0);
  
  trigger_state("end_level");
  
  end_cutscene();
  
}

function textbox_intro()
{
  Text.set_text(_("Despite the Ghost Tree's defeat and a sliver of life returning to the once lush forest, the land was still in shambles."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("The ground was riddled with cracks and crevices. And now one of them was blocking Tux's path..."));
  Text.fade_in(0.5);
  wait(3);
  Text.fade_out(0.5);
  
  wait(9);
  
  Text.set_text(_("Fortunately for him, a Granito, alive and well, revealed themself, excited to see him on the other side of the crevice."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("The Granito spoke of 3 groups of his fellow Granito who had gotten lost when trying to escape the roots to the east coast."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("They were swallowed by the earth as the ground collapsed in on itself. If Tux could help find them they could help him reach the other side."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("Tux agreed to help! Soon after he noticed a dying roots revealing a path underneath, he began his search for the lost Granito."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
}

function textbox_outro()
{ 
  Text.set_text(_("With the help of the saved Granito, Tux managed to reach the other side."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);

  Text.set_text(_("He was glad there were more Granito that had survived the Ghost Tree's overgrowth."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("One of the Granito asked him what a penguin like him was doing so far from the Antarctic."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("Tux replied, that he was looking for his friend Penny and that she was captured by an evil creature named \"Nolok\". A name the Granito remembered unpleasantly"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("The Granito mentioned they knew his homeland. But whether he still called it his home nor if he held Penny there, they do not know for certain."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("They offered to show Tux the way. It was the least they could do to show their gratitute"));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("Tux gladly accepted and together with a handful of Granito they soon made their way to the tropics, in the north-east."));
  Text.fade_in(0.5);
  wait(4);
  Text.fade_out(0.5);
  
  wait(1);
  
  Text.set_text(_("The remaining Granito would stay behind, slowly rebuilding their home, now that the corruption was finally vanquished..."));
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
states <- { init=0, intro=1, outro=2, textbox_intro=3, textbox_outro=4, end_level=5, free_granito1=6, free_granito2=7, free_granito3=8 };
function trigger_state(state) {
  local idx = states[state];
  if(!idx || idx <= state_idx)
    return;
  state_idx = idx;
  switch(state) {
    case "intro":
      level_intro();
      break;
	case "outro":
      level_outro();
      break;
	case "textbox_intro":
      textbox_intro();
      break;
	case "textbox_outro":
      textbox_outro();
      break;
	case "end_level":
      end_level();
      break;
	case "free_granito1":
      free_granito1();
      break;
	case "free_granito2":
      free_granito2();
      break;
	case "free_granito3":
      free_granito3();
      break;
  }
}

// ============================================================================
//   Granito Freed
// ============================================================================

sector.granito_freed <- 0


function free_granito1()
{
  granito_freed += 1
  
  start_cutscene();
  
  Effect.sixteen_to_nine(1);
  Tux.use_scripting_controller(true);
  
  Effect.fade_out(1);
  wait(1.2);
  Camera.set_scale(2);
  Camera.set_pos(11650, 2950);
  wait(0.3);
  Effect.fade_in(1);
  
  root_prison1.start_moving();
  
  Text.set_text(_("Granito freed!"));
  Text.fade_in(0.5);
  wait(1.5);
  Text.fade_out(0.5);
  wait(3)
  
  granito_big1.set_action("left");
  granito_big1.set_velocity(-80, 0);
  granito_small1.set_action("left");
  granito_small1.set_velocity(-100, 0);
  granito_small2.set_action("left");
  granito_small2.set_velocity(-100, 0);
  
  wait(3);
  
  Effect.fade_out(1);
  wait(1.2);
  Camera.set_scale(1.2);
  
  ///Teleport Granito to their destination.
  granito_big1.set_action("stand-left");
  granito_big1.set_velocity(0, 0);
  granito_small1.set_action("stand-left");
  granito_small1.set_velocity(0, 0);
  granito_small2.set_action("stand-left");
  granito_small2.set_velocity(0, 0);
  
  granito_big1.set_pos(10208, 1024);
  granito_small1.set_pos(10144, 1024);
  granito_small2.set_pos(10304, 1024);
  
  dead_root4.fade(0, 0);
  
  Camera.set_mode("normal");
  wait(0.3);
  Effect.fade_in(1);
  
  Tux.use_scripting_controller(false);
  Effect.four_to_three(1);
  
  end_cutscene();
}

function free_granito2()
{
  granito_freed += 1
  
  start_cutscene();
  
  Effect.sixteen_to_nine(1);
  Tux.use_scripting_controller(true);
  
  Effect.fade_out(1);
  wait(1.2);
  Camera.set_scale(2);
  Camera.set_pos(15600, 1700);
  wait(0.3);
  Effect.fade_in(1);
  
  root_prison2.start_moving();
  
  Text.set_text(_("Granito freed!"));
  Text.fade_in(0.5);
  wait(1.5);
  Text.fade_out(0.5);
  wait(3)
  
  granito_big2.set_action("left");
  granito_big2.set_velocity(-80, 0);
  granito_small3.set_action("left");
  granito_small3.set_velocity(-100, 0);
  granito_small4.set_action("left");
  granito_small4.set_velocity(-100, 0);
  
  wait(3);
  
  Effect.fade_out(1);
  wait(1.2);
  Camera.set_scale(1.2);
    
  ///Teleport Granito to their destination.
  granito_big2.set_action("stand-left");
  granito_big2.set_velocity(0, 0);
  granito_small3.set_action("stand-left");
  granito_small3.set_velocity(0, 0);
  granito_small4.set_action("stand-left");
  granito_small4.set_velocity(0, 0);
  
  granito_big2.set_pos(10560, 960);
  granito_small3.set_pos(10576, 928);
  granito_small4.set_pos(10576, 896);
  
  dead_root6.start_moving();

  Camera.set_mode("normal");
  wait(0.3);
  Effect.fade_in(1);
  
  Tux.use_scripting_controller(false);
  Effect.four_to_three(1);
  
  end_cutscene();
}

function free_granito3()
{
  granito_freed += 1
  
  start_cutscene();
  
  Effect.sixteen_to_nine(1);
  Tux.use_scripting_controller(true);
  
  Effect.fade_out(1);
  wait(1.2);
  Camera.set_scale(2);
  Camera.set_pos(15300, 3200);
  wait(0.3);
  Effect.fade_in(1);
  
  root_prison3.start_moving();
  
  Text.set_text(_("Granito freed!"));
  Text.fade_in(0.5);
  wait(1.5);
  Text.fade_out(0.5);
  wait(3)
  
  granito_big3.set_action("left");
  granito_big3.set_velocity(-80, 0);
  granito_small5.set_action("left");
  granito_small5.set_velocity(-100, 0);
  granito_small6.set_action("left");
  granito_small6.set_velocity(-100, 0);
  granito_small7.set_action("left");
  granito_small7.set_velocity(-100, 0);
  
  wait(3);
  
  Effect.fade_out(1);
  wait(1.2);
  Camera.set_scale(1.2);
  
  ///Teleport Granito to their destination.
  granito_big3.set_action("stand-left");
  granito_big3.set_velocity(0, 0);
  granito_small5.set_action("stand-left");
  granito_small5.set_velocity(0, 0);
  granito_small6.set_action("stand-left");
  granito_small6.set_velocity(0, 0);
  granito_small7.set_action("stand-left");
  granito_small7.set_velocity(0, 0);
  
  granito_big3.set_pos(10784, 864);
  granito_small5.set_pos(10800, 832);
  granito_small6.set_pos(10800, 800);
  granito_small7.set_pos(10800, 768);
  
  dead_root5.fade(0, 0);
  
  Camera.set_mode("normal");
  wait(0.3);
  Effect.fade_in(1);
  
  Tux.use_scripting_controller(false);
  Effect.four_to_three(1);
  
  end_cutscene();
}