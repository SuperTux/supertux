
function get_gold_key()
{
  add_key(KEY_GOLD);
  end_level();
}

function end_level()
{
  Sound.play_music("leveldone");
  wait(6);
  DisplayEffect.fade_out(2);
  wait(2);
  Level.finish();
}
