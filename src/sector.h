#ifndef __SECTOR_H__
#define __SECTOR_H__

#include <string>
#include <vector>
#include "vector.h"
#include "badguy.h"
#include "special.h"
#include "musicref.h"
#include "screen/drawing_context.h"

class GameObject;
class Background;
class Player;
class Camera;
class Trampoline;
class FlyingPlatform;
class TileMap;
class Upgrade;
class Bullet;
class BadGuy;
class Vector;
class LispReader;
class Tile;

struct SpawnPoint
{
  std::string name;
  Vector pos;
};

/** This class holds a sector (a part of a level) and all the game objects
 * (badguys, player, background, tilemap, ...)
 */
class Sector
{
public:
  Sector();
  ~Sector();

  /// read sector from lisp file
  void parse(LispReader& reader);
  void parse_old_format(LispReader& reader);
  /// write sector to lisp file
  void write(LispWriter& writer);

  /// activates this sector (change music, intialize player class, ...)
  void activate(const std::string& spawnpoint = "main");

  void action(float elapsed_time);
  void draw(DrawingContext& context);

  /// adds a gameobject
  void add_object(GameObject* object);

  const std::string& get_name() const
  { return name; }

  void play_music(int musictype);
  int get_music_type();
  
  /** Checks for all possible collisions. And calls the
      collision_handlers, which the collision_objects provide for this
      case (or not). */
  void collision_handler();
                                                                                
  void add_score(const Vector& pos, int s);
  void add_bouncy_distro(const Vector& pos);
  void add_broken_brick(const Vector& pos, Tile* tile);
  void add_broken_brick_piece(const Vector& pos,
      const Vector& movement, Tile* tile);
  void add_bouncy_brick(const Vector& pos);
                                                                                
  BadGuy* add_bad_guy(float x, float y, BadGuyKind kind);
                                                                                
  void add_upgrade(const Vector& pos, Direction dir, UpgradeKind kind);
  bool add_bullet(const Vector& pos, float xm, Direction dir);
                                                                                
  /** Try to grab the coin at the given coordinates */
  void trygrabdistro(const Vector& pos, int bounciness);
                                                                                
  /** Try to break the brick at the given coordinates */
  bool trybreakbrick(const Vector& pos, bool small);
                                                                                
  /** Try to get the content out of a bonus box, thus emptying it */
  void tryemptybox(const Vector& pos, Direction col_side);
                                                                                
  /** Try to bumb a badguy that might we walking above Tux, thus shaking
      the tile which the badguy is walking on an killing him this way */
  void trybumpbadguy(const Vector& pos);

  /** @evil@ */
  static Sector* current()
  { return _current; }

private:
  void load_music();
  
  static Sector* _current;
  
  std::string name;

  std::string song_title;
  MusicRef level_song;
  MusicRef level_song_fast;

public:
  float gravity;

  // some special objects, where we need direct access
  Player* player;
  TileMap* solids;
  Background* background;
  Camera* camera;
  
private:
  typedef std::vector<BadGuy*> BadGuys;
  BadGuys badguys;
  typedef std::vector<Trampoline*> Trampolines;
  Trampolines trampolines;
  typedef std::vector<FlyingPlatform*> FlyingPlatforms;
  FlyingPlatforms flying_platforms;

  std::vector<Upgrade*> upgrades;
  std::vector<Bullet*> bullets;

public: // ugly
  typedef std::vector<GameObject*> GameObjects;
  GameObjects gameobjects;

private:
  typedef std::vector<SpawnPoint*> SpawnPoints;
  SpawnPoints spawnpoints;

  int distro_counter;
  bool counting_distros;
  int currentmusic;        
};

#endif

