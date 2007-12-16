/**
 * This script is loaded into the console script interpreter.
 * You should define shortcuts and helper functions that are usefull for the
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

function worldmapfinish()
{
	save_state();
	foreach(levelname, level in state.worlds[state.world].levels) {
		level.solved = true;
	}
	update_worldmap();
}

function grow()
{
	sector.Tux.add_bonus("grow");
}

function fire()
{
	sector.Tux.add_bonus("fireflower");
}

function ice()
{
	sector.Tux.add_bonus("iceflower");
}

function shrink()
{
	sector.Tux.add_bonus("none");
}

function kill()
{
	sector.Tux.kill(true);
}

function lifeup()
{
	sector.Tux.add_coins(100);
}

/**
 * Display a list of functions in the roottable (or in the table specified)
 */
function functions(...)
{
	local obj = this;
	if(vargc == 1)
		obj = vargv[0];
	if(::type(obj) == "instance")
		obj = obj.getclass()

	while(obj != null) {
		foreach(key, val in obj) {
			if(::type(val) == "function")
				println(key);
		}
		obj = obj.parent;
	}
}

