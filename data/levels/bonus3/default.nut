//INITIALIZATION OF STATE VARIABLES
key_names <- [ "darkness" "unlicensed"]

if(!("keys" in state)){
  state.keys <- {};
  foreach(name in key_names){
    state.keys[name] <- false;
  }
  print("[DEBUG] key state initialized\n");
}

status <- state.keys;

function set_darkness(night){
  dark.fade(night ? 0.6 : 0, 1);
  status.darkness <- night;
  save_keys();
}

function set_unlicensed(under){
  if(true||!(under && status.unlicensed)){
    speed <- 0.5;
    pathing.fade(under ? 0 : 1, speed);
    land_detail_far.fade(under ? 0 : 1, speed);
    land_detail_near.fade(under ? 0 : 1, speed);
    foreground_far.fade(under ? 0 : 1, speed);
    foreground_near.fade(under ? 0 : 1, speed);
    map.fade(under ? 0 : 1, speed);
    exit.fade(under ? 0 : (outpath ? 1 : 1), speed);
    noexit.fade(under ? 0 : (outpath ? 0 : 0), speed);
    u_pathing.fade(under ? 1 : 0, speed);
    u_map.fade(under ? 1 : 0, speed);
    u_background.fade(under ? 1 : 0, speed);
    u_foreground.fade(under ? 1 : 0, speed);
    status.unlicensed <- under;
    save_keys();
    //print("[DEBUG] un "+under+"\n");
    play_music(under ? "music/voc-night.music" : "music/voc-daytime2.music");
  }
}

function query(item){return status[item];}

function fire_bonus(){
  Tux.deactivate();
  Tux.add_bonus("fireflower");
  play_sound("sounds/fire-flower.wav");
  print("fire bonus added\n");
  wait(1);
  Effect.fade_out(2);
  wait(2);
  Level.finish(true);
}

function ice_bonus(){
  Tux.deactivate();
  Tux.add_bonus("iceflower");
  play_sound("sounds/fire-flower.wav");
  print("ice bonus added\n");
  wait(1);
  Effect.fade_out(2);
  wait(2);
  Level.finish(true);
}

function save_keys(){state.keys <- status;}

function reset_keys(){
  foreach(name in key_names){
    status[name] <- false;
  }
  save_keys();
}

if("map" in this){
  outpath <- state.worlds["levels/bonus3/worldmap.stwm"].levels["sever_escape.stl"].solved;
  set_darkness(status.darkness);
  set_unlicensed(status.unlicensed);
  wait(0.1);
  if(!status.unlicensed){
    exit.fade(outpath ? 1 : 0, 0);
    noexit.fade(outpath ? 0 : 1, 0);
  }
  //print("[DEBUG] map set\n");
}