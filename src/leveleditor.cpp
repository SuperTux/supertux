/***************************************************************************
                  leveleditor.cpp  -  built'in leveleditor
                     -------------------
    begin                : June, 23 2004
    copyright            : (C) 2004 by Ricardo Cruz
    email                : rick2@aeiou.pt
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <algorithm>

#include "gui/mousecursor.h"
#include "gui/menu.h"
#include "gui/button.h"
#include "audio/sound_manager.h"
#include "app/gettext.h"
#include "app/setup.h"
#include "app/globals.h"
#include "special/sprite.h"
#include "leveleditor.h"
#include "resources.h"
#include "tile.h"
#include "tilemap.h"
#include "tile_manager.h"
#include "sector.h"
#include "background.h"
#include "gameloop.h"
#include "badguy.h"
#include "gameobjs.h"
#include "door.h"
#include "camera.h"

LevelEditor::LevelEditor()
{
show_grid = true;

selection.clear();
global_frame_counter = 0;
frame_timer.init(true);
level_name_timer.init(true);
selection_end = selection_ini = Vector(0,0);
left_button = middle_button = mouse_moved =  false;
level = 0;
level_subset = 0;

cur_layer = LAYER_TILES;
level_changed = false;

sector = 0;
zoom = 1.0;

/* Creating menus */
level_subsets = FileSystem::dsubdirs("/levels", "info");
subset_menu = new Menu();
subset_menu->additem(MN_LABEL,_("Load Subset"),0,0);
subset_menu->additem(MN_HL,"",0,0);
int i = 0;
for(std::set<std::string>::iterator it = level_subsets.begin(); it != level_subsets.end(); ++it, ++i)
  subset_menu->additem(MN_ACTION, (*it),0,0,i);
subset_menu->additem(MN_HL,"",0,0);
subset_menu->additem(MN_BACK,_("Back"),0,0);

create_subset_menu = new Menu();
create_subset_menu->additem(MN_LABEL,_("New Level Subset"),0,0);
create_subset_menu->additem(MN_HL,"",0,0);
create_subset_menu->additem(MN_TEXTFIELD,_("Filename   "),0,0,MN_ID_FILENAME_SUBSET);
create_subset_menu->additem(MN_TEXTFIELD,_("Title      "),0,0,MN_ID_TITLE_SUBSET);
create_subset_menu->additem(MN_TEXTFIELD,_("Description"),0,0,MN_ID_DESCRIPTION_SUBSET);
create_subset_menu->additem(MN_ACTION,_("Create"),0,0, MN_ID_CREATE_SUBSET);
create_subset_menu->additem(MN_HL,"",0,0);
create_subset_menu->additem(MN_BACK,_("Back"),0,0);

main_menu = new Menu();
main_menu->additem(MN_LABEL,_("Level Editor Menu"),0,0);
main_menu->additem(MN_HL,"",0,0);
main_menu->additem(MN_ACTION,_("Return to Level Editor"),0,0,MN_ID_RETURN);
main_menu->additem(MN_GOTO,_("Create Level Subset"),0,create_subset_menu);
main_menu->additem(MN_GOTO,_("Load Level Subset"),0,subset_menu);
main_menu->additem(MN_HL,"",0,0);
main_menu->additem(MN_ACTION,_("Quit Level Editor"),0,0,MN_ID_QUIT);

settings_menu = new Menu();
settings_menu->additem(MN_LABEL,_("Level Settings"),0,0);
settings_menu->additem(MN_HL,"",0,0);
settings_menu->additem(MN_TEXTFIELD,_("Name    "),0,0,MN_ID_NAME);
settings_menu->additem(MN_TEXTFIELD,_("Author  "),0,0,MN_ID_AUTHOR);
settings_menu->additem(MN_NUMFIELD, _("Width   "),0,0,MN_ID_WIDTH);
settings_menu->additem(MN_NUMFIELD, _("Height  "),0,0,MN_ID_HEIGHT);
settings_menu->additem(MN_HL,"",0,0);
settings_menu->additem(MN_ACTION,_("Apply"),0,0,MN_ID_APPLY_SETTINGS);

/* Creating button groups */
load_buttons_gfx();

tiles_board = new ButtonGroup(Vector(screen->w - 140, 100),
          Vector(32,32), Vector(4,8));

TileManager* tilemanager = TileManager::instance();

