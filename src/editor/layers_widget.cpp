//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "editor/layers_widget.hpp"

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "editor/editor_layer.hpp"
#include "editor/menu_list_item.hpp"
#include "editor/menu_list_widget.hpp"
#include "editor/object_menu.hpp"
#include "editor/tilebox.hpp"
#include "editor/tip.hpp"
#include "gui/menu_manager.hpp"
#include "math/vector.hpp"
#include "object/camera.hpp"
#include "object/path_gameobject.hpp"
#include "object/tilemap.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

#include <fmt/format.h>

EditorLayersWidget::EditorLayersWidget(Editor& editor) :
  m_editor(editor),
  m_editor_layers(),
  m_selected_tilemap(),
  m_add_icon("", "images/engine/editor/add.png"),
  m_add_layer_box(new EditorTilebox(editor, Rectf())),
  m_add_layer_box_visible(false),
  m_Ypos(448),
  m_Width(512),
  m_scroll(0),
  m_scroll_speed(0),
  m_sector_text(),
  m_sector_text_width(0),
  m_hovered_item(HoveredItem::NONE),
  m_hovered_layer_idx(),
  m_object_tip(new Tip()),
  m_layer_list_widget(std::make_unique<MenuListWidget>()),
  m_layer_list_widget_visible(),
  m_has_mouse_focus(false)
{
  m_add_layer_box->on_select([this](EditorTilebox& tilebox)
    {
      assert(m_editor.get_input_mode() == Editor::InputMode::OBJECT);

      const auto& object_class_name = m_editor.get_selected_object_class_name();
      auto game_object = GameObjectFactory::instance().create(object_class_name);
      m_editor.get_project()->get_sector()->add_object(std::move(game_object));
      m_add_layer_box_visible = false;
    });
}

void
EditorLayersWidget::draw(DrawingContext& context)
{
  // TODO: `LINE_THICKNESS` and `line_color` were copied from Editor::draw.
  // We need to find a better place for them, so as to not duplicate code.

  constexpr float LINE_THICKNESS = 1.f;
  Color line_color = (g_config->editorcolor - Color(0.2, 0.2, 0.2, 0.2)).validate();
  Rectf layers_rect = Rectf{0, SCREEN_HEIGHT - 32.f - LINE_THICKNESS,
                            SCREEN_WIDTH - 128.f, SCREEN_HEIGHT - 32.f};
  context.color().draw_filled_rect(layers_rect, line_color, LAYER_GUI + 1);

  if (m_add_layer_box_visible)
  {
    m_add_layer_box->draw(context);
  }
  else if (m_object_tip->get_visible())
  {
    auto position = get_layer_coords(m_hovered_layer_idx);
    m_object_tip->draw_up(context, position);
  }

  if(m_layer_list_widget_visible)
  {
    m_layer_list_widget->draw(context);
  }

  context.color().set_blur(g_config->editor_blur);
  context.color().draw_filled_rect(Rectf(Vector(0, static_cast<float>(m_Ypos)),
                                         Vector(static_cast<float>(m_Width), static_cast<float>(SCREEN_HEIGHT))),
                                     g_config->editorcolor,
                                     0.0f,
                                     LAYER_GUI-10);
  context.color().set_blur(0);

  Rectf target_rect = Rectf(0, 0, 0, 0);
  bool draw_rect = true;

  switch (m_hovered_item)
  {
    case HoveredItem::SPAWNPOINTS:
      target_rect = Rectf(Vector(0, static_cast<float>(m_Ypos)),
                          Vector(static_cast<float>(m_Xpos), static_cast<float>(SCREEN_HEIGHT)));
      break;

    case HoveredItem::SECTOR:
      target_rect = Rectf(Vector(static_cast<float>(m_Xpos), static_cast<float>(m_Ypos)),
                          Vector(static_cast<float>(m_sector_text_width + m_Xpos), static_cast<float>(SCREEN_HEIGHT)));
      break;

    case HoveredItem::LAYERS:
      {
        Vector coords = get_layer_coords(m_hovered_layer_idx);
        target_rect = Rectf(coords, coords + Vector(32, 32));
      }
      break;

    default:
      draw_rect = false;
      break;
  }

  if (draw_rect)
  {
    context.color().draw_filled_rect(target_rect, g_config->editorhovercolor, 0.0f,
                                       LAYER_GUI-5);
  }

  if (!m_editor.get_project()->is_level_loaded()) {
    return;
  }

  context.color().draw_text(Resources::normal_font, m_sector_text,
                            Vector(35.0f, static_cast<float>(m_Ypos) + 5.0f),
                            ALIGN_LEFT, LAYER_GUI, ColorScheme::Menu::default_color);

#if 0 // Hard disabled because this isn't really even usable, anyway
  if (g_config->editor_show_properties_sidebar)
    context.color().draw_filled_rect(Rectf(Vector(0, SCREEN_HEIGHT - 400), Vector(200, SCREEN_HEIGHT)),
                                     g_config->editorhovercolor, 0.0f, LAYER_GUI - 10);
#endif

  int pos = 0;
  for (const auto& editor_layer : m_editor_layers)
  {
    if (!editor_layer->is_valid()) continue;

    if (pos * 35 >= m_scroll)
      editor_layer->draw(context, get_layer_coords(pos));
    else if ((pos + 1) * 35 >= m_scroll)
      editor_layer->draw(context, get_layer_coords(pos), 35 - (m_scroll - pos * 35));

#if 0 // Same reason as above
    if (g_config->editor_show_properties_sidebar)
    {
      auto editor_layer_position = Vector(0, SCREEN_HEIGHT - 400 + (pos * 30));

      editor_layer->draw(context, editor_layer_position);

      auto layer_name = editor_layer->get_layer()->get_name();
      if (layer_name.empty())
      {
        layer_name =
          fmt::format(fmt::runtime(_("Unnamed {}")), editor_layer->get_layer()->get_display_name());
      }
      context.color().draw_text(Resources::small_font, layer_name,
                                editor_layer_position + Vector(35, 10), FontAlignment::ALIGN_LEFT,
                                LAYER_GUI - 9);
    }
#endif

    pos++;
  }
  if (pos * 35 >= m_scroll)
    m_add_icon.draw(context, get_layer_coords(pos));
  else if ((pos + 1) * 35 >= m_scroll)
    m_add_icon.draw(context, get_layer_coords(pos), 35 - (m_scroll - pos * 35));
}

