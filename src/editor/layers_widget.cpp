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
#include "editor/layer_icon.hpp"
#include "editor/object_menu.hpp"
#include "editor/tip.hpp"
#include "gui/menu_manager.hpp"
#include "math/vector.hpp"
#include "object/camera.hpp"
#include "object/path_gameobject.hpp"
#include "object/tilemap.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

EditorLayersWidget::EditorLayersWidget(Editor& editor) :
  m_editor(editor),
  m_layer_icons(),
  m_selected_tilemap(),
  m_Ypos(448),
  m_Width(512),
  m_scroll(0),
  m_scroll_speed(0),
  m_sector_text(),
  m_sector_text_width(0),
  m_hovered_item(HoveredItem::NONE),
  m_hovered_layer(-1),
  m_object_tip(),
  m_has_mouse_focus(false)
{
}

void
EditorLayersWidget::draw(DrawingContext& context)
{

  if (m_object_tip) {
    auto position = get_layer_coords(m_hovered_layer);
    m_object_tip->draw_up(context, position);
  }

  context.color().draw_filled_rect(Rectf(Vector(0, static_cast<float>(m_Ypos)),
                                         Vector(static_cast<float>(m_Width), static_cast<float>(SCREEN_HEIGHT))),
                                     Color(0.9f, 0.9f, 1.0f, 0.6f),
                                     0.0f,
                                     LAYER_GUI-10);

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
        Vector coords = get_layer_coords(m_hovered_layer);
        target_rect = Rectf(coords, coords + Vector(32, 32));
      }
      break;

    default:
      draw_rect = false;
      break;
  }

  if (draw_rect)
  {
    context.color().draw_filled_rect(target_rect, Color(0.9f, 0.9f, 1.0f, 0.6f), 0.0f,
                                       LAYER_GUI-5);
  }

  if (!m_editor.is_level_loaded()) {
    return;
  }

  context.color().draw_text(Resources::normal_font, m_sector_text,
                            Vector(35.0f, static_cast<float>(m_Ypos) + 5.0f),
                            ALIGN_LEFT, LAYER_GUI, ColorScheme::Menu::default_color);

  int pos = 0;
  for (const auto& layer_icon : m_layer_icons) {
    if (layer_icon->is_valid()) {
      if (pos * 35 >= m_scroll) {
        layer_icon->draw(context, get_layer_coords(pos));
      } else if ((pos + 1) * 35 >= m_scroll) {
        layer_icon->draw(context, get_layer_coords(pos), 35 - (m_scroll - pos * 35));
      }
    }
    pos++;
  }
}

void
EditorLayersWidget::update(float dt_sec)
{
  auto it = m_layer_icons.begin();
  while (it != m_layer_icons.end())
  {
    auto layer_icon = (*it).get();
    if (!layer_icon->is_valid())
      it = m_layer_icons.erase(it);
    else
      ++it;
  }
  
  if(m_scroll_speed < 0 && m_scroll > 0)
  {
    m_scroll -= 5;
  }
  else if (m_scroll_speed > 0 && m_scroll < (static_cast<int>(m_layer_icons.size()) - 1) * 35)
  {
    m_scroll += 5;
  }
}

bool
EditorLayersWidget::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  return false;
}

