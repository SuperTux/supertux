//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <memory>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "app/globals.h"
#include "sector.h"
#include "utils/lispreader.h"
#include "badguy.h"
#include "special.h"
#include "gameobjs.h"
#include "camera.h"
#include "background.h"
#include "particlesystem.h"
#include "tile.h"
#include "tilemap.h"
#include "audio/sound_manager.h"
#include "gameloop.h"
#include "resources.h"
#include "interactive_object.h"
#include "door.h"

Sector* Sector::_current = 0;

Sector::Sector()
  : gravity(10), player(0), solids(0), background(0), camera(0),
    currentmusic(LEVEL_MUSIC)
{
  song_title = "Mortimers_chipdisko.mod";
  player = new Player();
  add_object(player);
}

Sector::~Sector()
{
  for(GameObjects::iterator i = gameobjects.begin(); i != gameobjects.end();
      ++i)
    delete *i;

  for(SpawnPoints::iterator i = spawnpoints.begin(); i != spawnpoints.end();
      ++i)
    delete *i;
    
  if(_current == this)
    _current = 0;
}

void
Sector::parse(LispReader& lispreader)
{
  _current = this;
  
  for(lisp_object_t* cur = lispreader.get_lisp(); !lisp_nil_p(cur);
      cur = lisp_cdr(cur)) {
    std::string token = lisp_symbol(lisp_car(lisp_car(cur)));
    // FIXME: doesn't handle empty data
    lisp_object_t* data = lisp_car(lisp_cdr(lisp_car(cur)));
    LispReader reader(lisp_cdr(lisp_car(cur)));

    if(token == "name") {
      name = lisp_string(data);
    } else if(token == "gravity") {
      gravity = lisp_real(data);
    } else if(token == "music") {
      song_title = lisp_string(data);
      load_music();
    } else if(token == "camera") {
      if(camera) {
        std::cerr << "Warning: More than 1 camera defined in sector.\n";
        continue;
      }
      camera = new Camera(this);
      camera->read(reader);
      add_object(camera);
    } else if(token == "background") {
      background = new Background(reader);
      add_object(background);
    } else if(token == "playerspawn") {
      SpawnPoint* sp = new SpawnPoint;
      reader.read_string("name", sp->name);
      reader.read_float("x", sp->pos.x);
      reader.read_float("y", sp->pos.y);
      spawnpoints.push_back(sp);
    } else if(token == "tilemap") {
      TileMap* tilemap = new TileMap(reader);
      add_object(tilemap);

      if(tilemap->is_solid()) {
        if(solids) {
          std::cerr << "Warning multiple solid tilemaps in sector.\n";
          continue;
        }
        solids = tilemap;
      }
    } else if(badguykind_from_string(token) != BAD_INVALID) {
      add_object(new BadGuy(badguykind_from_string(token), reader));
    } else if(token == "trampoline") {
      add_object(new Trampoline(reader));
    } else if(token == "flying-platform") {
      add_object(new FlyingPlatform(reader));
    } else if(token == "particles-snow") {
      SnowParticleSystem* partsys = new SnowParticleSystem();
      partsys->parse(reader);
      add_object(partsys);
    } else if(token == "particles-clouds") {
      CloudParticleSystem* partsys = new CloudParticleSystem();
      partsys->parse(reader);
      add_object(partsys);
    } else if(token == "door") {
      add_object(new Door(reader));
    } else {
      std::cerr << "Unknown object type '" << token << "'.\n";
    }
  }

  if(!camera) {
    std::cerr << "sector '" << name << "' does not contain a camera.\n";
    camera = new Camera(this);
    add_object(camera);
  }
  if(!solids)
    throw std::runtime_error("sector does not contain a solid tile layer.");
}

