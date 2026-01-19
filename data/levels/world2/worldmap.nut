if ("world_select" in state) state.world_select["/levels/world2/worldmap.stwm"].unlocked = true;

fade_time <- 0.35;
fade_time_fast <- 0.1;

// If we are using world select to change WMs, then the states for other worlds won't get changed.
// Here, we account for this.

state.underground = false  // icy underground state

// ============================================================================
//   UNDERGROUND
// ============================================================================

if(! ("underground_f" in state)){
	state.underground_f <- false;
  print("[DEBUG] Underground_f state initialized\n");
}

function go_underground(under){
  under ? worldmap.settings.fade_to_ambient_light(0.4, 0.4, 0.45, fade_time) : worldmap.settings.fade_to_ambient_light(1, 1, 1, fade_time);
  trees_front1.fade(under ? 0 : 1, fade_time);
  trees_front2.fade(under ? 0 : 1, fade_time);
  trees_front3.fade(under ? 0 : 1, fade_time);
  trees_front4.fade(under ? 0 : 1, fade_time);
  underground_cover.fade(under ? 0 : 1, fade_time);
  underground_mask.fade(under ? 1 : 0, fade_time);
  state.underground_f <- under;
}

go_underground(state.underground_f);



// ============================================================================
//   CORRUPTED FOREST
// ============================================================================

if(! ("corrupted" in state)){
	state.corrupted <- false;
  print("[DEBUG] Corrupted Forest state initialized\n");
}

function corrupt_forest(corrupt){
  play_music(corrupt ? "music/forest/ghostforest_map.music" : "music/forest/new_forest_map.music");
  corrupt ? worldmap.settings.fade_to_ambient_light(0.48, 0.5, 0.6, fade_time) : worldmap.settings.fade_to_ambient_light(1, 1, 1, fade_time);
  
  ocean_corrupt.fade(corrupt ? 1 : 0, fade_time);
  land_corrupt.fade(corrupt ? 1 : 0, fade_time);
  edge_corrupt.fade(corrupt ? 1 : 0, fade_time);
  convex_corrupt.fade(corrupt ? 1 : 0, fade_time);
  front.fade(corrupt ? 0 : 1, fade_time);
  front_corrupt.fade(corrupt ? 1 : 0, fade_time);
  underground_corrupt.fade(corrupt ? 1 : 0, fade_time);
  corner_corrupt.fade(corrupt ? 1 : 0, fade_time);
  corner_corrupt.fade(corrupt ? 1 : 0, fade_time);



  trees_front1.fade(corrupt ? 0 : 1, fade_time);
  trees_front2.fade(corrupt ? 0 : 1, fade_time);
  trees_front3.fade(corrupt ? 0 : 1, fade_time);
  trees_front4.fade(corrupt ? 0 : 1, fade_time);
  trees_back.fade(corrupt ? 0 : 1, fade_time);
  trees_back_corrupt.fade(corrupt ? 1 : 0, fade_time);
  decoration.fade(corrupt ? 0 : 1, fade_time);
  decoration_corrupt.fade(corrupt ? 1 : 0, fade_time);
  castle.fade(corrupt ? 0 : 1, fade_time);
  castle_corrupt.fade(corrupt ? 1 : 0, fade_time);
  trees_front1.fade(corrupt ? 0 : 1, fade_time);
  trees_front1_corrupt.fade(corrupt ? 1 : 0, fade_time);
  trees_front2.fade(corrupt ? 0 : 1, fade_time);
  trees_front2_corrupt.fade(corrupt ? 1 : 0, fade_time);

  trees_front3.fade(corrupt ? 0 : 1, fade_time);
  trees_front3_corrupt.fade(corrupt ? 1 : 0, fade_time);
  
  trees_front4.fade(corrupt ? 0 : 1, fade_time);
  state.corrupted <- corrupt;
}

corrupt_forest(state.corrupted);

// ===================================
//   CONFLICTING GAMESTATES
// ===================================

// some features of underground/corrupt states are contradicted by each other
// when run every time the WM is loaded (how it is done above),
// so here is a manual override of these features that really only affects 
// when entering/exiting level.

if (state.underground_f && !state.corrupted) {
  worldmap.settings.fade_to_ambient_light(0.4, 0.4, 0.45, fade_time_fast);
  trees_front1.fade(0, fade_time_fast);
  trees_front2.fade(0, fade_time_fast);
  trees_front3.fade(0, fade_time_fast);
  trees_front4.fade(0, fade_time_fast);
}

// ============================================================================
//   ROAD FORKS
// ============================================================================

if (!("crushmore_secret" in state))
{
  // there is no compat... we ignore the old states for now. if they beat the
  // level or didn't complete the fork in the road then things would just get
  // weird regardless, like paths being opened despite never finding the
  // secrets... since the worldmap is pretty much redone anyway, new players
  // will start from the beginning anyway hopefully.
  state.crushmore_secret <- 0;
  state.hollow_secret <- 0;
  state.strike_secret <- 0;
  state.granito_secret <- 0;
}

state.crushmore_secret <- (state.crushmore_secret | SPECIALTILE_DIR_EAST | SPECIALTILE_DIR_WEST);
state.granito_secret <- (state.granito_secret | SPECIALTILE_DIR_SOUTH);
state.hollow_secret <- (state.hollow_secret | SPECIALTILE_DIR_SOUTH);
state.strike_secret <- (state.strike_secret | SPECIALTILE_DIR_SOUTH | SPECIALTILE_DIR_NORTH );

worldmap.crushmore_secret.set_direction_mask(state.crushmore_secret);
worldmap.granito_secret.set_direction_mask(state.granito_secret);
worldmap.strike_secret.set_direction_mask(state.strike_secret);
worldmap.hollow_secret.set_direction_mask(state.hollow_secret);
// TODO: make paths fade

if (("key_knowledge" in state)) {
	  if (state.key_knowledge == true) {
        display_keys(true); }}