tiles_board->add_button(Button(img_rubber_bt, _("Eraser"), SDLKey(SDLK_DELETE)), 0);
for(unsigned int id = 1; id < tilemanager->total_ids(); id++)
  {
  Tile* tile = tilemanager->get(id);
  if(!tile)
    continue;

  Surface* surface;
  if(tile->editor_images.size())
    surface = tile->editor_images[0];
  else if(tile->images.size())
    surface = tile->images[0];
  else
    continue;

  Button button = Button(surface, "", SDLKey(0));
  tiles_board->add_button(button, id);
  }
for(int i = 0; i < NUM_BadGuyKinds; i++)
  {
  // filter bomb, since it is only for internal use, not for levels
  if(i == BAD_BOMB)
    continue;

  BadGuyKind kind = BadGuyKind(i);
  BadGuy badguy(kind, 0,0);
  badguy.activate(LEFT);

  Surface *img = badguy.get_image();
  tiles_board->add_button(Button(img, "", SDLKey(SDLK_1+i)), -(i+1));
  }

tiles_board->add_button(Button(img_trampoline[0].get_frame(0), _("Trampoline"), SDLKey(0)), OBJ_TRAMPOLINE);
tiles_board->add_button(Button(img_flying_platform->get_frame(0), _("Flying Platform"), SDLKey(0)), OBJ_FLYING_PLATFORM);
tiles_board->add_button(Button(door->get_frame(0), _("Door"), SDLKey(0)), OBJ_DOOR);

tiles_layer = new ButtonGroup(Vector(12, screen->h-64), Vector(80,20), Vector(1,3));
tiles_layer->add_button(Button(img_foreground_bt, _("Edtit foreground tiles"),
                       SDLK_F10), LAYER_FOREGROUNDTILES);
tiles_layer->add_button(Button(img_interactive_bt, _("Edit interactive tiles"),
                       SDLK_F11), LAYER_TILES, true);
tiles_layer->add_button(Button(img_background_bt, _("Edit background tiles"),
                       SDLK_F12), LAYER_BACKGROUNDTILES);

level_options = new ButtonGroup(Vector(screen->w-164, screen->h-36), Vector(32,32), Vector(5,1));
level_options->add_pair_of_buttons(Button(img_next_sector_bt, _("Next sector"), SDLKey(0)), BT_NEXT_SECTOR,
               Button(img_previous_sector_bt, _("Prevous sector"), SDLKey(0)), BT_PREVIOUS_SECTOR);
level_options->add_pair_of_buttons(Button(img_next_level_bt, _("Next level"), SDLKey(0)), BT_NEXT_LEVEL,
               Button(img_previous_level_bt, _("Prevous level"), SDLKey(0)), BT_PREVIOUS_LEVEL);
level_options->add_button(Button(img_save_level_bt, _("Save level"), SDLK_F5), BT_LEVEL_SAVE);
level_options->add_button(Button(img_test_level_bt, _("Test level"), SDLK_F6), BT_LEVEL_TEST);
level_options->add_button(Button(img_setup_level_bt, _("Setup level"), SDLK_F7), BT_LEVEL_SETUP);
}

LevelEditor::~LevelEditor()
{
free_buttons_gfx();

delete tiles_board;
delete tiles_layer;
delete level_options;

delete subset_menu;
delete create_subset_menu;
delete main_menu;
delete settings_menu;

delete level;
delete level_subset;
}

void LevelEditor::load_buttons_gfx()
{
img_foreground_bt = new Surface(datadir + "/images/leveleditor/foreground.png", true);
img_interactive_bt = new Surface(datadir + "/images/leveleditor/interactive.png", true);
img_background_bt = new Surface(datadir + "/images/leveleditor/background.png", true);

img_save_level_bt = new Surface(datadir + "/images/leveleditor/save-level.png", true);
img_test_level_bt = new Surface(datadir + "/images/leveleditor/test-level.png", true);
img_setup_level_bt = new Surface(datadir + "/images/leveleditor/setup-level.png", true);

img_rubber_bt = new Surface(datadir + "/images/leveleditor/rubber.png", true);

img_previous_level_bt = new Surface(datadir + "/images/leveleditor/previous-level.png", true);
img_next_level_bt = new Surface(datadir + "/images/leveleditor/next-level.png", true);
img_previous_sector_bt = new Surface(datadir + "/images/leveleditor/previous-sector.png", true);
img_next_sector_bt = new Surface(datadir + "/images/leveleditor/next-sector.png", true);
}

