//  SuperTux
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

#include "audio/sound_manager.hpp"

#include <unordered_map>

#include <wstsound/sound_file.hpp>

#include "audio/sound_source.hpp"
#include "physfs/ifile_stream.hpp"
#include "util/file_system.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

namespace {

// List obtained with the help of sed:
// find | sort | sed 's:^\.:/music:; /\./ !d; s:\(.*/\)\([^/]*$\):{"\2", "\1\2"},:g'
std::unordered_map<std::string, std::string> fallback_paths = {
  {"airship_2.ogg", "/music/antarctic/airship_2.ogg"},
  {"airship_remix-2.music", "/music/antarctic/airship_remix-2.music"},
  {"airship_remix.music", "/music/antarctic/airship_remix.music"},
  {"airship_remix.ogg", "/music/antarctic/airship_remix.ogg"},
  {"arctic_breeze.music", "/music/antarctic/arctic_breeze.music"},
  {"arctic_breeze.ogg", "/music/antarctic/arctic_breeze.ogg"},
  {"arctic_cave.music", "/music/antarctic/arctic_cave.music"},
  {"arctic_cave.ogg", "/music/antarctic/arctic_cave.ogg"},
  {"bossattack.music", "/music/antarctic/bossattack.music"},
  {"bossattack.ogg", "/music/antarctic/bossattack.ogg"},
  {"cave.music", "/music/antarctic/cave.music"},
  {"cave.ogg", "/music/antarctic/cave.ogg"},
  {"chipdisko.music", "/music/antarctic/chipdisko.music"},
  {"chipdisko.ogg", "/music/antarctic/chipdisko.ogg"},
  {"jewels.music", "/music/antarctic/jewels.music"},
  {"jewels.ogg", "/music/antarctic/jewels.ogg"},
  {"salcon.music", "/music/antarctic/salcon.music"},
  {"salcon.ogg", "/music/antarctic/salcon.ogg"},
  {"voc-boss.music", "/music/antarctic/voc-boss.music"},
  {"voc-boss.ogg", "/music/antarctic/voc-boss.ogg"},
  {"voc-dark.music", "/music/antarctic/voc-dark.music"},
  {"voc-dark.ogg", "/music/antarctic/voc-dark.ogg"},
  {"voc-daytime2.music", "/music/antarctic/voc-daytime2.music"},
  {"voc-daytime2.ogg", "/music/antarctic/voc-daytime2.ogg"},
  {"voc-daytime.music", "/music/antarctic/voc-daytime.music"},
  {"voc-daytime.ogg", "/music/antarctic/voc-daytime.ogg"},
  {"voc-night.music", "/music/antarctic/voc-night.music"},
  {"voc-night.ogg", "/music/antarctic/voc-night.ogg"},
  {"darkforestkeep.music", "/music/castle/darkforestkeep.music"},
  {"darkforestkeep.ogg", "/music/castle/darkforestkeep.ogg"},
  {"fortress.music", "/music/castle/fortress.music"},
  {"fortress.ogg", "/music/castle/fortress.ogg"},
  {"beneath_the_rabbit_hole.music", "/music/forest/beneath_the_rabbit_hole.music"},
  {"beneath_the_rabbit_hole.ogg", "/music/forest/beneath_the_rabbit_hole.ogg"},
  {"bright_thunders.music", "/music/forest/bright_thunders.music"},
  {"bright_thunders.ogg", "/music/forest/bright_thunders.ogg"},
  {"clavelian_march.music", "/music/forest/clavelian_march.music"},
  {"clavelian_march.ogg", "/music/forest/clavelian_march.ogg"},
  {"forest2.music", "/music/forest/forest2.music"},
  {"forest2.ogg", "/music/forest/forest2.ogg"},
  {"forest3.music", "/music/forest/forest3.music"},
  {"forest3.ogg", "/music/forest/forest3.ogg"},
  {"forest-cave.music", "/music/forest/forest-cave.music"},
  {"forest-cave.ogg", "/music/forest/forest-cave.ogg"},
  {"forest-map.music", "/music/forest/forest-map.music"},
  {"forestmap.ogg", "/music/forest/forestmap.ogg"},
  {"forest.music", "/music/forest/forest.music"},
  {"forest.ogg", "/music/forest/forest.ogg"},
  {"forest-sprint.music", "/music/forest/forest-sprint.music"},
  {"forest-sprint.ogg", "/music/forest/forest-sprint.ogg"},
  {"forest_theme.music", "/music/forest/forest_theme.music"},
  {"forest_theme.ogg", "/music/forest/forest_theme.ogg"},
  {"ghostforest2.music", "/music/forest/ghostforest2.music"},
  {"ghostforest2.ogg", "/music/forest/ghostforest2.ogg"},
  {"ghostforest_map.music", "/music/forest/ghostforest_map.music"},
  {"ghostforest_map.ogg", "/music/forest/ghostforest_map.ogg"},
  {"ghostforest.music", "/music/forest/ghostforest.music"},
  {"ghostforest.ogg", "/music/forest/ghostforest.ogg"},
  {"greatgigantic.music", "/music/forest/greatgigantic.music"},
  {"greatgigantic.ogg", "/music/forest/greatgigantic.ogg"},
  {"new_forest_map.music", "/music/forest/new_forest_map.music"},
  {"new_forest_map.ogg", "/music/forest/new_forest_map.ogg"},
  {"shallow-green.music", "/music/forest/shallow-green.music"},
  {"shallow-green.ogg", "/music/forest/shallow-green.ogg"},
  {"treeboss.music", "/music/forest/treeboss.music"},
  {"treeboss.ogg", "/music/forest/treeboss.ogg"},
  {"wisphunt.music", "/music/forest/wisphunt.music"},
  {"wisphunt.ogg", "/music/forest/wisphunt.ogg"},
  {"battle_theme.music", "/music/misc/battle_theme.music"},
  {"battle_theme.ogg", "/music/misc/battle_theme.ogg"},
  {"bonuscave.music", "/music/misc/bonuscave.music"},
  {"bonuscave.ogg", "/music/misc/bonuscave.ogg"},
  {"christmas_theme.music", "/music/misc/christmas_theme.music"},
  {"christmas_theme.ogg", "/music/misc/christmas_theme.ogg"},
  {"credits.music", "/music/misc/credits.music"},
  {"credits.ogg", "/music/misc/credits.ogg"},
  {"halloween_1.music", "/music/misc/halloween_1.music"},
  {"halloween_1.ogg", "/music/misc/halloween_1.ogg"},
  {"intro.music", "/music/misc/intro.music"},
  {"intro.ogg", "/music/misc/intro.ogg"},
  {"invincible.ogg", "/music/misc/invincible.ogg"},
  {"leveldone.ogg", "/music/misc/leveldone.ogg"},
  {"theme.music", "/music/misc/theme.music"},
  {"theme.ogg", "/music/misc/theme.ogg"},
};

const std::string& get_fallback_path(const std::string& file_path)
{
  std::string file_name = FileSystem::basename(file_path);
  auto it = fallback_paths.find(file_name);
  if (it != fallback_paths.end())
    return it->second;
  // No fallback path found
  return file_path;
}

struct MusicFile
{
  std::string file;
  float loop_begin;
  float loop_at;
};

ReaderDocument doc_from_file_fallback(std::string& filename)
{
  try {
    return ReaderDocument::from_file(filename);
  } catch(const std::exception&) {
    filename = get_fallback_path(filename);
    return ReaderDocument::from_file(filename);
  }
}

MusicFile load_music_file(const std::string& filename_original)
{
  // filename may be changed by doc_from_file_fallback
  std::string filename = filename_original;
  auto doc = doc_from_file_fallback(filename);
  auto root = doc.get_root();
  if (root.get_name() != "supertux-music") {
    throw std::runtime_error("file is not a supertux-music file.");
  }

  auto music = root.get_mapping();

  std::string raw_music_file;
  float loop_begin = 0;
  float loop_at    = -1;

  music.get("file", raw_music_file);
  music.get("loop-begin", loop_begin);
  music.get("loop-at", loop_at);

  if (loop_begin < 0) {
    throw std::runtime_error("can't loop from negative value");
  }

  std::string basedir = FileSystem::dirname(filename);
  raw_music_file = FileSystem::normalize(basedir + raw_music_file);

  return MusicFile{raw_music_file, loop_begin, loop_at};
}

} // namespace

