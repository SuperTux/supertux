if(! ("underground" in state)){
	state.underground <- false;
  print("[DEBUG] Underground state initialized\n");
}

function go_underground(under){
  fade_time <- 0.35;
  Underground_cover.fade(under ? 0 : 1, fade_time);
  Land_foreground_1.fade(under ? 0 : 1, fade_time);
  Land_foreground_2.fade(under ? 0 : 1, fade_time);
  Land_foreground_3.fade(under ? 0 : 1, fade_time);
  Underground_mask.fade(under ? 1 : 0, fade_time);
  Underground_darkness.fade(under ? 1 : 0, fade_time);
  state.underground <- under;
}

go_underground(state.underground);
