function add_key(key)
{
  local keys = state.world2_keys;
  keys[key] = true;
  update_keys();
  end_level();
}

function level2_init()
{
  add_key("air");
  add_key("earth");
  add_key("wood");
  add_key("fire");
  add_key("water");
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
if(! ("world2_keys" in state))
	state.world2_keys <- {}
	
local keys = state.world2_keys;
if(! ("air" in keys))
	keys.air <- false;
if(! ("earth" in keys))
	keys.earth <- false;
if(! ("wood" in keys))
	keys.wood <- false;
if(! ("fire" in keys))
	keys.fire <- false;
if(! ("water" in keys))
	keys.water <- false;

/// this function updates the key images (call this if tux has collected a key)
function update_keys()
{
	local keys = state.world2_keys;
	key_air.set_action(keys.air ? "display" : "outline");
	key_earth.set_action(keys.earth ? "display" : "outline");
	key_wood.set_action(keys.wood ? "display" : "outline");
	key_fire.set_action(keys.fire ? "display" : "outline");
	key_water.set_action(keys.water ? "display" : "outline");
}

local x = 10;
local y = 10;


key_air <- FloatingImage("images/objects/keys/key_air.sprite");
key_air.set_anchor_point(ANCHOR_TOP_LEFT);
key_air.set_pos(x, y);
key_air.set_visible(true);
x += 30;

key_earth <- FloatingImage("images/objects/keys/key_earth.sprite");
key_earth.set_anchor_point(ANCHOR_TOP_LEFT);
key_earth.set_pos(x, y);
key_earth.set_visible(true);
x += 30;

key_wood <- FloatingImage("images/objects/keys/key_wood.sprite");
key_wood.set_anchor_point(ANCHOR_TOP_LEFT);
key_wood.set_pos(x, y);
key_wood.set_visible(true);
x += 30;

key_fire <- FloatingImage("images/objects/keys/key_fire.sprite");
key_fire.set_anchor_point(ANCHOR_TOP_LEFT);
key_fire.set_pos(x, y);
key_fire.set_visible(true);
x += 30;

key_water <- FloatingImage("images/objects/keys/key_water.sprite");
key_water.set_anchor_point(ANCHOR_TOP_LEFT);
key_water.set_pos(x, y);
key_water.set_visible(true);
x += 30;

update_keys();
