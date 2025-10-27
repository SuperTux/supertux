
import("/levels/preload_worldselect.nut");

fade_time <- 0.35;

// ============================================================================
//   AMBIENT LIGHT
// ============================================================================

if("ambient_r" in state && "ambient_g" in state && "ambient_b" in state){
  worldmap.settings.fade_to_ambient_light(state.ambient_r,
                                          state.ambient_g,
                                          state.ambient_b,
                                          0);
}

// ============================================================================
//   UNDERGROUND
// ============================================================================

if(! ("underground" in state)){
	state.underground <- false;
  print("[DEBUG] Underground state initialized\n");
}

function go_underground(under){
  under ? worldmap.settings.fade_to_ambient_light(0.4, 0.45, 0.6, fade_time) : worldmap.settings.fade_to_ambient_light(1, 1, 1, fade_time);
  Underground_cover.fade(under ? 0 : 1, fade_time);
  Land_foreground_1.fade(under ? 0 : 1, fade_time);
  Land_foreground_2.fade(under ? 0 : 1, fade_time);
  Land_foreground_3.fade(under ? 0 : 1, fade_time);
  Land_foreground_4.fade(under ? 0 : 1, fade_time);
  Underground_mask.fade(under ? 1 : 0, fade_time);
  state.underground <- under;
}

go_underground(state.underground);



// ============================================================================
//   ROAD FORKS
// ============================================================================

function reset_forks(fade_time = 0.35) {
	sector.fork_secret.fade(0.25, fade_time);
	sector.fridge_secret.fade(0.25, fade_time);
	sector.slide_secret.fade(0.25, fade_time);
}
