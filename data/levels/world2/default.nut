
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
  DisplayEffect.sixteen_to_nine(2);
  Text.set_text(translate("---Insert Cutscene Here---"));
  Tux.walk(100);
  Text.fade_in(2);
  wait(4);
  Text.fade_out(1);
  wait(1);
  DisplayEffect.four_to_three();
  Tux.activate();
}
