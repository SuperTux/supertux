

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

// Initialize keys
if(! ("world2_keys" in state))
	state.world2_keys <- {}
	
local keys = state.world2_keys;
if(! ("brass" in keys))
	keys.brass <- false;
if(! ("gold" in keys))
	keys.gold <- false;

local x = 10;
local y = 10;

key_brass <- FloatingImage("images/objects/keys/key_brass.sprite");
key_brass.set_anchor_point(ANCHOR_TOPLEFT);
key_brass.set_pos(x, y);
key_brass.set_visible(true);
key_brass.set_action(keys.brass ? "display" : "outline");
x += 30;

key_gold <- FloatingImage("images/objects/keys/key_gold.sprite");
key_gold.set_anchor_point(ANCHOR_TOPLEFT);
key_gold.set_pos(x, y);
key_gold.set_visible(true);
key_gold.set_action(keys.gold ? "display" : "outline");
x += 30;