void LevelEditor::free_buttons_gfx()
{
delete img_foreground_bt;
delete img_interactive_bt;
delete img_background_bt;

delete img_save_level_bt;
delete img_test_level_bt;
delete img_setup_level_bt;

delete img_rubber_bt;

delete img_previous_level_bt;
delete img_next_level_bt;
delete img_previous_sector_bt;
delete img_next_sector_bt;
}

void LevelEditor::run(const std::string filename)
{
SoundManager::get()->halt_music();
Menu::set_current(0);

DrawingContext context;

if(!filename.empty())
  {
  level_nb = -1;
  load_level(filename);
  }
else
  Menu::set_current(main_menu);

mouse_cursor->set_state(MC_NORMAL);

done = false;
while(!done)
  {
  events();
  action();
  draw(context);
  }

if(level_changed)
  if(confirm_dialog(NULL, _("Level not saved. Wanna to?")))
    save_level();
}

void LevelEditor::events()
{
mouse_moved = false;

while(SDL_PollEvent(&event))
  {
  Menu* menu = Menu::current();
  if(menu)
    {
    menu->event(event);
    menu->action();
    if(menu == main_menu)
      {
      switch (main_menu->check())
        {
        case MN_ID_RETURN:
          Menu::set_current(0);
          break;
        case MN_ID_QUIT:
          done = true;
          break;
        }
      }
    else if(menu == create_subset_menu)
      {
      if(create_subset_menu->get_item_by_id(MN_ID_FILENAME_SUBSET).input[0] == '\0')
        create_subset_menu->get_item_by_id(MN_ID_CREATE_SUBSET).kind = MN_DEACTIVE;
      else if(create_subset_menu->check() == MN_ID_CREATE_SUBSET)
        {   // applying settings:
        LevelSubset::create(create_subset_menu->get_item_by_id(MN_ID_FILENAME_SUBSET).input);

        delete level_subset;
        level_subset = new LevelSubset();
        level_subset->load(create_subset_menu->get_item_by_id(MN_ID_FILENAME_SUBSET).input);

        level_subset->title = create_subset_menu->item[MN_ID_TITLE_SUBSET].input;
        level_subset->description = create_subset_menu->item[MN_ID_DESCRIPTION_SUBSET].input;

        load_level(1);

        create_subset_menu->get_item_by_id(MN_ID_FILENAME_SUBSET).change_input("");
        create_subset_menu->get_item_by_id(MN_ID_TITLE_SUBSET).change_input("");
        create_subset_menu->get_item_by_id(MN_ID_DESCRIPTION_SUBSET).change_input("");
        }
      }
    else if(menu == subset_menu)
      {
      int i = subset_menu->check();
      if(i >= 0)
        {
        std::set<std::string>::iterator it = level_subsets.begin();
        for(int t = 0; t < i; t++)
          it++;
        load_level_subset(*it);
        Menu::set_current(0);
        }
      }
    else if(menu == settings_menu)
      {
      if(settings_menu->check() == MN_ID_APPLY_SETTINGS)
        {   // applying settings:
        level_changed = true;

        level->name = settings_menu->get_item_by_id(MN_ID_NAME).input;
        level->author = settings_menu->get_item_by_id(MN_ID_AUTHOR).input;

        solids->resize(atoi(settings_menu->get_item_by_id(MN_ID_WIDTH).input.c_str()),
              atoi(settings_menu->get_item_by_id(MN_ID_HEIGHT).input.c_str()));
        foregrounds->resize(atoi(settings_menu->get_item_by_id(MN_ID_WIDTH).input.c_str()),
              atoi(settings_menu->get_item_by_id(MN_ID_HEIGHT).input.c_str()));
        backgrounds->resize(atoi(settings_menu->get_item_by_id(MN_ID_WIDTH).input.c_str()),
              atoi(settings_menu->get_item_by_id(MN_ID_HEIGHT).input.c_str()));

        Menu::set_current(0);
        }
      }
    }
  // check for events in buttons
  else if(tiles_board->event(event))
    {
    std::vector <int> vector;
    vector.push_back(tiles_board->selected_id());

    selection.clear();
    selection.push_back(vector);
    continue;
    }
  else if(tiles_layer->event(event))
    {
    cur_layer = tiles_layer->selected_id();
    continue;
    }
  else if(level_options->event(event))
    {
    switch(level_options->selected_id())
      {
      case BT_LEVEL_SAVE:
        save_level();
        break;
      case BT_LEVEL_TEST:
        test_level();
        break;
      case BT_LEVEL_SETUP:
        Menu::set_current(settings_menu);
        break;
      case BT_NEXT_LEVEL:
        if(level_nb+1 < level_subset->get_num_levels())
          load_level(level_nb + 1);
        else
          {
          char str[1024];
          sprintf(str,_("Level %d doesn't exist. Create it?"), level_nb + 1);
          if(confirm_dialog(NULL, str))
            {
            Level new_lev;
            level_subset->add_level("new_level.stl");
            new_lev.save(level_subset->get_level_filename(level_nb + 1));
            load_level(level_nb);
            }
          }
        break;
      case BT_PREVIOUS_LEVEL:
        if(level_nb-1 > 0)
          load_level(level_nb - 1);
        break;
      case BT_NEXT_SECTOR:
std::cerr << "next sector.\n";
std::cerr << "total sectors: " << level->get_total_sectors() << std::endl;
        load_sector(level->get_next_sector(sector));
        break;
      case BT_PREVIOUS_SECTOR:
std::cerr << "previous sector.\n";
        load_sector(level->get_previous_sector(sector));
        break;
      }
    level_options->set_unselected();
    continue;
    }
  else
    {
    switch(event.type)
      {
      case SDL_MOUSEMOTION:
        mouse_moved = true;
        if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(SDL_BUTTON_RIGHT))
          {  // movement like in strategy games
          scroll.x += -1 * event.motion.xrel;
          scroll.y += -1 * event.motion.yrel;
          }
        break;

      case SDL_MOUSEBUTTONDOWN:
        mouse_moved = true;
        if(event.button.button == SDL_BUTTON_LEFT)
          left_button = true;
        else if(event.button.button == SDL_BUTTON_MIDDLE)
          {
          middle_button = true;
          selection_ini = Vector(event.button.x, event.button.y);
          }
        break;

      case SDL_MOUSEBUTTONUP:
        mouse_moved = true;
        if(event.button.button == SDL_BUTTON_LEFT)
          left_button = false;
        else if(event.button.button == SDL_BUTTON_MIDDLE)
          {
          middle_button = false;
          selection_end = Vector(event.button.x, event.button.y);

          if(selection_end.x < selection_ini.x)
            {
            float t = selection_ini.x;
            selection_ini.x = selection_end.x;
            selection_end.x = t;
            }
          if(selection_end.y < selection_ini.y)
            {
            float t = selection_ini.y;
            selection_ini.y = selection_end.y;
            selection_end.y = t;
            }

          selection.clear();
          std::vector <int> vector;

          TileMap* tilemap = 0;
          if(cur_layer == LAYER_FOREGROUNDTILES)
            tilemap = foregrounds;
          else if(cur_layer == LAYER_TILES)
            tilemap = solids;
          else if(cur_layer == LAYER_BACKGROUNDTILES)
            tilemap = backgrounds;

          for(int x = 0; x < (int)((selection_end.x - selection_ini.x)*zoom / 32) + 1; x++)
            {
            vector.clear();
            for(int y = 0; y < (int)((selection_end.y - selection_ini.y)*zoom / 32) + 1; y++)
              {
              vector.push_back(tilemap->get_tile(x +
               (int)(((selection_ini.x+scroll.x)*zoom)/32),
               y + (int)(((selection_ini.y+scroll.y)*zoom)/32))->id);
              }
            selection.push_back(vector);
            }
          }
        break;

      case SDL_KEYDOWN:   // key pressed
        switch(event.key.keysym.sym)
          {
          case SDLK_ESCAPE:
            Menu::set_current(main_menu);
            break;
          /* scrolling related events: */
          case SDLK_HOME:
            scroll.x = 0;
            break;
          case SDLK_END:
            scroll.x = sector->solids->get_height()*32 - screen->w;
            break;
          case SDLK_LEFT:
            scroll.x -= 80;
            break;
          case SDLK_RIGHT:
            scroll.x += 80;
            break;
          case SDLK_UP:
            scroll.y -= 80;
            break;
          case SDLK_DOWN:
            scroll.y += 80;
            break;
          case SDLK_PAGEUP:
            scroll.x -= 450;
            break;
          case SDLK_PAGEDOWN:
            scroll.x += 450;
            break;
          case SDLK_PLUS:
          case SDLK_KP_PLUS:
            zoom += 0.10;
            break;
          case SDLK_MINUS:
          case SDLK_KP_MINUS:
            zoom -= 0.10;
            break;

          case SDLK_F1:
            show_help();
            break;
          case SDLK_F2:
            show_grid = !show_grid;
            break;
          default:
            break;
          }
        break;

      case SDL_QUIT:   // window closed
        done = true;
        break;

        default:
          break;
      }
    }
  }
}

