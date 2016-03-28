/**
 * This script is loaded into the console script interpreter.
 * You should define shortcuts and helper functions that are useful for the
 * console here
 */

function flip()
{
	Level.flip_vertically();
}

function finish()
{
	Level.finish(true);
}

function edit()
{
	Level.edit(true);
}

function play()
{
	Level.edit(false);
}

function worldmapfinish()
{
	foreach(world in state.worlds) {
		foreach(levelname, level in world.levels) {
			level.solved = true;
		}
	}
	load_state();
}

function grow()
{
	sector.Tux.add_bonus("grow");
}

function make_invincible()
{
      sector.Tux.make_invincible();
}

function fire()
{
	sector.Tux.add_bonus("fireflower");
}

function ice()
{
	sector.Tux.add_bonus("iceflower");
}

function air()
{
	sector.Tux.add_bonus("airflower");
}

function earth()
{
	sector.Tux.add_bonus("earthflower");
}

function shrink()
{
	sector.Tux.kill(false);
}

function kill()
{
	sector.Tux.kill(true);
}

function lifeup()
{
	sector.Tux.add_coins(100);
}

function none()
{
	sector.Tux.set_bonus("none");
}

function help()
{
	println("This is the console, enabled with developer mode. You can run the same scripts as you can in-level, but make sure to put 'sector.' before a command for a sector command. You can also run scripts defined in scripts/console.nut."); 
}

/**
 * Display a list of functions in the roottable (or in the table specified)
 */
function functions(...)
{
	local obj = this;
	if(vargv.len() == 1)
		obj = vargv[0];
	if(::type(obj) == "instance")
		obj = obj.getclass()

	foreach(key, val in obj) {
		if(::type(val) == "function")
			println(key);
	}
}
