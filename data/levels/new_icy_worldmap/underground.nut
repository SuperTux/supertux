if(! ("underground" in state)){
	state.underground <- false;
  print("underground state initialized\n");
}

function go_underground(under){
  foreground_far.fade(under ? 0 : 1, 1);
  foreground_mid.fade(under ? 0 : 1, 1);
  foreground_near.fade(under ? 0 : 1, 1);
  underground.fade(under ? 1 : 0, 1);
  underground_mask.fade(under ? 1 : 0, 1);
  underground_shade.fade(under ? 1 : 0, 1);
  state.underground <- under;
}

go_underground(state.underground);
print("worldmap set\n");