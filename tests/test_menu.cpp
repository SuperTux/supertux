#include "gui/menu.hpp"
#include "gui/item_action.hpp"
#include "supertux/menu_action.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Menu basic item management", "[gui][menu]") {
  Menu menu;

  SECTION("Add a non-skippable item and check active index") {
    auto& item = menu.add_entry(1, "Start Game");
    REQUIRE_FALSE(item.skippable());
    REQUIRE(menu.get_active_item_index() == 0);
  }

  SECTION("Add a skippable item and ensure active index is not set") {
    struct DummySkippableItem : public MenuItem {
      bool skippable() const override { return true; }
      void draw(DrawingContext&, const Vector&, int, bool) override {}
    };

    menu.add_item(std::make_unique<DummySkippableItem>());
    REQUIRE(menu.get_active_item_index() == -1);
  }

  SECTION("Delete an item and ensure active item adjusts") {
    menu.add_entry(1, "Play");
    menu.add_entry(2, "Options");
    REQUIRE(menu.get_active_item_index() == 0);

    menu.delete_item(0);
    REQUIRE(menu.get_active_item_index() == 0);  // "Options" moves to index 0
  }

  SECTION("Clear the menu") {
    menu.add_entry(1, "Play");
    menu.clear();
    REQUIRE(menu.get_active_item_index() == -1);
    REQUIRE(menu.get_item_count() == 0);
  }
}

TEST_CASE("Menu navigation", "[gui][menu]") {
  Menu menu;
  menu.add_entry(1, "Play");
  menu.add_entry(2, "Options");
  menu.add_entry(3, "Exit");

  SECTION("Navigate down") {
    menu.process_action(MenuAction::DOWN);
    REQUIRE(menu.get_active_item_index() == 1);
  }

  SECTION("Navigate up wraps around") {
    menu.process_action(MenuAction::UP);
    REQUIRE(menu.get_active_item_index() == 2);
  }

  SECTION("Navigate down wraps around") {
    menu.process_action(MenuAction::DOWN);
    menu.process_action(MenuAction::DOWN);
    menu.process_action(MenuAction::DOWN);
    REQUIRE(menu.get_active_item_index() == 0);
  }
}

/* EOF */
