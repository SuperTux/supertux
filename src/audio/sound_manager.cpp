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

#include <SDL.h>
#include <assert.h>
#include <stdexcept>
#include <sstream>
#include <memory>

#include "audio/dummy_sound_source.hpp"
#include "audio/sound_file.hpp"
#include "audio/stream_sound_source.hpp"
#include "util/log.hpp"

SoundManager::SoundManager() :
  device(0),
  context(0),
  sound_enabled(false),
  buffers(),
  sources(),
  update_list(),
  music_source(),
  music_enabled(false),
  current_music()
{
  try {
    device = alcOpenDevice(0);
    if (device == NULL) {
      throw std::runtime_error("Couldn't open audio device.");
    }

    int attributes[] = { 0 };
    context = alcCreateContext(device, attributes);
    check_alc_error("Couldn't create audio context: ");
    alcMakeContextCurrent(context);
    check_alc_error("Couldn't select audio context: ");

    check_al_error("Audio error after init: ");
    sound_enabled = true;
    music_enabled = true;

    set_listener_orientation(Vector(0.0f, 0.0f), Vector(0.0f, -1.0f));
  } catch(std::exception& e) {
    if(context != NULL) {
      alcDestroyContext(context);
      context = NULL;
    }
    if(device != NULL) {
      alcCloseDevice(device);
      device = NULL;
    }
    log_warning << "Couldn't initialize audio device: " << e.what() << std::endl;
    print_openal_version();
  }
}

SoundManager::~SoundManager()
{
  music_source.reset();
  sources.clear();

  for(const auto& buffer : buffers) {
    alDeleteBuffers(1, &buffer.second);
  }

  if(context != NULL) {
    alcDestroyContext(context);
    context = NULL;
  }
  if(device != NULL) {
    alcCloseDevice(device);
    device = NULL;
  }
}

ALuint
SoundManager::load_file_into_buffer(SoundFile& file)
{
  ALenum format = get_sample_format(file);
  ALuint buffer;
  alGenBuffers(1, &buffer);
  check_al_error("Couldn't create audio buffer: ");
  std::unique_ptr<char[]> samples(new char[file.size]);
  file.read(samples.get(), file.size);
  log_debug << "buffer: " << buffer << "\n"
            << "format: " << format << "\n"
            << "samples: " << samples.get() << "\n"
            << "file size: " << static_cast<ALsizei>(file.size) << "\n"
            << "file rate: " << static_cast<ALsizei>(file.rate) << "\n";

  alBufferData(buffer, format, samples.get(),
               static_cast<ALsizei>(file.size),
               static_cast<ALsizei>(file.rate));
  check_al_error("Couldn't fill audio buffer: ");

  return buffer;
}

std::unique_ptr<OpenALSoundSource>
SoundManager::intern_create_sound_source(const std::string& filename)
{
  assert(sound_enabled);

  std::unique_ptr<OpenALSoundSource> source(new OpenALSoundSource);

  ALuint buffer;

  // reuse an existing static sound buffer
  SoundBuffers::iterator i = buffers.find(filename);
  if(i != buffers.end()) {
    buffer = i->second;
  } else {
    // Load sound file
    std::unique_ptr<SoundFile> file(load_sound_file(filename));

    if(file->size < 100000) {
      buffer = load_file_into_buffer(*file);
      buffers.insert(std::make_pair(filename, buffer));
    } else {
      std::unique_ptr<StreamSoundSource> source_(new StreamSoundSource);
      source_->set_sound_file(std::move(file));
      return std::move(source_);
    }

    log_debug << "Uncached sound \"" << filename << "\" requested to be played" << std::endl;
  }

  alSourcei(source->source, AL_BUFFER, buffer);
  return source;
}

std::unique_ptr<SoundSource>
SoundManager::create_sound_source(const std::string& filename)
{
  if(!sound_enabled)
    return create_dummy_sound_source();

  try {
    return intern_create_sound_source(filename);
  } catch(std::exception &e) {
    log_warning << "Couldn't create audio source: " << e.what() << std::endl;
    return create_dummy_sound_source();
  }
}