void LevelEditor::action()
{
mouse_cursor->set_state(MC_NORMAL);
if(tiles_board->is_hover() || tiles_layer->is_hover() || level_options->is_hover())
  mouse_cursor->set_state(MC_LINK);

if(sector)
  {
  if(!frame_timer.check())
    {
    frame_timer.start(25);
    ++global_frame_counter;
    }

  // don't scroll before the start or after the level's end
  float width = sector->solids->get_width() * 32;
  float height = sector->solids->get_height() * 32;

  if(scroll.x < -screen->w/2)
    scroll.x = -screen->w/2;
  if(scroll.x > width - screen->w/2)
    scroll.x = width - screen->w/2;
  if(scroll.y < -screen->h/2)
    scroll.y = -screen->h/2;
  if(scroll.y > height - screen->h/2)
    scroll.y = height - screen->h/2;

  // set camera translation, since BadGuys like it
  sector->camera->set_scrolling((int)scroll.x, (int)scroll.y);

  if(left_button && mouse_moved)
    for(unsigned int x = 0; x < selection.size(); x++)
      for(unsigned int y = 0; y < selection[x].size(); y++)
        change((int)(scroll.x + event.button.x) + (x*32),
             (int)(scroll.y + event.button.y) + (y*32), selection[x][y], 
             cur_layer);
  }
}

