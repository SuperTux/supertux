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

#include <sstream>
#include <stdexcept>

#include "lisp/parser.hpp"
#include "math/vector.hpp"
#include "util/reader.hpp"
#include "supertux/object_factory.hpp"

#include "badguy/angrystone.hpp"
#include "badguy/badguy.hpp"
#include "badguy/bomb.hpp"
#include "badguy/skydive.hpp"
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
#include "badguy/mriceblock.hpp"
#include "badguy/mrtree.hpp"
#include "badguy/owl.hpp"
#include "badguy/plant.hpp"
#include "badguy/poisonivy.hpp"
#include "badguy/root.hpp"
#include "badguy/short_fuse.hpp"
#include "badguy/skullyhop.hpp"
#include "badguy/smartball.hpp"
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
#include "badguy/treewillowisp.hpp"
#include "badguy/walking_badguy.hpp"
#include "badguy/walkingleaf.hpp"
#include "badguy/willowisp.hpp"
#include "badguy/yeti.hpp"
#include "badguy/yeti_stalactite.hpp"
#include "badguy/zeekling.hpp"

#include "object/ambient_sound.hpp"
#include "object/anchor_point.hpp"
#include "object/background.hpp"
#include "object/bicycle_platform.hpp"
#include "object/block.hpp"
#include "object/bonus_block.hpp"
#include "object/bouncy_coin.hpp"
#include "object/brick.hpp"
#include "object/broken_brick.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/candle.hpp"
#include "object/cloud_particle_system.hpp"
#include "object/coin.hpp"
#include "object/coin_explode.hpp"
#include "object/coin_rain.hpp"
#include "object/comet_particle_system.hpp"
#include "object/decal.hpp"
#include "object/display_effect.hpp"
#include "object/electrifier.hpp"
#include "object/endsequence_fireworks.hpp"
#include "object/endsequence.hpp"
#include "object/endsequence_walkleft.hpp"
#include "object/endsequence_walkright.hpp"
#include "object/explosion.hpp"
#include "object/falling_coin.hpp"
#include "object/firefly.hpp"
#include "object/fireworks.hpp"
#include "object/floating_image.hpp"
#include "object/floating_text.hpp"
#include "object/flower.hpp"
#include "object/ghost_particle_system.hpp"
#include "object/gradient.hpp"
#include "object/growup.hpp"
#include "object/hurting_platform.hpp"
#include "object/icecrusher.hpp"
#include "object/infoblock.hpp"
#include "object/invisible_block.hpp"
#include "object/invisible_wall.hpp"
#include "object/ispy.hpp"
#include "object/lantern.hpp"
#include "object/level_time.hpp"
#include "object/light.hpp"
#include "object/magicblock.hpp"
#include "object/moving_sprite.hpp"
#include "object/oneup.hpp"
#include "object/particles.hpp"
#include "object/particlesystem.hpp"
#include "object/particlesystem_interactive.hpp"
#include "object/path.hpp"
#include "object/path_walker.hpp"
#include "object/platform.hpp"
#include "object/player.hpp"
#include "object/pneumatic_platform.hpp"
#include "object/portable.hpp"
#include "object/powerup.hpp"
#include "object/pulsing_light.hpp"
#include "object/pushbutton.hpp"
#include "object/rain_particle_system.hpp"
#include "object/rainsplash.hpp"
#include "object/rock.hpp"
#include "object/rusty_trampoline.hpp"
#include "object/scripted_object.hpp"
#include "object/skull_tile.hpp"
#include "object/smoke_cloud.hpp"
#include "object/snow_particle_system.hpp"
#include "object/specialriser.hpp"
#include "object/spotlight.hpp"
#include "object/sprite_particle.hpp"
#include "object/star.hpp"
#include "object/text_object.hpp"
#include "object/thunderstorm.hpp"
#include "object/tilemap.hpp"
#include "object/trampoline.hpp"
#include "object/unstable_tile.hpp"
#include "object/weak_block.hpp"
#include "object/wind.hpp"

#include "trigger/climbable.hpp"
#include "trigger/door.hpp"
#include "trigger/scripttrigger.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "trigger/sequence_trigger.hpp"
#include "trigger/switch.hpp"

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

ObjectFactory::~ObjectFactory()
{
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
  add_factory<Candle>("candle");
  add_factory<Coin>("coin");
  add_factory<Decal>("decal");
  add_factory<Explosion>("explosion");
  add_factory<Firefly>("firefly");
  add_factory<Gradient>("gradient");
  add_factory<HeavyCoin>("heavycoin");
  add_factory<HurtingPlatform>("hurting_platform");
  add_factory<IceCrusher>("icecrusher");
  add_factory<InfoBlock>("infoblock");
  add_factory<InvisibleWall>("invisible_wall");
  add_factory<Ispy>("ispy");
  add_factory<Lantern>("lantern");
  add_factory<LevelTime>("leveltime");
  add_factory<MagicBlock>("magicblock");
  add_factory<Platform>("platform");
  add_factory<PneumaticPlatform>("pneumatic-platform");
  add_factory<PowerUp>("powerup");
  add_factory<PushButton>("pushbutton");
  add_factory<Rock>("rock");
  add_factory<ScriptedObject>("scriptedobject");
  add_factory<SkullTile>("skull_tile");
  add_factory<Spotlight>("spotlight");
  add_factory<Thunderstorm>("thunderstorm");
  add_factory<TileMap>("tilemap");
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
}

GameObject*
ObjectFactory::create(const std::string& name, const Reader& reader)
{
  Factories::iterator i = factories.find(name);

  if (i == factories.end())
  {
    std::stringstream msg;
    msg << "No factory for object '" << name << "' found.";
    throw std::runtime_error(msg.str());
  }
  else
  {
    return i->second->create(reader);
  }
}

GameObject*
ObjectFactory::create(const std::string& name, const Vector& pos, const Direction dir)
{
  std::stringstream lisptext;
  lisptext << "((x " << pos.x << ")"
           << " (y " << pos.y << ")";
  if(dir != AUTO)
    lisptext << " (direction " << dir << "))";

  lisp::Parser parser;
  const lisp::Lisp* lisp = parser.parse(lisptext, "create_object");

  GameObject* object = create(name, *(lisp->get_car()));
  return object;
}

/* EOF */