void
Sector::parse_old_format(LispReader& reader)
{
  _current = this;
  
  name = "main";
  reader.read_float("gravity", gravity);

  std::string backgroundimage;
  reader.read_string("background", backgroundimage);
  float bgspeed = .5;
  reader.read_float("bkgd_speed", bgspeed);

  Color bkgd_top, bkgd_bottom;
  int r = 0, g = 0, b = 128;
  reader.read_int("bkgd_red_top", r);
  reader.read_int("bkgd_green_top",  g);
  reader.read_int("bkgd_blue_top",  b);
  bkgd_top.red = r;
  bkgd_top.green = g;
  bkgd_top.blue = b;
  
  reader.read_int("bkgd_red_bottom",  r);
  reader.read_int("bkgd_green_bottom", g);
  reader.read_int("bkgd_blue_bottom", b);
  bkgd_bottom.red = r;
  bkgd_bottom.green = g;
  bkgd_bottom.blue = b;
  
  if(backgroundimage != "") {
    background = new Background;
    background->set_image(backgroundimage, bgspeed);
    add_object(background);
  } else {
    background = new Background;
    background->set_gradient(bkgd_top, bkgd_bottom);
    add_object(background);
  }

  std::string particlesystem;
  reader.read_string("particle_system", particlesystem);
  if(particlesystem == "clouds")
    add_object(new CloudParticleSystem());
  else if(particlesystem == "snow")
    add_object(new SnowParticleSystem());

  Vector startpos(100, 170);
  reader.read_float("start_pos_x", startpos.x);
  reader.read_float("start_pos_y", startpos.y);

  SpawnPoint* spawn = new SpawnPoint;
  spawn->pos = startpos;
  spawn->name = "main";
  spawnpoints.push_back(spawn);

  song_title = "Mortimers_chipdisko.mod";
  reader.read_string("music", song_title);
  load_music();

  int width, height = 15;
  reader.read_int("width", width);
  reader.read_int("height", height);
  
  std::vector<unsigned int> tiles;
  if(reader.read_int_vector("interactive-tm", tiles)
      || reader.read_int_vector("tilemap", tiles)) {
    TileMap* tilemap = new TileMap();
    tilemap->set(width, height, tiles, LAYER_TILES, true);
    solids = tilemap;
    add_object(tilemap);
  }

  if(reader.read_int_vector("background-tm", tiles)) {
    TileMap* tilemap = new TileMap();
    tilemap->set(width, height, tiles, LAYER_BACKGROUNDTILES, false);
    add_object(tilemap);
  }

  if(reader.read_int_vector("foreground-tm", tiles)) {
    TileMap* tilemap = new TileMap();
    tilemap->set(width, height, tiles, LAYER_FOREGROUNDTILES, false);
    add_object(tilemap);
  }

  // TODO read resetpoints

  // read objects
  {
    lisp_object_t* cur = 0;
    if(reader.read_lisp("objects", cur)) {
      while(!lisp_nil_p(cur)) {
        lisp_object_t* data = lisp_car(cur);
        std::string object_type = lisp_symbol(lisp_car(data));
                                                                                
        LispReader reader(lisp_cdr(data));
                                                                                
        if(object_type == "trampoline") {
          add_object(new Trampoline(reader));
        }
        else if(object_type == "flying-platform") {
          add_object(new FlyingPlatform(reader));
        }
        else {
          BadGuyKind kind = badguykind_from_string(object_type);
          add_object(new BadGuy(kind, reader));
        }
                                                                                
        cur = lisp_cdr(cur);
      }
    }
  }

  // add a camera
  camera = new Camera(this);
  add_object(camera);
}

void
Sector::write(LispWriter& writer)
{
  writer.write_string("name", name);
  writer.write_float("gravity", gravity);
  writer.write_string("music", song_title);

  // write spawnpoints
  for(SpawnPoints::iterator i = spawnpoints.begin(); i != spawnpoints.end();
      ++i) {
    SpawnPoint* spawn = *i;
    writer.start_list("playerspawn");
    writer.write_string("name", spawn->name);
    writer.write_float("x", spawn->pos.x);
    writer.write_float("y", spawn->pos.y);
    writer.end_list("playerspawn");
  }

  // write objects
  for(GameObjects::iterator i = gameobjects.begin();
      i != gameobjects.end(); ++i) {
    Serializable* serializable = dynamic_cast<Serializable*> (*i);
    if(serializable)
      serializable->write(writer);
  }
}