void
EditorLayersWidget::update(float dt_sec)
{
  remove_invalid_layers();

  auto selected_tilemap = get_selected_tilemap();
  if (!selected_tilemap)
  {
    // Ensure a tilemap is always selected
    refresh_layers();
  }

  if(m_scroll_speed < 0 && m_scroll > 0)
  {
    m_scroll -= 5;
  }
  else if (m_scroll_speed > 0 && m_scroll < (static_cast<int>(m_editor_layers.size()) - 1) * 35)
  {
    m_scroll += 5;
  }
}

bool
EditorLayersWidget::event(const SDL_Event& ev)
{
  bool result = Widget::event(ev);

  if (m_add_layer_box_visible)
    result |= m_add_layer_box->event(ev);

  if (m_layer_list_widget_visible)
    result |= m_layer_list_widget->event(ev);

  return result;
}

bool
EditorLayersWidget::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  return m_layer_list_widget->on_mouse_button_up(button);
}

bool
EditorLayersWidget::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  if (m_layer_list_widget_visible && m_layer_list_widget->on_mouse_button_down(button))
    return true;

  auto hovered_item = m_hovered_item;
  m_hovered_item = HoveredItem::NONE;

  if (button.button == SDL_BUTTON_LEFT)
  {
    auto editor_events = m_editor.get_event_handling();
    auto mouse_pos = editor_events->get_mouse_pos();
    auto add_layer_box_rect = m_add_layer_box->get_rect();
    if (m_add_layer_box_visible && !add_layer_box_rect.contains(mouse_pos))
    {
      m_add_layer_box_visible = false;
      return true;
    }

    switch (hovered_item)
    {
      case HoveredItem::SECTOR:
        MenuManager::instance().set_menu(MenuStorage::EDITOR_SECTORS_MENU);
        return true;

      case HoveredItem::LAYERS:
        if (m_hovered_layer_idx > m_editor_layers.size())
        {
          return false;
        }

        if (m_hovered_layer_idx == m_editor_layers.size()) // Add layer button
        {
          m_add_layer_box_visible = !m_add_layer_box_visible;
          if (m_add_layer_box_visible)
          {
            m_add_layer_box->select_layers_objectgroup();

            const Vector coords = get_layer_coords(static_cast<int>(m_editor_layers.size()));
            m_add_layer_box->set_rect(Rectf(coords - Vector(48.f, std::max(m_add_layer_box->get_tiles_height(), 32.f)),
                                            coords + Vector(80.f, 0.f)));
          }
        }
        else
        {
          auto overlay_widget = m_editor.get_overlay_widget();
          auto& hovered_layer = get_hovered_layer();
          auto hovered_object = hovered_layer->get_game_object();

          if (auto tilemap = hovered_layer->get_tilemap())
          {
            set_selected_tilemap(tilemap);
          }

          if (hovered_object->can_follow_path())
          { 
            overlay_widget->edit_object_path(hovered_object);
            m_editor.set_selected_object(hovered_object);
          }
        }
        return true;
      
      default:
        return false;
      }
  }
  else if (button.button == SDL_BUTTON_RIGHT)
  {
    if (hovered_item == HoveredItem::LAYERS && m_hovered_layer_idx < m_editor_layers.size())
    {
      auto layer_object = get_hovered_layer()->get_game_object();
      MenuManager::instance().push_menu(std::make_unique<ObjectMenu>(layer_object));
      m_editor.set_selected_object(layer_object);
      return true;
    }
  }

  return false;
}