#define FADING_TIME 600

void LevelEditor::draw(DrawingContext& context)
{
context.draw_text(white_text, _("Level Editor"), Vector(10, 5), LEFT_ALLIGN, LAYER_GUI);
mouse_cursor->draw(context);

// draw a filled background
context.draw_filled_rect(Vector(0,0), Vector(screen->w,screen->h), Color(60,60,60), LAYER_BACKGROUND0-1);

if(level_name_timer.check())
  {
  context.push_transform();
  if(level_name_timer.get_left() < FADING_TIME)
    context.set_alpha(level_name_timer.get_left() * 255 / FADING_TIME);

  context.draw_text(gold_text, level->name, Vector(screen->w/2, 30), CENTER_ALLIGN, LAYER_GUI);
  if(level_nb != -1)
    {
    char str[128];
    sprintf(str, "%i/%i", level_nb+1, level_subset->get_num_levels());
    context.draw_text(gold_text, str, Vector(screen->w/2, 50), CENTER_ALLIGN, LAYER_GUI);
    }

  context.pop_transform();
  }
if(sector)
  context.draw_text(white_small_text, _("F1 for help"), Vector(5, 510), LEFT_ALLIGN, LAYER_GUI-10);
else
  context.draw_text(white_small_text, _("Choose a level subset"), Vector(5, 510), LEFT_ALLIGN, LAYER_GUI-10);

Menu* menu = Menu::current();
if(menu)
  menu->draw(context);
else
  {
  tiles_board->draw(context);
  tiles_layer->draw(context);
  level_options->draw(context);
  }

// draw selection
if(sector)
  {
  if(!middle_button)
    {
    context.set_drawing_effect(SEMI_TRANSPARENT);

    if(selection.size())
      {
      if(selection[0][0] == 0 && selection.size() == 1)
          context.draw_surface(img_rubber_bt, Vector(event.button.x - 8,
          event.button.y - 8), LAYER_GUI-2);
      else if(selection[0][0] < 0)
        {
        int id = selection[0][0];

        if(id == OBJ_TRAMPOLINE)
          context.draw_surface(img_trampoline[0].get_frame(0), Vector(event.button.x - 8,
          event.button.y - 8), LAYER_GUI-2);
        else if(id == OBJ_FLYING_PLATFORM)
          context.draw_surface(img_flying_platform->get_frame(0), Vector(event.button.x - 8,
          event.button.y - 8), LAYER_GUI-2);
        else if(id == OBJ_DOOR)
          context.draw_surface(door->get_frame(0), Vector(event.button.x - 8,
          event.button.y - 8), LAYER_GUI-2);
        else
          {
          BadGuyKind kind = BadGuyKind((-id)-1);
          BadGuy badguy(kind, 0,0);
          badguy.activate(LEFT);
          Surface *img = badguy.get_image();

          context.draw_surface(img, Vector(event.button.x - 8,
          event.button.y - 8), LAYER_GUI-2);
          }
        }
      else
        {
        TileManager* tilemanager = TileManager::instance();
        for(unsigned int x = 0; x < selection.size(); x++)
          for(unsigned int y = 0; y < selection[x].size(); y++)
            tilemanager->draw_tile(context, selection[x][y],
                Vector(event.button.x + x*32 - 8, event.button.y + y*32 - 8),
                LAYER_GUI-2);
        }
      }
    context.set_drawing_effect(NONE_EFFECT);
    }
  else
    context.draw_filled_rect(Vector(std::min((int)selection_ini.x, (int)event.button.x)*zoom,
                   std::min((int)selection_ini.y, (int)event.button.y))*zoom,
                   Vector(abs(event.button.x - (int)selection_ini.x)*zoom,
                   abs(event.button.y - (int)selection_ini.y)*zoom),
                   Color(170,255,170,128), LAYER_GUI-2);

  if(show_grid)
    {
    for(int x = 0; x < screen->w / (32*zoom); x++)
      {
      int pos = (int)(x*32*zoom) - ((int)scroll.x % 32);
      context.draw_filled_rect(Vector (pos, 0), Vector(1, screen->h),
                Color(225, 225, 225), LAYER_GUI-50);
      }
    for(int y = 0; y < screen->h / (32*zoom); y++)
      {
      int pos = (int)(y*32*zoom) - ((int)scroll.y % 32);
      context.draw_filled_rect(Vector (0, pos), Vector(screen->w, 1),
                Color(225, 225, 225), LAYER_GUI-50);
      }
    }

  context.push_transform();
  context.set_translation(scroll);
  context.set_zooming(zoom);

  for(Sector::GameObjects::iterator i = sector->gameobjects.begin(); i != sector->gameobjects.end(); ++i)
    {
    TileMap* tilemap = dynamic_cast<TileMap*> (*i);
    if(tilemap)
      {  // draw the non-selected tiles semi-transparently
      context.push_transform();

      if(tilemap->get_layer() != cur_layer)
        context.set_drawing_effect(SEMI_TRANSPARENT);
      (*i)->draw(context);

      context.pop_transform();
      }
    Background* background = dynamic_cast<Background*> (*i);
    if(background)
      {  // don't resize background
      context.push_transform();
      context.set_translation(scroll);
      context.set_zooming(1.0);
      (*i)->draw(context);
      context.pop_transform();
      }
    else
      (*i)->draw(context);
    }

  context.pop_transform();
  }
else
  context.draw_filled_rect(Vector(0,0), Vector(screen->w,screen->h),Color(0,0,0), LAYER_BACKGROUND0);

context.do_drawing();
}