void
Sector::do_vertical_flip()
{
  for(GameObjects::iterator i = gameobjects_new.begin(); i != gameobjects_new.end(); ++i)
    {
    TileMap* tilemap = dynamic_cast<TileMap*> (*i);
    if(tilemap)
      {
      tilemap->do_vertical_flip();
      }

    BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
    if(badguy)
      badguy->start_position.y = solids->get_height()*32 - badguy->start_position.y - 32;
    Trampoline* trampoline = dynamic_cast<Trampoline*> (*i);
    if(trampoline)
      trampoline->base.y = solids->get_height()*32 - trampoline->base.y - 32;
    FlyingPlatform* flying_platform = dynamic_cast<FlyingPlatform*> (*i);
    if(flying_platform)
      flying_platform->base.y = solids->get_height()*32 - flying_platform->base.y - 32;
    Door* door = dynamic_cast<Door*> (*i);
    if(door)
      door->set_area(door->get_area().x, solids->get_height()*32 - door->get_area().y - 32);
    }

  for(SpawnPoints::iterator i = spawnpoints.begin(); i != spawnpoints.end();
      ++i) {
    SpawnPoint* spawn = *i;
    spawn->pos.y = solids->get_height()*32 - spawn->pos.y - 32;
  }
}

void
Sector::add_object(GameObject* object)
{
  gameobjects_new.push_back(object);
}

void
Sector::activate(const std::string& spawnpoint)
{
  _current = this;

  // Apply bonuses from former levels
  switch (player_status.bonus)
    {
    case PlayerStatus::NO_BONUS:
      break;
                                                                                
    case PlayerStatus::FLOWER_BONUS:
      player->got_power = Player::FIRE_POWER;  // FIXME: add ice power to here
      // fall through
                                                                                
    case PlayerStatus::GROWUP_BONUS:
      player->grow(false);
      break;
    }

  SpawnPoint* sp = 0;
  for(SpawnPoints::iterator i = spawnpoints.begin(); i != spawnpoints.end();
      ++i) {
    if((*i)->name == spawnpoint) {
      sp = *i;
      break;
    }
  }
  if(!sp) {
    std::cerr << "Spawnpoint '" << spawnpoint << "' not found.\n";
  } else {
    player->move(sp->pos);
  }

  camera->reset(Vector(player->base.x, player->base.y));
}

void
Sector::action(float elapsed_time)
{
  player->check_bounds(camera);
                                                                                
  /* update objects (don't use iterators here, because the list might change
   * during the iteration)
   */
  for(size_t i = 0; i < gameobjects.size(); ++i)
    if(gameobjects[i]->is_valid())
      gameobjects[i]->action(elapsed_time);
                                                                                
  /* Handle all possible collisions. */
  collision_handler();
                                                                                
  update_game_objects();
}