void
SoundManager::preload(const std::string& filename)
{
  if(!sound_enabled)
    return;

  SoundBuffers::iterator i = buffers.find(filename);
  // already loaded?
  if(i != buffers.end())
    return;
  try {
    std::unique_ptr<SoundFile> file (load_sound_file(filename));
    // only keep small files
    if(file->size >= 100000)
      return;

    ALuint buffer = load_file_into_buffer(*file);
    buffers.insert(std::make_pair(filename, buffer));
  } catch(std::exception& e) {
    log_warning << "Error while preloading sound file: " << e.what() << std::endl;
  }
}

void
SoundManager::play(const std::string& filename, const Vector& pos)
{
  if(!sound_enabled)
    return;

  try {
    std::unique_ptr<OpenALSoundSource> source(intern_create_sound_source(filename));

    if(pos.x < 0 || pos.y < 0) {
      source->set_relative(true);
    } else {
      source->set_position(pos);
    }
    source->play();
    sources.push_back(std::move(source));
  } catch(std::exception& e) {
    log_warning << "Couldn't play sound " << filename << ": " << e.what() << std::endl;
  }
}

void
SoundManager::manage_source(std::unique_ptr<SoundSource> source)
{
  assert(source);
  if (dynamic_cast<OpenALSoundSource*>(source.get()))
  {
    std::unique_ptr<OpenALSoundSource> openal_source(dynamic_cast<OpenALSoundSource*>(source.release()));
    sources.push_back(std::move(openal_source));
  }
}

void
SoundManager::register_for_update(StreamSoundSource* sss)
{
  if (sss)
  {
    update_list.push_back(sss);
  }
}

void
SoundManager::remove_from_update(StreamSoundSource* sss)
{
  if (sss)
  {
    StreamSoundSources::iterator i = update_list.begin();
    while( i != update_list.end() ){
      if( *i == sss ){
        i = update_list.erase(i);
      } else {
        ++i;
      }
    }
  }
}

void
SoundManager::enable_sound(bool enable)
{
  if(device == NULL)
    return;

  sound_enabled = enable;
}

void
SoundManager::enable_music(bool enable)
{
  if(device == NULL)
    return;

  music_enabled = enable;
  if(music_enabled) {
    play_music(current_music);
  } else {
    if(music_source) {
      music_source.reset();
    }
  }
}

void
SoundManager::stop_music(float fadetime)
{
  if(fadetime > 0) {
    if(music_source
       && music_source->get_fade_state() != StreamSoundSource::FadingOff)
      music_source->set_fading(StreamSoundSource::FadingOff, fadetime);
  } else {
    music_source.reset();
  }
  current_music = "";
}

void
SoundManager::play_music(const std::string& filename, bool fade)
{
  if(filename == current_music && music_source != NULL)
  {
    if(music_source->paused())
    {
      music_source->resume();
    }
    else if(!music_source->playing())
    {
      music_source->play();
    }
    return;
  }
  current_music = filename;
  if(!music_enabled)
    return;

  if(filename.empty()) {
    music_source.reset();
    return;
  }

  try {
    std::unique_ptr<StreamSoundSource> newmusic (new StreamSoundSource());
    newmusic->set_sound_file(load_sound_file(filename));
    newmusic->set_looping(true);
    newmusic->set_relative(true);
    if(fade)
      newmusic->set_fading(StreamSoundSource::FadingOn, .5f);
    newmusic->play();

    music_source = std::move(newmusic);
  } catch(std::exception& e) {
    log_warning << "Couldn't play music file '" << filename << "': " << e.what() << std::endl;
    // When this happens, previous music continued playing, stop it, just in case.
    stop_music(0);
  }
}

void
SoundManager::pause_music(float fadetime)
{
  if(music_source == NULL)
    return;

  if(fadetime > 0) {
    if(music_source
       && music_source->get_fade_state() != StreamSoundSource::FadingPause)
      music_source->set_fading(StreamSoundSource::FadingPause, fadetime);
  } else {
    music_source->pause();
  }
}

