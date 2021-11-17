#ifndef HEADER_SUPERTUX_GUI_MENU_LIST_HPP
#define HEADER_SUPERTUX_GUI_MENU_LIST_HPP

#include "gui/menu.hpp"

class ListMenu final : public Menu 
{
public:
    ListMenu(std::vector<std::string>* items_, int* selected_);

    void menu_action(MenuItem& item);

private:
    std::vector<std::string>* items;
    int* selected;

private:
  // non-copyable footer
  ListMenu(const ListMenu&) = delete;
  ListMenu& operator=(const ListMenu&) = delete;
};

#endif

/* EOF */