void
Sector::update_game_objects()
{
  /** cleanup marked objects */
  for(std::vector<GameObject*>::iterator i = gameobjects.begin();
      i != gameobjects.end(); /* nothing */) {
    if((*i)->is_valid() == false) {
      BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
      if(badguy) {
        badguys.erase(std::remove(badguys.begin(), badguys.end(), badguy),
            badguys.end());
      }
      Bullet* bullet = dynamic_cast<Bullet*> (*i);
      if(bullet) {
        bullets.erase(
            std::remove(bullets.begin(), bullets.end(), bullet),
            bullets.end());
      }
      InteractiveObject* interactive_object =
          dynamic_cast<InteractiveObject*> (*i);
      if(interactive_object) {
        interactive_objects.erase(
            std::remove(interactive_objects.begin(), interactive_objects.end(),
                interactive_object), interactive_objects.end());
      }
      Upgrade* upgrade = dynamic_cast<Upgrade*> (*i);
      if(upgrade) {
        upgrades.erase(
            std::remove(upgrades.begin(), upgrades.end(), upgrade),
            upgrades.end());
      }
      Trampoline* trampoline = dynamic_cast<Trampoline*> (*i);
      if(trampoline) {
        trampolines.erase(
            std::remove(trampolines.begin(), trampolines.end(), trampoline),
            trampolines.end());
      }
      FlyingPlatform* flying_platform= dynamic_cast<FlyingPlatform*> (*i);
      if(flying_platform) {
        flying_platforms.erase(
            std::remove(flying_platforms.begin(), flying_platforms.end(), flying_platform),
            flying_platforms.end());
      }
      SmokeCloud* smoke_cloud = dynamic_cast<SmokeCloud*> (*i);
      if(smoke_cloud) {
        smoke_clouds.erase(
            std::remove(smoke_clouds.begin(), smoke_clouds.end(), smoke_cloud),
            smoke_clouds.end());
      }
                                                                                
      delete *i;
      i = gameobjects.erase(i);
    } else {
      ++i;
    }
  }

  /* add newly created objects */
  for(std::vector<GameObject*>::iterator i = gameobjects_new.begin();
      i != gameobjects_new.end(); ++i)
  {
          BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
          if(badguy)
            badguys.push_back(badguy);
          Bullet* bullet = dynamic_cast<Bullet*> (*i);
          if(bullet)
            bullets.push_back(bullet);
          Upgrade* upgrade = dynamic_cast<Upgrade*> (*i);
          if(upgrade)
            upgrades.push_back(upgrade);
          Trampoline* trampoline = dynamic_cast<Trampoline*> (*i);
          if(trampoline)
            trampolines.push_back(trampoline);
          FlyingPlatform* flying_platform = dynamic_cast<FlyingPlatform*> (*i);
          if(flying_platform)
            flying_platforms.push_back(flying_platform);
          InteractiveObject* interactive_object 
              = dynamic_cast<InteractiveObject*> (*i);
          if(interactive_object)
            interactive_objects.push_back(interactive_object);
          SmokeCloud* smoke_cloud = dynamic_cast<SmokeCloud*> (*i);
          if(smoke_cloud)
            smoke_clouds.push_back(smoke_cloud);


          gameobjects.push_back(*i);
  }
  gameobjects_new.clear();
}

void
Sector::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(camera->get_translation());
  
  for(GameObjects::iterator i = gameobjects.begin();
      i != gameobjects.end(); ++i) {
    if( (*i)->is_valid() )
      (*i)->draw(context);
  }

  context.pop_transform();
}

void
Sector::collision_handler()
{
  // CO_BULLET & CO_BADGUY check
  for(unsigned int i = 0; i < bullets.size(); ++i)
    {
      for (BadGuys::iterator j = badguys.begin(); j != badguys.end(); ++j)
        {
          if((*j)->dying != DYING_NOT)
            continue;
                                                                                
          if(rectcollision(bullets[i]->base, (*j)->base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              (*j)->collision(bullets[i], CO_BULLET, COLLISION_NORMAL);
              bullets[i]->collision(CO_BADGUY);
              break; // bullet is invalid now, so break
            }
        }
    }
                                                                                
  /* CO_BADGUY & CO_BADGUY check */
  for (BadGuys::iterator i = badguys.begin(); i != badguys.end(); ++i)
    {
      if((*i)->dying != DYING_NOT)
        continue;
                                                                                
      BadGuys::iterator j = i;
      ++j;
      for (; j != badguys.end(); ++j)
        {
          if(j == i || (*j)->dying != DYING_NOT)
            continue;
                                                                                
          if(rectcollision((*i)->base, (*j)->base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              (*j)->collision(*i, CO_BADGUY);
              (*i)->collision(*j, CO_BADGUY);
            }
        }
    }
  if(player->dying != DYING_NOT) return;
                                                                                
  // CO_BADGUY & CO_PLAYER check
  for (BadGuys::iterator i = badguys.begin(); i != badguys.end(); ++i)
    {
      if((*i)->dying != DYING_NOT)
        continue;
                                                                                
      if(rectcollision_offset((*i)->base, player->base, 0, 0))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          if (player->previous_base.y < player->base.y &&
              player->previous_base.y + player->previous_base.height
              < (*i)->base.y + (*i)->base.height/2
              && !player->invincible_timer.started())
            {
              (*i)->collision(player, CO_PLAYER, COLLISION_SQUISH);
            }
          else
            {
              player->collision(*i, CO_BADGUY);
              (*i)->collision(player, CO_PLAYER, COLLISION_NORMAL);
            }
        }
    }
                                                                                
  // CO_UPGRADE & CO_PLAYER check
  for(unsigned int i = 0; i < upgrades.size(); ++i)
    {
      if(rectcollision(upgrades[i]->base, player->base))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          upgrades[i]->collision(player, CO_PLAYER, COLLISION_NORMAL);
        }
    }
                                                                                
  // CO_TRAMPOLINE & (CO_PLAYER or CO_BADGUY)
  for (Trampolines::iterator i = trampolines.begin(); i != trampolines.end(); ++i)
  {
    if (rectcollision((*i)->base, player->base))
    {
      if (player->previous_base.y < player->base.y &&
          player->previous_base.y + player->previous_base.height
          < (*i)->base.y + (*i)->base.height/2)
      {
        (*i)->collision(player, CO_PLAYER, COLLISION_SQUISH);
      }
      else if (player->previous_base.y <= player->base.y)
      {
        player->collision(*i, CO_TRAMPOLINE);
        (*i)->collision(player, CO_PLAYER, COLLISION_NORMAL);
      }
    }
  }
                                                                                
  // CO_FLYING_PLATFORM & (CO_PLAYER or CO_BADGUY)
  for (FlyingPlatforms::iterator i = flying_platforms.begin(); i != flying_platforms.end(); ++i)
  {
    if (rectcollision((*i)->base, player->base))
    {
      if (player->previous_base.y < player->base.y &&
          player->previous_base.y + player->previous_base.height
          < (*i)->base.y + (*i)->base.height/2)
      {
        (*i)->collision(player, CO_PLAYER, COLLISION_SQUISH);
        player->collision(*i, CO_FLYING_PLATFORM);
      }
/*      else if (player->previous_base.y <= player->base.y)
      {
      }*/
    }
  }
}

