function add_key(key)
{
  local keys = state.treasure_keys;
  keys[key] = true;
  if(! ("key_knowledge" in state))
	  state.key_knowledge <- true
  update_keys();
}

function remove_key(key)
{
  local keys = state.treasure_keys;
  keys[key] = false;
  update_keys();
}

function treasure_init()
{
  foreach(name in ["air", "earth", "wood", "fire", "water"])
  {
    add_key(name);
  }
  Tux.deactivate();
  Effect.sixteen_to_nine(2);
  Text.set_text(translate("---Insert Cutscene Here---"));
  Tux.walk(100);
  Text.fade_in(2);
  wait(4);
  Text.fade_out(1);
  wait(1);
  Effect.four_to_three();
  Tux.activate();
}

/***************************************
 * Handling of the "keys" in the world *
 ***************************************/
if(! ("treasure_keys" in state))
	state.treasure_keys <- {}
	
local keys = state.treasure_keys;
foreach(name in ["air", "earth", "wood", "fire", "water"])
{
  if(! (name in keys))
    keys[name] <- false;
}

/// this function updates the key images (call this if tux has collected a key)
function update_keys()
{
	local keys = state.treasure_keys;
	foreach(name in ["air", "earth", "wood", "fire", "water"])
	{
	  key[name].set_action(keys[name] ? "display" : "outline");
	}
}

function display_keys(visibility)
{
  foreach(name in ["air", "earth", "wood", "fire", "water"])
  {
    key[name].set_visible(visibility);
  }
}

if(! ("key" in this))
 key <- {};

local x = 0;
local y = 85;

foreach(name in ["air", "earth", "wood", "fire", "water"])
{
  if(! (name in key) ) {
    key[name] <- FloatingImage("images/objects/keys/key_" + name + ".sprite");
    key[name].set_anchor_point(ANCHOR_TOP_LEFT);
    key[name].set_pos(x, y);
    key[name].set_visible(false);
  }
  y += 35;
}

update_keys();
display_keys(false);
