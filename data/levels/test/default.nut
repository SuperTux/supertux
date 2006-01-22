/* Default functions for the whole levelset */

function intro()
{
  DisplayEffect.sixteen_to_nine();
  PENNY.set_action("stand");
  Text.set_centered(true);
  Text.set_text(translate("SuperTux\nMilestone 2"));
  Text.fade_in(2);
  wait(4);
  Text.fade_out(1);
}
