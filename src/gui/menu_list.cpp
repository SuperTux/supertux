#include "gui/menu_item.hpp"
#include "gui/menu_list.hpp"
#include "gui/menu_manager.hpp"

ListMenu::ListMenu(std::vector<std::string>* items_, int* selected_) : 
  items(items_), 
  selected(selected_)
{
  int i = 0;
  for(auto& item : *items) {
    add_entry(i, item);
    i++;
  }
  add_hl();
  add_back("OK");
}

void
ListMenu::menu_action(MenuItem& item) {
  *selected = item.get_id();
  MenuManager::instance().pop_menu();
}

/* EOF */
