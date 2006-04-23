

function get_gold_key()
{
  add_key(KEY_GOLD);
  end_level();
}

function level2_init()
{
  add_key(KEY_BRASS);
  add_key(KEY_IRON);
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
if(! ("brass" in keys))
	keys.brass <- false;
if(! ("iron" in keys))
	keys.iron <- false;
if(! ("bronze" in keys))
	keys.bronze <- false;
if(! ("silver" in keys))
	keys.silver <- false;
if(! ("gold" in keys))
	keys.gold <- false;

/// this function updates the key images (call this if tux has collected a key)
function update_keys()
{
	local keys = state.world2_keys;
	key_brass.set_action(keys.brass ? "display" : "outline");
	key_iron.set_action(keys.iron ? "display" : "outline");
	key_bronze.set_action(keys.bronze ? "display" : "outline");
	key_silver.set_action(keys.silver ? "display" : "outline");
	key_gold.set_action(keys.gold ? "display" : "outline");
}

local x = 10;
local y = 10;

key_brass <- FloatingImage("images/objects/keys/key_brass.sprite");
key_brass.set_anchor_point(ANCHOR_TOP_LEFT);
key_brass.set_pos(x, y);
key_brass.set_visible(true);
x += 30;

key_iron <- FloatingImage("images/objects/keys/key_iron.sprite");
key_iron.set_anchor_point(ANCHOR_TOP_LEFT);
key_iron.set_pos(x, y);
key_iron.set_visible(true);
x += 30;

key_bronze <- FloatingImage("images/objects/keys/key_bronze.sprite");
key_bronze.set_anchor_point(ANCHOR_TOP_LEFT);
key_bronze.set_pos(x, y);
key_bronze.set_visible(true);
x += 30;

key_silver <- FloatingImage("images/objects/keys/key_silver.sprite");
key_silver.set_anchor_point(ANCHOR_TOP_LEFT);
key_silver.set_pos(x, y);
key_silver.set_visible(true);
x += 30;

key_gold <- FloatingImage("images/objects/keys/key_gold.sprite");
key_gold.set_anchor_point(ANCHOR_TOP_LEFT);
key_gold.set_pos(x, y);
key_gold.set_visible(true);
x += 30;

update_keys();