void LevelEditor::load_level_subset(std::string filename)
{
delete level_subset;
level_subset = new LevelSubset();
level_subset->load(filename.c_str());
load_level(1);
}

void LevelEditor::load_level(std::string filename)
{
if(level_changed)
  {
  if(confirm_dialog(NULL, _("Level not saved. Wanna to?")))
    save_level();
  else
    return;
  }

level_filename = filename;

delete level;
level = new Level();
level->load(filename);

load_sector("main");
level_name_timer.start(3000);
scroll.x = scroll.y = 0;
level_changed = false;

settings_menu->get_item_by_id(MN_ID_NAME).change_input(level->name.c_str());
settings_menu->get_item_by_id(MN_ID_AUTHOR).change_input(level->author.c_str());
}

void LevelEditor::load_level(int nb)
{
if(level_changed)
  {
  if(confirm_dialog(NULL, _("Level not saved. Wanna to?")))
    save_level();
  else
    return;
  }

level_nb = nb;
level_filename = level_subset->get_level_filename(level_nb);

load_level(level_filename);
}

void LevelEditor::load_sector(std::string name)
{
sector_name = name;
sector = level->get_sector(sector_name);
if(!sector)
  Termination::abort("Level has no " + sector_name + " sector.", "");

load_sector(sector);
}