void
Sector::add_score(const Vector& pos, int s)
{
  player_status.score += s;
                                                                                
  add_object(new FloatingScore(pos, s));
}
                                                                                
void
Sector::add_bouncy_distro(const Vector& pos)
{
  add_object(new BouncyDistro(pos));
}
                                                                                
void
Sector::add_broken_brick(const Vector& pos, Tile* tile)
{
  add_broken_brick_piece(pos, Vector(-1, -4), tile);
  add_broken_brick_piece(pos + Vector(0, 16), Vector(-1.5, -3), tile);
                                                                                
  add_broken_brick_piece(pos + Vector(16, 0), Vector(1, -4), tile);
  add_broken_brick_piece(pos + Vector(16, 16), Vector(1.5, -3), tile);
}
                                                                                
void
Sector::add_broken_brick_piece(const Vector& pos, const Vector& movement,
    Tile* tile)
{
  add_object(new BrokenBrick(tile, pos, movement));
}
                                                                                
void
Sector::add_bouncy_brick(const Vector& pos)
{
  add_object(new BouncyBrick(pos));
}

BadGuy*
Sector::add_bad_guy(float x, float y, BadGuyKind kind)
{
  BadGuy* badguy = new BadGuy(kind, x, y);
  add_object(badguy);
  return badguy;
}
                                                                                
void
Sector::add_upgrade(const Vector& pos, Direction dir, UpgradeKind kind)
{
  add_object(new Upgrade(pos, dir, kind));
}
                                                                                
bool
Sector::add_bullet(const Vector& pos, float xm, Direction dir)
{
  if(player->got_power == Player::FIRE_POWER)
    {
    if(bullets.size() > MAX_FIRE_BULLETS-1)
      return false;
    }
  else if(player->got_power == Player::ICE_POWER)
    {
    if(bullets.size() > MAX_ICE_BULLETS-1)
      return false;
    }
                                                                                
  Bullet* new_bullet = 0;
  if(player->got_power == Player::FIRE_POWER)
    new_bullet = new Bullet(pos, xm, dir, FIRE_BULLET);
  else if(player->got_power == Player::ICE_POWER)
    new_bullet = new Bullet(pos, xm, dir, ICE_BULLET);
  else
    throw std::runtime_error("wrong bullet type.");
  add_object(new_bullet);
                                                                                
  SoundManager::get()->play_sound(IDToSound(SND_SHOOT));
                                                                                
  return true;
}

