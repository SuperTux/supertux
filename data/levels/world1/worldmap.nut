
import("/levels/preload_worldselect.nut");

fade_time <- 0.35;

// If we are using world select to change WMs, then the states for other worlds won't get changed.
// Here, we account for this.

if("underground_f" in state)
  state.underground_f = false  // forest underground state
if("corrupted" in state)
  state.corrupted = false

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

if (!("fork_secret_v2" in state))
{
  // there is no compat... we ignore the old states for now. if they beat the
  // level or didn't complete the fork in the road then things would just get
  // weird regardless, like paths being opened despite never finding the
  // secrets... since the worldmap is pretty much redone anyway, new players
  // will start from the beginning anyway hopefully.
  state.fork_secret_v2 <- 0;
  state.fridge_secret_v2 <- 0;
  state.slide_secret_v2 <- 0;
}

// Always allow these dirs regardless
state.fork_secret_v2 <- (state.fork_secret_v2 | SPECIALTILE_DIR_WEST);
state.fridge_secret_v2 <- (state.fridge_secret_v2 | SPECIALTILE_DIR_EAST);
state.slide_secret_v2 <- (state.slide_secret_v2 | SPECIALTILE_DIR_EAST);
worldmap.fork_secret_obj.set_direction_mask(state.fork_secret_v2);
worldmap.fridge_secret_obj.set_direction_mask(state.fridge_secret_v2);
worldmap.slide_secret_obj.set_direction_mask(state.slide_secret_v2);

if(!state.fork_secret_v2) fork_secret.fade(0.2, 0);
else fork_secret.fade(1, 0);
if(!state.fridge_secret_v2) fridge_secret.fade(0.2, 0);
else fridge_secret.fade(1, 0);
if(!state.slide_secret_v2) slide_secret.fade(0.2, 0);
else slide_secret.fade(1, 0);

if (("key_knowledge" in state)) {
	  if (state.key_knowledge == true) {
        display_keys(true); }}