bool
EditorLayersWidget::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  if (m_layer_list_widget_visible && m_layer_list_widget->on_mouse_motion(motion))
    return true;

  Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);

  if (m_hovered_item != HoveredItem::SECTOR && m_layer_list_widget_visible && !m_layer_list_widget->get_area().contains(mouse_pos))
  {
    m_layer_list_widget_visible = false;
    return true;
  }

  float x = mouse_pos.x - static_cast<float>(m_Xpos);
  float y = mouse_pos.y - static_cast<float>(m_Ypos);

#if 0
  if (g_config->editor_show_properties_sidebar)
  {
    // LAYERS_BOX_EXPERIMENT_BEGIN
    if (mouse_pos.y > SCREEN_HEIGHT - 400 && mouse_pos.x < 200)
    {
      int layer_box_mouse_pos = mouse_pos.y - 400;
      m_hovered_item = HoveredItem::LAYERS;
      m_hovered_layer_idx = static_cast<int>(layer_box_mouse_pos / 30.0f);
      update_tip();
      return true;
    }
    // LAYERS_BOX_EXPERIMENT_END
  }
#endif

  if (y < 0 || x > static_cast<float>(m_Width)) {
    m_hovered_item = HoveredItem::NONE;
    m_object_tip->set_visible(false);
    m_has_mouse_focus = false;
    m_scroll_speed = 0;
    return false;
  }

  m_has_mouse_focus = true;

  if (x < 0) {
    m_hovered_item = HoveredItem::SPAWNPOINTS;
    m_object_tip->set_visible(false);
    return true;
  }

  if (x <= static_cast<float>(m_sector_text_width)) {
    m_hovered_item = HoveredItem::SECTOR;
    m_layer_list_widget_visible = true;
    m_object_tip->set_visible(false);
  } else {
    // Scrolling
    if (x < static_cast<float>(m_sector_text_width + 32))
    {
      m_scroll_speed = -1;
      m_add_layer_box_visible = false;
    }
    else if (x > static_cast<float>(SCREEN_WIDTH - 160)) // 160 = 128 + 32
    {
      m_scroll_speed = 1;
      m_add_layer_box_visible = false;
    }
    else
    {
      m_scroll_speed = 0;
    }

    unsigned int new_hovered_layer_idx = get_layer_idx(mouse_pos);
    if (m_hovered_layer_idx != new_hovered_layer_idx || m_hovered_item != HoveredItem::LAYERS) {
      m_hovered_layer_idx = new_hovered_layer_idx;
      update_tip();
    }
    m_hovered_item = HoveredItem::LAYERS;
  }

  return true;
}


bool
EditorLayersWidget::on_mouse_wheel(const SDL_MouseWheelEvent& wheel)
{
  if (!m_has_mouse_focus)
  {
    return false;
  }

  if((wheel.x < 0 || wheel.y < 0) && !(wheel.x > 0 || wheel.y > 0))
  {
    if (m_scroll >= 16)
    {
      m_scroll -= 16;
    }
    else
    {
      m_scroll = 0;
    }
  }
  else if ((wheel.x > 0 || wheel.y > 0) && !(wheel.x < 0 || wheel.y < 0))
  {
    auto last_item_position = (static_cast<int>(m_editor_layers.size()) - 1) * 35;
    if (m_scroll < last_item_position)
    {
      m_scroll += 16;
    }
    else
    {
      m_scroll = last_item_position;
    }
  }

  return false;
}

bool
EditorLayersWidget::has_mouse_focus() const
{
  return m_has_mouse_focus;
}

void
EditorLayersWidget::on_window_resize()
{
  m_Ypos = SCREEN_HEIGHT - 32;
  m_Width = SCREEN_WIDTH - 128;

  m_layer_list_widget->set_position(Vector(m_Xpos, m_Ypos), true);
}

void
EditorLayersWidget::setup()
{
  on_window_resize();
}

void
EditorLayersWidget::refresh()
{
  auto editor_project = m_editor.get_project();
  auto sector = editor_project->get_sector();

  if (sector == nullptr)
    return;

  m_editor_layers.clear();

  for (const auto& obj : sector->get_objects())
    add_layer(obj.get(), /* sort_layers_after_adding = */ false);

  sort_layers();

  populate_layers_list_widget();

  refresh_sector_text();
}

