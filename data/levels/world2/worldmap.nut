
fade_time <- 0.35;

// ============================================================================
//   UNDERGROUND
// ============================================================================

if(! ("underground" in state)){
	state.underground <- false;
  print("[DEBUG] Underground state initialized\n");
}

function go_underground(under){
  under ? worldmap.settings.fade_to_ambient_light(0.4, 0.4, 0.45, fade_time) : worldmap.settings.fade_to_ambient_light(1, 1, 1, fade_time);
  trees_front1.fade(under ? 0 : 1, fade_time);
  trees_front2.fade(under ? 0 : 1, fade_time);
  trees_front3.fade(under ? 0 : 1, fade_time);
  trees_front4.fade(under ? 0 : 1, fade_time);
  underground_cover.fade(under ? 0 : 1, fade_time);
  underground_mask.fade(under ? 1 : 0, fade_time);
  state.underground <- under;
}

go_underground(state.underground);



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
  trees_front3.fade(corrupt? 0 : 1, fade_time);
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



// ============================================================================
//   SECRET AREAS
// ============================================================================