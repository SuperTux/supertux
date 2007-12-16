if(! ("world" in state)) {
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