SoundManager::SoundManager() :
  m_sound_mgr([](std::filesystem::path const& filename){
    return std::make_unique<IFileStream>(filename.string());
  }),
  m_sound_enabled(true),
  m_music_enabled(true)
{
}

void
SoundManager::play(const std::string& name, const Vector& pos, const float gain)
{
  auto source = m_sound_mgr.sound().play(name);
  source->set_position(pos.x, pos.y, 0.0f);
  source->set_gain(gain);
}

void
SoundManager::play(const std::string& name, const float gain)
{
  play(name, Vector(-1, -1), gain);
}

/** preloads a sound, so that you don't get a lag later when playing it */
void
SoundManager::preload(const std::string& name)
{
}

void
SoundManager::play_music(const std::string& filename, float fadetime)
{
  m_music_filename = filename;

  if (filename.ends_with(".music")) {
    auto music_file = load_music_file(filename);
    m_sound_mgr.music().play(music_file.file, wstsound::SoundSourceType::STREAM);
  } else {
    m_sound_mgr.music().play(filename, wstsound::SoundSourceType::STREAM);
  }
}

void
SoundManager::play_music(const std::string& filename, bool fade)
{
  play_music(filename, fade ? 0.5f : 0);
}

void
SoundManager::pause_music(float fadetime)
{
  m_sound_mgr.music().pause();
}

