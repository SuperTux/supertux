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
println("Commands:"); 
println("flip(): Flip the level vertically."); 
println("finish(): Complete the level."); 
println("edit(): Place the game into an edit mode, so when Tux dies or reaches a point he will be in ghost mode."); 
println("play(): Take the game out of the edit mode (will restart the level)."); 
println("worldmapfinish(): Whilst viewing the worldmap, this command will complete every level in that worldmap.");
println("grow(): Grows Tux as if he had taken an egg.");
println("fire(): Makes Tux Fire Tux as if he had taken a fireflower."); 
println("ice(): Makes Tux Ice Tux as if he had taken an iceflower.");
println("air(): Makes Tux Air Tux as if he had taken an airflower.");
println("earth(): Makes Tux Earth Tux as if he had taken an earthflower.");
println("shrink(): Hurts Tux.");
println("none(): Makes Tux little Tux.");
println("make_invincible(): Makes Tux invincible as if he had taken a star powerup.");
println("lifeup(): Gives Tux 100 coins.");
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
