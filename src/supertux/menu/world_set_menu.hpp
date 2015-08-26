#ifndef WORLD_SET_MENU_HPP_INCLUDED
#define WORLD_SET_MENU_HPP_INCLUDED

#include "gui/menu.hpp"

enum WorldSetMenuIDs
{
  WORLDSET_CAMPAIGN1,
  WORLDSET_CAMPAIGN2,
  WORLDSET_CONTRIB
//WORLDSET_ADDON
};

class World;

class WorldSetMenu : public Menu
{
public:
  WorldSetMenu();
  
  void menu_action(MenuItem* item) override;
};


#endif // WORLD_SET_MENU_HPP_INCLUDED