#include "gui/menu_item.hpp"
#include "gui/menu_list.hpp"
#include "gui/menu_manager.hpp"

ListMenu::ListMenu(const std::vector<std::string>& items, int* selected) : 
  m_selected(selected)
{
  for(uint i = 0; i < items.size(); i++) {
    add_entry(i, items[i]);
  }
  add_hl();
  add_back("OK");
}

void
ListMenu::menu_action(MenuItem& item) {
  if(m_selected) {
    *m_selected = item.get_id();
  }
  MenuManager::instance().pop_menu();
}

/* EOF */
