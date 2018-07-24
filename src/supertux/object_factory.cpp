//  SuperTux
//  Copyright (C) 2004 Ricardo Cruz <rick2@aeiou.pt>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "supertux/object_factory.hpp"

#include <sstream>

#include "audio/sound_source.hpp"
#include "badguy/angrystone.hpp"
#include "badguy/bouncing_snowball.hpp"
#include "badguy/captainsnowball.hpp"
#include "badguy/crystallo.hpp"
#include "badguy/dart.hpp"
#include "badguy/darttrap.hpp"
#include "badguy/dispenser.hpp"
#include "badguy/fish.hpp"
#include "badguy/flame.hpp"
#include "badguy/flyingsnowball.hpp"
#include "badguy/ghostflame.hpp"
#include "badguy/ghosttree.hpp"
#include "badguy/goldbomb.hpp"
#include "badguy/haywire.hpp"
#include "badguy/iceflame.hpp"
#include "badguy/igel.hpp"
#include "badguy/jumpy.hpp"
#include "badguy/kamikazesnowball.hpp"
#include "badguy/kugelblitz.hpp"
#include "badguy/livefire.hpp"
#include "badguy/mole.hpp"
#include "badguy/mole_rock.hpp"
#include "badguy/mrbomb.hpp"
#include "badguy/mrtree.hpp"
#include "badguy/owl.hpp"
#include "badguy/plant.hpp"
#include "badguy/poisonivy.hpp"
#include "badguy/short_fuse.hpp"
#include "badguy/skullyhop.hpp"
#include "badguy/skydive.hpp"
#include "badguy/smartball.hpp"
#include "badguy/smartblock.hpp"
#include "badguy/snail.hpp"
#include "badguy/snowball.hpp"
#include "badguy/snowman.hpp"
#include "badguy/spidermite.hpp"
#include "badguy/spiky.hpp"
#include "badguy/sspiky.hpp"
#include "badguy/stalactite.hpp"
#include "badguy/stumpy.hpp"
#include "badguy/toad.hpp"
#include "badguy/totem.hpp"
#include "badguy/walking_candle.hpp"
#include "badguy/walkingleaf.hpp"
#include "badguy/willowisp.hpp"
#include "badguy/yeti.hpp"
#include "badguy/yeti_stalactite.hpp"
#include "badguy/zeekling.hpp"
#include "editor/spawnpoint_marker.hpp"
#include "editor/worldmap_objects.hpp"
#include "math/vector.hpp"
#include "object/ambient_sound.hpp"
#include "object/background.hpp"
#include "object/bicycle_platform.hpp"
#include "object/bonus_block.hpp"
#include "object/brick.hpp"
#include "object/candle.hpp"
#include "object/cloud_particle_system.hpp"
#include "object/coin.hpp"
#include "object/decal.hpp"
#include "object/explosion.hpp"
#include "object/firefly.hpp"
#include "object/ghost_particle_system.hpp"
#include "object/gradient.hpp"
#include "object/hurting_platform.hpp"
#include "object/icecrusher.hpp"
#include "object/infoblock.hpp"
#include "object/invisible_block.hpp"
#include "object/invisible_wall.hpp"
#include "object/ispy.hpp"
#include "object/lantern.hpp"
#include "object/level_time.hpp"
#include "object/magicblock.hpp"
#include "object/platform.hpp"
#include "object/pneumatic_platform.hpp"
#include "object/powerup.hpp"
#include "object/pushbutton.hpp"
#include "object/rain_particle_system.hpp"
#include "object/rusty_trampoline.hpp"
#include "object/scripted_object.hpp"
#include "object/skull_tile.hpp"
#include "object/snow_particle_system.hpp"
#include "object/spotlight.hpp"
#include "object/thunderstorm.hpp"
#include "object/tilemap.hpp"
#include "object/torch.hpp"
#include "object/trampoline.hpp"
#include "object/unstable_tile.hpp"
#include "object/weak_block.hpp"
#include "object/wind.hpp"
#include "supertux/level.hpp"
#include "supertux/tile_manager.hpp"
#include "trigger/climbable.hpp"
#include "trigger/door.hpp"
#include "trigger/scripttrigger.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "trigger/sequence_trigger.hpp"
#include "trigger/switch.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

ObjectFactory&
ObjectFactory::instance()
{
  static ObjectFactory instance_;
  return instance_;
}

ObjectFactory::ObjectFactory() :
  factories()
{
  init_factories();
}