void
SoundManager::resume_music(float fadetime)
{
  m_sound_mgr.music().resume();
}

void
SoundManager::stop_music()
{
  m_sound_mgr.music().stop();
}

void
SoundManager::stop_music(float fadetime)
{
  m_sound_mgr.music().stop();
}

std::string
SoundManager::get_current_music() const
{
  return {};
}

void
SoundManager::pause_sounds()
{
  m_sound_mgr.sound().pause();
}

void
SoundManager::resume_sounds()
{
  m_sound_mgr.sound().resume();
}

void
SoundManager::stop_sounds()
{
 m_sound_mgr.sound().stop();
}

void
SoundManager::update()
{
#if 0
  if (m_music_source == nullptr)
    return;

  if (fadetime > 0) {
    if (m_music_source
       && m_music_source->get_fade_state() != StreamSoundSource::FadingResume) {
      m_music_source->set_fading(StreamSoundSource::FadingResume, fadetime);
      m_music_source->resume();
    }
  } else {
    m_music_source->resume();
  }
#endif

  m_sound_mgr.update(1.0f);
}

std::unique_ptr<SoundSource>
SoundManager::create_sound_source(const std::string& filename)
{
  auto source = m_sound_mgr.create_sound_source(filename, m_sound_mgr.sound(),
                                                wstsound::SoundSourceType::STATIC);
  return std::make_unique<SoundSource>(source);
}

void
SoundManager::set_listener_position(const Vector& position)
{
  m_sound_mgr.set_listener_position(position.x, position.y, -300);
}

void
SoundManager::manage_source(std::unique_ptr<SoundSource> source)
{
}

void
SoundManager::enable_sound(bool sound_enabled)
{
  m_sound_enabled = sound_enabled;
}

void
SoundManager::enable_music(bool music_enabled)
{
  m_music_enabled = music_enabled;
}

void
SoundManager::set_music_volume(int volume)
{
  m_sound_mgr.music().set_gain(static_cast<float>(volume) / 100.0f);
}

void
SoundManager::set_sound_volume(int volume)
{
  m_sound_mgr.sound().set_gain(static_cast<float>(volume) / 100.0f);
}

bool
SoundManager::is_audio_enabled() const
{
  return true;
}

/* EOF */
