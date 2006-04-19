if(! ("intro_displayed" in state)) {
	load_level("levels/world1/intro.stl");
	wait_for_screenswitch();
	wait_for_screenswitch();
	state.intro_displayed <- true;
	save_state();
}
load_worldmap("levels/world1/worldmap.stwm");
fadeout_screen(0.5);
wait_for_screenswitch();
save_state();
wait_for_screenswitch();
save_state();
print("Goodbye from world1\n");