void
SoundManager::pause_sounds()
{
  for(auto& source : sources) {
    if(source->playing()) {
      source->pause();
    }
  }
}

void
SoundManager::resume_sounds()
{
  for(auto& source : sources) {
    if(source->paused()) {
      source->resume();
    }
  }
}

void
SoundManager::stop_sounds()
{
  for(auto& source : sources) {
    source->stop();
  }
}

void
SoundManager::resume_music(float fadetime)
{
  if(music_source == NULL)
    return;

  if(fadetime > 0) {
    if(music_source
       && music_source->get_fade_state() != StreamSoundSource::FadingResume)
      music_source->set_fading(StreamSoundSource::FadingResume, fadetime);
  } else {
    music_source->resume();
  }
}

void
SoundManager::set_listener_position(const Vector& pos)
{
  static Uint32 lastticks = SDL_GetTicks();

  Uint32 current_ticks = SDL_GetTicks();
  if(current_ticks - lastticks < 300)
    return;
  lastticks = current_ticks;

  alListener3f(AL_POSITION, pos.x, pos.y, -300);
}

void
SoundManager::set_listener_velocity(const Vector& vel)
{
  alListener3f(AL_VELOCITY, vel.x, vel.y, 0);
}

void
SoundManager::set_listener_orientation(const Vector& at, const Vector& up)
{
  ALfloat orientation[]={at.x, at.y, 1.0, up.x, up.y, 0.0};
  alListenerfv(AL_ORIENTATION, orientation);
}

void
SoundManager::update()
{
  static Uint32 lasttime = SDL_GetTicks();
  Uint32 now = SDL_GetTicks();

  if(now - lasttime < 300)
    return;
  lasttime = now;

  // update and check for finished sound sources
  for(SoundSources::iterator i = sources.begin(); i != sources.end(); ) {
    auto& source = *i;

    source->update();

    if(!source->playing()) {
      i = sources.erase(i);
    } else {
      ++i;
    }
  }
  // check streaming sounds
  if(music_source) {
    music_source->update();
  }

  if (context)
  {
    alcProcessContext(context);
    check_alc_error("Error while processing audio context: ");
  }

  //run update() for stream_sound_source
  StreamSoundSources::iterator s = update_list.begin();
  while( s != update_list.end() ){
    (*s)->update();
    ++s;
  }
}

ALenum
SoundManager::get_sample_format(const SoundFile& file)
{
  if(file.channels == 2) {
    if(file.bits_per_sample == 16) {
      return AL_FORMAT_STEREO16;
    } else if(file.bits_per_sample == 8) {
      return AL_FORMAT_STEREO8;
    } else {
      throw std::runtime_error("Only 16 and 8 bit samples supported");
    }
  } else if(file.channels == 1) {
    if(file.bits_per_sample == 16) {
      return AL_FORMAT_MONO16;
    } else if(file.bits_per_sample == 8) {
      return AL_FORMAT_MONO8;
    } else {
      throw std::runtime_error("Only 16 and 8 bit samples supported");
    }
  }

  throw std::runtime_error("Only 1 and 2 channel samples supported");
}

void
SoundManager::print_openal_version()
{
  log_info << "OpenAL Vendor: " << alGetString(AL_VENDOR) << std::endl;
  log_info << "OpenAL Version: " << alGetString(AL_VERSION) << std::endl;
  log_info << "OpenAL Renderer: " << alGetString(AL_RENDERER) << std::endl;
  log_info << "OpenAl Extensions: " << alGetString(AL_EXTENSIONS) << std::endl;
}

void
SoundManager::check_alc_error(const char* message) const
{
  int err = alcGetError(device);
  if(err != ALC_NO_ERROR) {
    std::stringstream msg;
    msg << message << alcGetString(device, err);
    throw std::runtime_error(msg.str());
  }
}

void
SoundManager::check_al_error(const char* message)
{
  int err = alGetError();
  if(err != AL_NO_ERROR) {
    std::stringstream msg;
    msg << message << alGetString(err);
    throw std::runtime_error(msg.str());
  }
}

/* EOF */