void
ObjectFactory::init_factories()
{
  // badguys
  add_factory<AngryStone>("angrystone");
  add_factory<BouncingSnowball>("bouncingsnowball");
  add_factory<CaptainSnowball>("captainsnowball");
  add_factory<Crystallo>("crystallo");
  add_factory<Dart>("dart");
  add_factory<DartTrap>("darttrap");
  add_factory<Dispenser>("dispenser");
  add_factory<Fish>("fish");
  add_factory<Flame>("flame");
  add_factory<FlyingSnowBall>("flyingsnowball");
  add_factory<Ghostflame>("ghostflame");
  add_factory<GhostTree>("ghosttree");
  add_factory<GoldBomb>("goldbomb");
  add_factory<Haywire>("haywire");
  add_factory<Iceflame>("iceflame");
  add_factory<Igel>("igel");
  add_factory<Jumpy>("jumpy");
  add_factory<KamikazeSnowball>("kamikazesnowball");
  add_factory<Kugelblitz>("kugelblitz");
  add_factory<LeafShot>("leafshot");
  add_factory<LiveFire>("livefire");
  add_factory<LiveFireAsleep>("livefire_asleep");
  add_factory<LiveFireDormant>("livefire_dormant");
  add_factory<Mole>("mole");
  add_factory<MoleRock>("mole_rock");
  add_factory<MrBomb>("mrbomb");
  add_factory<MrIceBlock>("mriceblock");
  add_factory<MrTree>("mrtree");
  add_factory<Owl>("owl");
  add_factory<Plant>("plant");
  add_factory<PoisonIvy>("poisonivy");
  add_factory<ShortFuse>("short_fuse");
  add_factory<SSpiky>("sspiky");
  add_factory<SkyDive>("skydive");
  add_factory<SkullyHop>("skullyhop");
  add_factory<SmartBall>("smartball");
  add_factory<SmartBlock>("smartblock");
  add_factory<Snail>("snail");
  add_factory<SnowBall>("snowball");
  add_factory<Snowman>("snowman");
  add_factory<SpiderMite>("spidermite");
  add_factory<Spiky>("spiky");
  add_factory<Stalactite>("stalactite");
  add_factory<Stumpy>("stumpy");
  add_factory<Toad>("toad");
  add_factory<Totem>("totem");
  add_factory<WalkingCandle>("walking_candle");
  add_factory<WalkingLeaf>("walkingleaf");
  add_factory<WillOWisp>("willowisp");
  add_factory<Yeti>("yeti");
  add_factory<YetiStalactite>("yeti_stalactite");
  add_factory<Zeekling>("zeekling");

  // other objects
  add_factory<AmbientSound>("ambient_sound");
  add_factory<Background>("background");
  add_factory<BicyclePlatform>("bicycle-platform");
  add_factory<BonusBlock>("bonusblock");
  add_factory<Brick>("brick");
  add_factory<Candle>("candle");
  add_factory<CloudParticleSystem>("particles-clouds");
  add_factory<Coin>("coin");
  add_factory<Decal>("decal");
  add_factory<Explosion>("explosion");
  add_factory<Firefly>("firefly");
  add_factory<GhostParticleSystem>("particles-ghosts");
  add_factory<Gradient>("gradient");
  add_factory<HeavyCoin>("heavycoin");
  add_factory<HurtingPlatform>("hurting_platform");
  add_factory<IceCrusher>("icecrusher");
  add_factory<InfoBlock>("infoblock");
  add_factory<InvisibleBlock>("invisible_block");
  add_factory<InvisibleWall>("invisible_wall");
  add_factory<Ispy>("ispy");
  add_factory<Lantern>("lantern");
  add_factory<LevelTime>("leveltime");
  add_factory<MagicBlock>("magicblock");
  add_factory<Platform>("platform");
  add_factory<PneumaticPlatform>("pneumatic-platform");
  add_factory<PowerUp>("powerup");
  add_factory<PushButton>("pushbutton");
  add_factory<RainParticleSystem>("particles-rain");
  add_factory<Rock>("rock");
  add_factory<ScriptedObject>("scriptedobject");
  add_factory<SkullTile>("skull_tile");
  add_factory<SnowParticleSystem>("particles-snow");
  add_factory<Spotlight>("spotlight");
  add_factory<Thunderstorm>("thunderstorm");
  add_factory<Torch>("torch");
  add_factory<Trampoline>("trampoline");
  add_factory<RustyTrampoline>("rustytrampoline");
  add_factory<UnstableTile>("unstable_tile");
  add_factory<WeakBlock>("weak_block");
  add_factory<Wind>("wind");

  // trigger
  add_factory<Climbable>("climbable");
  add_factory<Door>("door");
  add_factory<ScriptTrigger>("scripttrigger");
  add_factory<SecretAreaTrigger>("secretarea");
  add_factory<SequenceTrigger>("sequencetrigger");
  add_factory<Switch>("switch");

  // editor stuff
  add_factory<SpawnPointMarker>("spawnpoint");

  // worldmap editor objects
  add_factory<worldmap_editor::LevelDot>("level");
  add_factory<worldmap_editor::SpecialTile>("special-tile");
  add_factory<worldmap_editor::SpriteChange>("sprite-change");
  add_factory<worldmap_editor::Teleporter>("teleporter");
  add_factory<worldmap_editor::WorldmapSpawnPoint>("worldmap-spawnpoint");

  add_factory("tilemap", [](const ReaderMapping& reader) {
      auto tileset = TileManager::current()->get_tileset(Level::current()->get_tileset());
      return std::make_shared<TileMap>(tileset, reader);
    });
}

GameObjectPtr
ObjectFactory::create(const std::string& name, const ReaderMapping& reader) const
{
  Factories::const_iterator i = factories.find(name);

  if (i == factories.end())
  {
    std::stringstream msg;
    msg << "No factory for object '" << name << "' found.";
    throw std::runtime_error(msg.str());
  }
  else
  {
    return i->second(reader);
  }
}

GameObjectPtr
ObjectFactory::create(const std::string& name, const Vector& pos, const Direction& dir, const std::string& data) const
{
  std::stringstream lisptext;
  lisptext << "(" << name << "\n"
           << " (x " << pos.x << ")"
           << " (y " << pos.y << ")" << data;
  if(dir != AUTO) {
    lisptext << " (direction \"" << dir << "\"))";
  } else {
    lisptext << ")";
  }

  auto doc = ReaderDocument::parse(lisptext);
  return create(name, doc.get_root().get_mapping());
}

/* EOF */
