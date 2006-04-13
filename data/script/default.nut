
function end_level()
{
  Sound.play_music("music/leveldone.ogg");
  Tux.deactivate();
  wait(6);
  Effect.fade_out(2);
  wait(2);
  Level.finish(true);
}

function levelflip()
{
  Effect.fade_out(1);
  wait(1);
  Level.flip_vertically();
  Effect.fade_in(1);
}