bool
Sector::add_smoke_cloud(const Vector& pos)
{
  add_object(new SmokeCloud(pos));
  return true;
}

/* Break a brick: */
bool
Sector::trybreakbrick(const Vector& pos, bool small)
{
  Tile* tile = solids->get_tile_at(pos);
  if (!tile)
  {
    char errmsg[64];
    sprintf(errmsg, "Invalid tile at %i,%i", (int)((pos.x+1)/32*32), (int)((pos.y+1)/32*32));
    throw SuperTuxException(errmsg, __FILE__, __LINE__);
  }

  if (tile->attributes & Tile::BRICK)
    {
      if (tile->data > 0)
        {
          /* Get a distro from it: */
          add_bouncy_distro(
              Vector(((int)(pos.x + 1) / 32) * 32, (int)(pos.y / 32) * 32));
                                                                                
          // TODO: don't handle this in a global way but per-tile...
          if (!counting_distros)
            {
              counting_distros = true;
              distro_counter = 5;
            }
          else
            {
              distro_counter--;
            }
                                                                                
          if (distro_counter <= 0)
            {
              counting_distros = false;
              solids->change_at(pos, tile->next_tile);
            }
                                                                                
          SoundManager::get()->play_sound(IDToSound(SND_DISTRO));
          player_status.score = player_status.score + SCORE_DISTRO;
          player_status.distros++;
          return true;
        }
      else if (!small)
        {
          /* Get rid of it: */
          solids->change_at(pos, tile->next_tile);
                                                                                
          /* Replace it with broken bits: */
          add_broken_brick(Vector(
                                 ((int)(pos.x + 1) / 32) * 32,
                                 (int)(pos.y / 32) * 32), tile);
                                                                                
          /* Get some score: */
          SoundManager::get()->play_sound(IDToSound(SND_BRICK));
          player_status.score = player_status.score + SCORE_BRICK;
                                                                                
          return true;
        }
    }
                                                                                
  return false;
}
                                                                                
/* Empty a box: */
void
Sector::tryemptybox(const Vector& pos, Direction col_side)
{
  Tile* tile = solids->get_tile_at(pos);
  if (!tile)
  {
    char errmsg[64];
    sprintf(errmsg, "Invalid tile at %i,%i", (int)((pos.x+1)/32*32), (int)((pos.y+1)/32*32));
    throw SuperTuxException(errmsg, __FILE__, __LINE__);
  }


  if (!(tile->attributes & Tile::FULLBOX))
    return;
                                                                                
  // according to the collision side, set the upgrade direction
  if(col_side == LEFT)
    col_side = RIGHT;
  else
    col_side = LEFT;
                                                                                
  int posx = ((int)(pos.x+1) / 32) * 32;
  int posy = (int)(pos.y/32) * 32 - 32;
  switch(tile->data)
    {
    case 1: // Box with a distro!
      add_bouncy_distro(Vector(posx, posy));
      SoundManager::get()->play_sound(IDToSound(SND_DISTRO));
      player_status.score = player_status.score + SCORE_DISTRO;
      player_status.distros++;
      break;
                                                                                
    case 2: // Add a fire flower upgrade!
      if (player->size == SMALL)     /* Tux is small, add mints! */
        add_upgrade(Vector(posx, posy), col_side, UPGRADE_GROWUP);
      else     /* Tux is big, add a fireflower: */
        add_upgrade(Vector(posx, posy), col_side, UPGRADE_FIREFLOWER);
      SoundManager::get()->play_sound(IDToSound(SND_UPGRADE));
      break;
                                                                                
    case 5: // Add an ice flower upgrade!
      if (player->size == SMALL)     /* Tux is small, add mints! */
        add_upgrade(Vector(posx, posy), col_side, UPGRADE_GROWUP);
      else     /* Tux is big, add an iceflower: */
        add_upgrade(Vector(posx, posy), col_side, UPGRADE_ICEFLOWER);
      SoundManager::get()->play_sound(IDToSound(SND_UPGRADE));
      break;
                                                                                
    case 3: // Add a golden herring
      add_upgrade(Vector(posx, posy), col_side, UPGRADE_HERRING);
      break;
                                                                                
    case 4: // Add a 1up extra
      add_upgrade(Vector(posx, posy), col_side, UPGRADE_1UP);
      break;
    default:
      break;
    }
                                                                                
  /* Empty the box: */
  solids->change_at(pos, tile->next_tile);
}
                                                                                
