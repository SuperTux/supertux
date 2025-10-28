function play_cutscene() {
	start_cutscene();
	Level.pause_target_timer();
	Effect.sixteen_to_nine(1);
	Tux.deactivate()
	if(boss.get_action() == "busted-left") {
		if(Tux.get_x() + Tux.get_width() > boss.get_x() - boss.get_width()) {
			Tux.set_dir(false);
			Tux.walk(-100);
			while(Tux.get_x() + Tux.get_width() > boss.get_x() - boss.get_width()) wait(0);
			Tux.walk(0);
		}
		Tux.set_dir(true);
		Tux.walk(100);
		while(Tux.get_x() + Tux.get_width() < boss.get_x() - boss.get_width()) wait(0);
		Tux.walk(0);
	} else {
		if(Tux.get_x() < boss.get_x() + boss.get_width() * 2) {
			Tux.set_dir(true);
			Tux.walk(100);
			while(Tux.get_x() < boss.get_x() + boss.get_width() * 2) wait(0);
			Tux.walk(0);
		}
		Tux.set_dir(false);
		Tux.walk(-100);
		while(Tux.get_x() > boss.get_x() + boss.get_width() * 2) wait(0);
		Tux.walk(0);
	}

	Text.set_anchor_point(ANCHOR_BOTTOM);
 	Text.set_anchor_offset(0, -160);
	wait(1);

	Text.set_text(_("As Tux stared at the yeti he knocked out, it looked back at him, visibly frightened."));
	Text.fade_in(1);
	wait(5);
	Text.set_text(_("After what seemed like an eternity, the yeti spoke. It told Tux that he won't find Nolok on Icy Island, and that to continue his journey..."));
	wait(6);
	Text.set_text(_("...he would have to find the Glacier Isles, a set of small islands just off the shore of Icy Island, and find a crystal mine,"));
	wait(6);
	Text.set_text(_("which he would have to traverse. It told Tux that there is a dock behind the mine,"));
	wait(4);
	Text.set_text(_("where he could get a boat and set sail for Rooted Forest, the island on which Nolok's second fortress lay."));
	wait(5);
	Text.fade_out(1);
	Effect.fade_out(2);
	wait(2);
	end_cutscene();
	Level.finish(true);
}
