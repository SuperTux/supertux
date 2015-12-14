print("ice_mountain.nut loaded\n");

//INITIALIZATION OF STATE VARIABLES
key_names <- ["pink" "green" "red" "yellow" "darkness" "unlicensed" "star"]

if(!("keys" in state)){
  state.keys <- {};
  foreach(name in key_names){
    state.keys[name] <- false;
  }
  print("key state initialized\n");
}

status <- state.keys;

function set_darkness(night){
  dark.fade(night ? 0.6 : 0, 1);
  status.darkness <- night;
  save_keys();
  print("dark "+night+"\n");
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
    print("un "+under+"\n");
    play_music(under ? "music/voc-night.music" : "music/voc-daytime2.music");
  }
}

function find(crystal){
  if(!status[crystal]){
    status[crystal] <- true;
    play_sound("sounds/upgrade.wav");
    save_keys();
  } else{
    play_sound("sounds/ticking.wav");
  }
}

function query(item){return status[item];}
function check(crystal){return (status.green && status.yellow && status.orange && status.pink);}

function set_bonus(){
  status.star <- true;
  wait(0.2);
  indicator.set_visible(true);
  save_keys();
  play_sound("sounds/invincible_start.ogg");
  print("invincibility mode set\n");
}

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

indicator <- FloatingImage("images/powerups/star/star.sprite");
indicator.set_anchor_point(ANCHOR_TOP_LEFT);
indicator.set_pos(5, 5);
indicator.set_visible(query("star"));

if(status.star){
  if(!("map" in this)){
    Tux.make_invincible();}
  else{
    status.star <- false;
    indicator.set_visible(false);
    save_keys();
    print("invincibility mode deactivated\n");
  }
}

if("map" in this){
  outpath <- state.worlds["levels/pre_git_bugtracker_worldmap/worldmap.stwm"].levels["sever_escape.stl"].solved;
  set_darkness(status.darkness);
  set_unlicensed(status.unlicensed);
  wait(1);
  if(!status.unlicensed){
    exit.fade(outpath ? 1 : 0, 0);
    noexit.fade(outpath ? 0 : 1, 0);
  }
  print("map set\n");
}
