
function end_level()
{
  Sound.play_music("leveldone");
  Tux.deactivate();
  wait(6);
  DisplayEffect.fade_out(2);
  wait(2);
  Level.finish();
}

function levelflip()
{
  DisplayEffect.fade_out(1);
  wait(1);
  Level.flip_vertically();
  DisplayEffect.fade_in(1);
}
