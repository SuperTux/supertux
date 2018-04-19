#include "supertux/menu/confirm_dialog.hpp"
#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/gettext.hpp"

ConfirmDialog::ConfirmDialog(const std::string& title, std::function<void(void)> callback):
  m_title(title),
  m_callback(callback)
{
  // Initialize the dialog 
  add_label(_("Confirm dialog"));
  add_label(title);
  add_hl();
  add_entry(1, _("Yes"));
  add_entry(2, _("No"));
}  

ConfirmDialog::ConfirmDialog(const std::string& title, std::function<void(void)> callback, std::string& yes, std::string& no):
  m_title(title),
  m_callback(callback)
{
  // Initialize the dialog 
  add_label(_("Confirm dialog"));
  add_label(title);
  add_hl();
  add_entry(1, _(yes));
  add_entry(2, _(no));
}  
void ConfirmDialog::menu_action(MenuItem* item)
{
  if(item->id == 1)
  {
    m_callback();
    MenuManager::instance().pop_menu();
  }else if(item->id == 2)
  {
    MenuManager::instance().pop_menu();
  }
  
}