function antigravity() {
  if (!("antigravity" in state))
    state.antigravity <- true
  else
    state.antigravity = true

  if ("disp1" in sector) disp1.deactivate();
  if ("disp2" in sector) disp2.deactivate();
  if ("disp3" in sector) disp3.deactivate();
  if ("disp4" in sector) disp4.deactivate();

  sector.settings.set_gravity(-3);
}

function gravity() {
  if (!("antigravity" in state))
    state.antigravity <- false
  else
    state.antigravity = false

  if ("disp1" in sector) disp1.activate();
  if ("disp2" in sector) disp2.activate();
  if ("disp3" in sector) disp3.activate();
  if ("disp4" in sector) disp4.activate();

  sector.settings.set_gravity(10);
}
