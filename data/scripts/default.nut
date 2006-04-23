/**
 * This script gets loaded into the squirrel root vm in supertux. So functions
 * and variables you define here can be used in all threads
 */


// TODO: move this to world2
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

function println(val)
{
	print(val);
	print("\n");
}

if(! ("state" in this))
  state <- {};

