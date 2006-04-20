if(! ("intro_displayed" in state)) {
	load_level("levels/world1/intro.stl");
	wait_for_screenswitch();
	wait_for_screenswitch();
	state.intro_displayed <- true;
	save_state();
}
if(! ("world" in state)) {
	println("No worldfound");
	state.world <- "levels/world1/worldmap.stwm";
	save_state();
}

// load worldmap and wait till it is displayed
load_worldmap(state.world);
fadeout_screen(0.5);
wait_for_screenswitch();
save_state();

worldthread <- get_current_thread();
// wait for worldchanges
while(true) {
	::suspend();
	exit_screen();
	load_worldmap(state.world);
	save_state();
}