void
EditorLayersWidget::populate_layers_list_widget()
{
  m_layer_list_widget->clear_items();

  auto layer_icon_rect = Rectf(0, 0, 16, 16);
  for (const auto &editor_layer : m_editor_layers)
  {
    auto select_layer = [this, tilemap = editor_layer->get_tilemap()]
    {
      if (tilemap == nullptr)
        return;

      set_selected_tilemap(tilemap);
    };

    auto menu_item =
      std::make_unique<MenuListItem>(editor_layer->get_display_name(), select_layer, 
                                     editor_layer->get_game_object()->get_icon_path(), layer_icon_rect);

    auto is_selected_layer = [tilemap = editor_layer->get_tilemap()]
    {
      return tilemap != nullptr && tilemap->is_selected_in_editor();
    };

    menu_item->set_is_selected_handler(is_selected_layer);

    m_layer_list_widget->add_item(std::move(menu_item));
  }
}

void
EditorLayersWidget::refresh_sector_text()
{
  auto sector = m_editor.get_project()->get_sector();
  m_sector_text = fmt::format(fmt::runtime(_("Sector: {}")), sector->get_name());
  m_sector_text_width  = int(Resources::normal_font->get_text_width(m_sector_text)) + 6;
}

void
EditorLayersWidget::refresh_layers()
{
  bool tilemap_selected = false;
  TileMap* first_tilemap = nullptr;
  for (const auto& layer : m_editor_layers)
  {
    if (!layer->is_tilemap())
      continue;

    auto tm = layer->get_tilemap();
    if (!tm)
      continue;

    if (first_tilemap == nullptr)
    {
      first_tilemap = tm;
    }

    if (tm->is_solid() && !tilemap_selected)
    {
      set_selected_tilemap(tm);
      tilemap_selected = true;
    }
  }

  if (!tilemap_selected && first_tilemap != nullptr)
  {
    set_selected_tilemap(first_tilemap);
  }
}

void
EditorLayersWidget::sort_layers()
{
  auto layers_comparator = [](const std::unique_ptr<EditorLayer> &lhs, const std::unique_ptr<EditorLayer> &rhs)
  {
    return lhs->get_zpos() < rhs->get_zpos();
  };
  std::sort(m_editor_layers.begin(), m_editor_layers.end(), layers_comparator);
}

void
EditorLayersWidget::add_layer(GameObject* object, bool sort_layers_after_adding)
{
  if (!object->has_settings())
    return;

  auto* layer = dynamic_cast<LayerObject*>(object);
  if (!layer)
    return;

  auto editor_layer = std::make_unique<EditorLayer>(layer);
  m_editor_layers.push_back(std::move(editor_layer));

  if (sort_layers_after_adding)
  {
    sort_layers();
  }
}

void
EditorLayersWidget::update_tip()
{
  remove_invalid_layers();

  if (m_hovered_layer_idx == m_editor_layers.size())
     m_object_tip->set_info(_("Add Layer"));
  else if (m_hovered_layer_idx > m_editor_layers.size())
    m_object_tip->set_visible(false);
  else
    m_object_tip->set_info_for_object(*get_hovered_layer()->get_game_object());
}

void
EditorLayersWidget::update_current_tip()
{
  if (!m_object_tip->get_visible())
    return;

  update_tip();
}

void
EditorLayersWidget::remove_invalid_layers()
{
  auto remove_predicate = [](const std::unique_ptr<EditorLayer> &layer)
  {
    return layer && !layer->is_valid();
  };

  m_editor_layers.erase(std::remove_if(
      m_editor_layers.begin(), m_editor_layers.end(),
      remove_predicate), m_editor_layers.end());
}

TileMap*
EditorLayersWidget::get_selected_tilemap() const
{
  auto editor_project = m_editor.get_project();
  auto sector = editor_project->get_sector();
  return sector->get_object_by_uid<TileMap>(m_selected_tilemap);
}

void
EditorLayersWidget::set_selected_tilemap(TileMap* tilemap)
{
  m_selected_tilemap = tilemap->get_uid();
}

Vector
EditorLayersWidget::get_layer_coords(const int layer_idx) const
{
  return Vector(static_cast<float>(layer_idx * 35 + m_Xpos + m_sector_text_width - m_scroll),
                static_cast<float>(m_Ypos));
}

int
EditorLayersWidget::get_layer_idx(const Vector& coords) const
{
  return static_cast<int>((coords.x - static_cast<float>(m_Xpos - m_scroll) - static_cast<float>(m_sector_text_width)) / 35.0f);
}
