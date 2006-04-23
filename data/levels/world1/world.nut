if(! ("intro_displayed" in state)) {
	println("Display intro");
	load_level("levels/world1/intro.stl");
	println("Wait for screenswitch");
	wait_for_screenswitch();
	println("ok1");
	wait_for_screenswitch();
	println("ok2");
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