void LevelEditor::load_sector(Sector* sector_)
{
if(sector == NULL)
  {
  if(confirm_dialog(NULL, _("No more sectors exist. Create another?")))
    {
    Sector* nsector = new Sector();
    level->add_sector(nsector);
    sector = nsector;
    }
  return;
  }

sector = sector_;

/* Load sector stuff */

sector->update_game_objects();

foregrounds = solids = backgrounds = 0;
/* Point foregrounds, backgrounds, solids to its layer */
for(Sector::GameObjects::iterator i = sector->gameobjects.begin(); i != sector->gameobjects.end(); i++)
  {
  BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
  if(badguy)
    badguy->activate(LEFT);

  TileMap* tilemap = dynamic_cast<TileMap*> (*i);
  if(tilemap)
    {
    if(tilemap->get_layer() == LAYER_FOREGROUNDTILES)
      foregrounds = tilemap;
    else if(tilemap->get_layer() == LAYER_TILES)
      solids = tilemap;
    else if(tilemap->get_layer() == LAYER_BACKGROUNDTILES)
      backgrounds = tilemap;
    }
  }

if(!foregrounds)
  {
  TileMap* tilemap = new TileMap(LAYER_FOREGROUNDTILES, false, solids->get_width(), solids->get_height());
  sector->add_object(tilemap);
  sector->update_game_objects();
  }
if(!backgrounds)
  {
  TileMap* tilemap = new TileMap(LAYER_BACKGROUNDTILES, false, solids->get_width(), solids->get_height());
  sector->add_object(tilemap);
  sector->update_game_objects();
  }

char str[64];
sprintf(str, "%i", solids->get_width());
settings_menu->get_item_by_id(MN_ID_WIDTH).change_input(str);
sprintf(str, "%i", solids->get_height());
settings_menu->get_item_by_id(MN_ID_HEIGHT).change_input(str);
}

void LevelEditor::save_level()
{
level->save(level_filename);
level_changed = false;
}

void LevelEditor::test_level()
{
if(level_changed)
  {
  if(confirm_dialog(NULL, _("Level not saved. Wanna to?")))
    save_level();
  else
    return;
  }

GameSession session(level_filename, ST_GL_TEST);
session.run();
//  player_status.reset();
sound_manager->halt_music();
}

void LevelEditor::change(int x, int y, int newtile, int layer)
{  // find the tilemap of the current layer, and then change the tile
if(x < 0 || (unsigned int)x > sector->solids->get_width()*32 ||
   y < 0 || (unsigned int)y > sector->solids->get_height()*32)
  return;

level_changed = true;

if(zoom != 1)
  {  // no need to do this for normal view (no zoom)
  x = (int)(x * (zoom*32) / 32);
  y = (int)(y * (zoom*32) / 32);
  }

if(newtile < 0)  // add object
  {
  // remove an active tile or object that might be there
  change(x, y, 0, LAYER_TILES);

  if(newtile == OBJ_TRAMPOLINE)
    sector->add_object(new Trampoline(x, y));
  else if(newtile == OBJ_FLYING_PLATFORM)
    sector->add_object(new FlyingPlatform(x, y));
  else if(newtile == OBJ_DOOR)
    sector->add_object(new Door(x, y));
  else
    sector->add_bad_guy(x, y, BadGuyKind((-newtile)-1), true);

  sector->update_game_objects();
  }
else if(cur_layer == LAYER_FOREGROUNDTILES)
  foregrounds->change(x/32, y/32, newtile);
else if(cur_layer == LAYER_TILES)
  {
  // remove a bad guy if it's there
  // we /32 in order to round numbers
  for(Sector::GameObjects::iterator i = sector->gameobjects.begin(); i < sector->gameobjects.end(); i++)
    {
    BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
    if(badguy)
      if((int)badguy->base.x/32 == x/32 && (int)badguy->base.y/32 == y/32)
        sector->gameobjects.erase(i);
    Trampoline* trampoline = dynamic_cast<Trampoline*> (*i);
    if(trampoline)
    {
      if((int)trampoline->base.x/32 == x/32 && (int)trampoline->base.y/32 == y/32)
        sector->gameobjects.erase(i);
        }
    FlyingPlatform* flying_platform = dynamic_cast<FlyingPlatform*> (*i);
    if(flying_platform)
      if((int)flying_platform->base.x/32 == x/32 && (int)flying_platform->base.y/32 == y/32)
        sector->gameobjects.erase(i);
    Door* door = dynamic_cast<Door*> (*i);
    if(door)
      if((int)door->get_area().x/32 == x/32 && (int)door->get_area().y/32 == y/32)
        sector->gameobjects.erase(i);
    }
  sector->update_game_objects();
  solids->change(x/32, y/32, newtile);
  }
else if(cur_layer == LAYER_BACKGROUNDTILES)
  backgrounds->change(x/32, y/32, newtile);
}