bool
EditorLayersWidget::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  if (button.button == SDL_BUTTON_LEFT)
  {
    switch (m_hovered_item)
    {
      case HoveredItem::SECTOR:
        m_editor.disable_keyboard();
        MenuManager::instance().set_menu(MenuStorage::EDITOR_SECTORS_MENU);
        return true;

      case HoveredItem::LAYERS:
        if (m_hovered_layer >= m_layer_icons.size())
        {
          return false;
        }
        else
        {
          if (m_layer_icons[m_hovered_layer]->is_tilemap()) {
            if (m_selected_tilemap) {
              m_selected_tilemap->m_editor_active = false;
            }
            m_selected_tilemap = static_cast<TileMap*>(m_layer_icons[m_hovered_layer]->get_layer());
            m_selected_tilemap->m_editor_active = true;
            m_editor.edit_path(m_selected_tilemap->get_path(), m_selected_tilemap);
          } else {
            auto cam = dynamic_cast<Camera*>(m_layer_icons[m_hovered_layer]->get_layer());
            if (cam) {
              m_editor.edit_path(cam->get_path(), cam);
            }
          }
          return true;
        }

      default:
        return false;
    }
  }
  else if (button.button == SDL_BUTTON_RIGHT)
  {
    if (m_hovered_item == HoveredItem::LAYERS && m_hovered_layer < m_layer_icons.size()) {
      auto om = std::make_unique<ObjectMenu>(m_editor, m_layer_icons[m_hovered_layer]->get_layer());
      m_editor.m_deactivate_request = true;
      MenuManager::instance().push_menu(std::move(om));
      return true;
    } else {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool
EditorLayersWidget::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  float x = mouse_pos.x - static_cast<float>(m_Xpos);
  float y = mouse_pos.y - static_cast<float>(m_Ypos);
  if (y < 0 || x > static_cast<float>(m_Width)) {
    m_hovered_item = HoveredItem::NONE;
    m_object_tip = nullptr;
    m_has_mouse_focus = false;
    m_scroll_speed = 0;
    return false;
  }

  m_has_mouse_focus = true;

  if (x < 0) {
    m_hovered_item = HoveredItem::SPAWNPOINTS;
    m_object_tip = nullptr;
    return true;
  } else {
    if (x <= static_cast<float>(m_sector_text_width)) {
      m_hovered_item = HoveredItem::SECTOR;
      m_object_tip = nullptr;
    } else {
      // Scrolling
      if (x < static_cast<float>(m_sector_text_width + 32)) {
        m_scroll_speed = -1;
      } else if (x > static_cast<float>(SCREEN_WIDTH - 160)) { // 160 = 128 + 32
        m_scroll_speed = 1;
      } else {
        m_scroll_speed = 0;
      }
      unsigned int new_hovered_layer = get_layer_pos(mouse_pos);
      if (m_hovered_layer != new_hovered_layer || m_hovered_item != HoveredItem::LAYERS) {
        m_hovered_layer = new_hovered_layer;
        update_tip();
      }
      m_hovered_item = HoveredItem::LAYERS;
    }
  }

  return true;
}


bool
EditorLayersWidget::on_mouse_wheel(const SDL_MouseWheelEvent& wheel)
{
  if (m_has_mouse_focus)
  {
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
      if (m_scroll < (static_cast<int>(m_layer_icons.size()) - 1) * 35)
      {
        m_scroll += 16;
      }
      else
      {
        m_scroll = (static_cast<int>(m_layer_icons.size()) - 1) * 35;
      }
      
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
EditorLayersWidget::resize()
{
  m_Ypos = SCREEN_HEIGHT - 32;
  m_Width = SCREEN_WIDTH - 128;
}

void
EditorLayersWidget::setup()
{
  resize();
}

void
EditorLayersWidget::refresh()
{
  m_selected_tilemap = nullptr;
  m_layer_icons.clear();

  bool tsel = false;
  for (auto& i : m_editor.get_sector()->get_objects())
  {
    auto* go = i.get();
    auto* mo = dynamic_cast<MovingObject*>(go);
    if (!mo && go->has_settings()) {
      if (!dynamic_cast<PathGameObject*>(go)) {
        add_layer(go);
      }

      auto tm = dynamic_cast<TileMap*>(go);
      if (tm) {
        if ( !tm->is_solid() || tsel ) {
          tm->m_editor_active = false;
        } else {
          m_selected_tilemap = tm;
          tm->m_editor_active = true;
          tsel = true;
        }
      }
    }
  }

  sort_layers();
  refresh_sector_text();
}

void
EditorLayersWidget::refresh_sector_text()
{
  m_sector_text = _("Sector") + ": " + m_editor.get_sector()->get_name();
  m_sector_text_width  = int(Resources::normal_font->get_text_width(m_sector_text)) + 6;
}

void
EditorLayersWidget::sort_layers()
{
  std::sort(m_layer_icons.begin(), m_layer_icons.end(),
            [](const std::unique_ptr<LayerIcon>& lhs, const std::unique_ptr<LayerIcon>& rhs) {
              return lhs->get_zpos() < rhs->get_zpos();
            });
}

void
EditorLayersWidget::add_layer(GameObject* layer)
{
  auto icon = std::make_unique<LayerIcon>(layer);
  int z_pos = icon->get_zpos();

  // The icon is inserted to the correct position.
  for (auto i = m_layer_icons.begin(); i != m_layer_icons.end(); ++i) {
    const auto& li = i->get();
    if (li->get_zpos() < z_pos) {
      m_layer_icons.insert(i, move(icon));
      return;
    }
  }

  m_layer_icons.push_back(move(icon));
}

void
EditorLayersWidget::update_tip()
{
  if ( m_hovered_layer >= m_layer_icons.size() ) {
    m_object_tip = nullptr;
    return;
  }
  m_object_tip = std::make_unique<Tip>(*m_layer_icons[m_hovered_layer]->get_layer());
}

Vector
EditorLayersWidget::get_layer_coords(const int pos) const
{
  return Vector(static_cast<float>(pos * 35 + m_Xpos + m_sector_text_width - m_scroll),
                static_cast<float>(m_Ypos));
}

int
EditorLayersWidget::get_layer_pos(const Vector& coords) const
{
  return static_cast<int>((coords.x - static_cast<float>(m_Xpos - m_scroll) - static_cast<float>(m_sector_text_width)) / 35.0f);
}

/* EOF */