/* Try to grab a distro: */
void
Sector::trygrabdistro(const Vector& pos, int bounciness)
{
  Tile* tile = solids->get_tile_at(pos);
  if (!tile)
  {
    /*char errmsg[64];
    sprintf(errmsg, "Invalid tile at %i,%i", (int)((pos.x+1)/32*32), (int)((pos.y+1)/32*32));
    throw SuperTuxException(errmsg, __FILE__, __LINE__); */
    
    //Bad tiles (i.e. tiles that are not defined in supertux.stgt but appear in the map) are changed to ID 0 (blank tile)
    std::cout << "Warning: Undefined tile at " <<(int)pos.x/32 << "/" << (int)pos.y/32 << " (ID: " << (int)solids->get_tile_id_at(pos).id << ")" << std::endl;
    solids->change_at(pos,0);
    tile = solids->get_tile_at(pos);
  }


  if (!(tile->attributes & Tile::COIN))
    return;

  solids->change_at(pos, tile->next_tile);
  SoundManager::get()->play_sound(IDToSound(SND_DISTRO));
                                                                            
  if (bounciness == BOUNCE)
    {
      add_bouncy_distro(Vector(((int)(pos.x + 1) / 32) * 32,
                              (int)(pos.y / 32) * 32));
    }
                                                                            
  player_status.score = player_status.score + SCORE_DISTRO;
  player_status.distros++;

}
                                                                                
/* Try to bump a bad guy from below: */
void
Sector::trybumpbadguy(const Vector& pos)
{
  // Bad guys:
  for (BadGuys::iterator i = badguys.begin(); i != badguys.end(); ++i)
    {
      if ((*i)->base.x >= pos.x - 32 && (*i)->base.x <= pos.x + 32 &&
          (*i)->base.y >= pos.y - 16 && (*i)->base.y <= pos.y + 16)
        {
          (*i)->collision(player, CO_PLAYER, COLLISION_BUMP);
        }
    }
                                                                                
  // Upgrades:
  for (unsigned int i = 0; i < upgrades.size(); i++)
    {
      if (upgrades[i]->base.height == 32 &&
          upgrades[i]->base.x >= pos.x - 32 && upgrades[i]->base.x <= pos.x + 32 &&
          upgrades[i]->base.y >= pos.y - 16 && upgrades[i]->base.y <= pos.y + 16)
        {
          upgrades[i]->collision(player, CO_PLAYER, COLLISION_BUMP);
        }
    }
}

void
Sector::load_music()
{
  char* song_path;
  char* song_subtitle;
                                                                                
  level_song = SoundManager::get()->load_music(datadir + "/music/" + song_title);
                                                                                
  song_path = (char *) malloc(sizeof(char) * datadir.length() +
                              strlen(song_title.c_str()) + 8 + 5);
  song_subtitle = strdup(song_title.c_str());
  strcpy(strstr(song_subtitle, "."), "\0");
  sprintf(song_path, "%s/music/%s-fast%s", datadir.c_str(),
          song_subtitle, strstr(song_title.c_str(), "."));
  if(!SoundManager::get()->exists_music(song_path)) {
    level_song_fast = level_song;
  } else {
    level_song_fast = SoundManager::get()->load_music(song_path);
  }
  free(song_subtitle);
  free(song_path);
}

void
Sector::play_music(int type)
{
  currentmusic = type;
  switch(currentmusic) {
    case HURRYUP_MUSIC:
      SoundManager::get()->play_music(level_song_fast);
      break;
    case LEVEL_MUSIC:
      SoundManager::get()->play_music(level_song);
      break;
    case HERRING_MUSIC:
      SoundManager::get()->play_music(herring_song);
      break;
    default:
      SoundManager::get()->halt_music();
      break;
  }
}

int
Sector::get_music_type()
{
  return currentmusic;
}