void LevelEditor::show_help()
{
DrawingContext context;

bool show_grid_t = show_grid;
show_grid = false;
mouse_cursor->set_state(MC_HIDE);


char str[1024];
char *text1[] = {
         _("This is the built-in level editor. It's aim is to be intuitive\n"
         "and simple to use, so it should be pretty straight forward.\n"
         "\n"
         "To open a level, first you'll have to select a level subset from\n"
         "the menu (or create your own).\n"
         "A level subset is basically a collection of levels.\n"
         "They can then be played from the Contrib menu.\n"
         "\n"
         "To access the menu from the level editor, just press Esc.\n"
         "\n"
         "You are currently looking to the level, to scroll it, just\n"
         "press the right mouse button and drag the mouse. It will move like\n"
         "a strategy game.\n"
         "You can also use the arrow keys and Page Up/Down.\n"
         "\n"
         "'+' and '-' keys can be used to zoom in/out the level.\n"
         "\n"
         "You probably already noticed those floating group of buttons.\n"
         "Each one serves a different purpose. To select a certain button\n"
         "just press the Left mouse button on it. A few buttons have key\n"
         "shortcuts, you can know it by pressing the Right mouse button on\n"
         "it. That will also show what that button does.\n"
         "Group of buttons can also be move around by just dragging them,\n"
         "while pressing the Left mouse button.\n"
         "\n"
         "Let's learn a bit of what each group of buttons do, shall we?\n"
         "\n"
         "To starting putting tiles and objects around use the bigger gropup\n"
         "of buttons. Each button is a different tile. To put it on the level,\n"
         "just press it and then left click in the level.\n"
         "You can also copy tiles from the level by using the middle mouse button.\n"
         "Use the mouse wheel to scroll that group of buttons. You will find\n"
         "enemies and game objects in the bottom.\n")
                };

char *text2[] = {
         _("The Foreground/Interactive/Background buttons may be used to\n"
         "see and edit the respective layer. Level's have three tiles layers:\n"
         "Foreground - tiles are drawn in top of everything and have no contact\n"
         "with the player.\n"
         "Interactive - these are the tiles that have contact with the player.\n"
         "Background - tiles are drawn in bottom of everything and have no contact\n"
         "with the player.\n"
         "The unselected layers will be drawn semi-transparently.\n"
         "\n"
         "At last, but not least, the group of buttons that's left serves\n"
         "to do related actions with the level.\n"
         "From left to right:\n"
         "Mini arrows - can be used to choose other sectors.\n"
         "Sectors are mini-levels, so to speak, that can be accessed using a door.\n"
         "Big arrows - choose other level in the same level subset.\n"
         "Diskette - save the level\n"
         "Tux - test the level\n"
         "Tools - set a few settings for the level, incluiding resizing it.\n"
         "\n"
         "We have reached the end of this Howto.\n"
         "\n"
         "Don't forget to send us a few cool levels. :)\n"
         "\n"
         "Enjoy,\n"
         "  SuperTux development team\n"
         "\n"
         "ps: If you are looking for something more powerfull, you can give it a\n"
         "try to FlexLay. FlexLay is a level editor that supports several games,\n"
         "including SuperTux. It is an independent project.\n"
         "Webpage: http://pingus.seul.org/~grumbel/flexlay/")
                };

char **text[] = { text1, text2 };


bool done;
for(unsigned int i = 0; i < sizeof(text) / sizeof(text[0]); i++)
  {
  draw(context);

  context.draw_text(blue_text, _("- Level Editor's Help -"), Vector(screen->w/2, 60), CENTER_ALLIGN, LAYER_GUI);

  context.draw_text(white_small_text, *text[i], Vector(20, 120), LEFT_ALLIGN, LAYER_GUI);

  sprintf(str,_("Press any key to continue - Page %d/%d"), i+1, sizeof(text) / sizeof(text[0]));
  context.draw_text(gold_text, str, Vector(screen->w/2, screen->h-60), CENTER_ALLIGN, LAYER_GUI);

  context.do_drawing();

  done = false;

  while(!done)
    {
    done = wait_for_event(event);
    SDL_Delay(50);
    }
  }

show_grid = show_grid_t;
mouse_cursor->set_state(MC_NORMAL);
}
