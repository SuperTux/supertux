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
  music_source(0),
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
  delete music_source;

  for(SoundSources::iterator i = sources.begin(); i != sources.end(); ++i) {
    delete *i;
  }

  for(SoundBuffers::iterator i = buffers.begin(); i != buffers.end(); ++i) {
    ALuint buffer = i->second;
    alDeleteBuffers(1, &buffer);
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
SoundManager::load_file_into_buffer(SoundFile* file)
{
  ALenum format = get_sample_format(file);
  ALuint buffer;
  alGenBuffers(1, &buffer);
  check_al_error("Couldn't create audio buffer: ");
  char* samples = new char[file->size];
  try {
    file->read(samples, file->size);
    alBufferData(buffer, format, samples,
                 static_cast<ALsizei> (file->size),
                 static_cast<ALsizei> (file->rate));
    check_al_error("Couldn't fill audio buffer: ");
  } catch(...) {
    delete[] samples;
    throw;
  }
  delete[] samples;

  return buffer;
}

OpenALSoundSource*
SoundManager::intern_create_sound_source(const std::string& filename)
{
  assert(sound_enabled);

  std::auto_ptr<OpenALSoundSource> source (new OpenALSoundSource());

  ALuint buffer;

  // reuse an existing static sound buffer
  SoundBuffers::iterator i = buffers.find(filename);
  if(i != buffers.end()) {
    buffer = i->second;
  } else {
    // Load sound file
    std::auto_ptr<SoundFile> file (load_sound_file(filename));

    if(file->size < 100000) {
      buffer = load_file_into_buffer(file.get());
      buffers.insert(std::make_pair(filename, buffer));
    } else {
      StreamSoundSource* source = new StreamSoundSource();
      source->set_sound_file(file.release());
      return source;
    }

    log_debug << "Uncached sound \"" << filename << "\" requested to be played" << std::endl;
  }

  alSourcei(source->source, AL_BUFFER, buffer);
  return source.release();
}

SoundSource*
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
    std::auto_ptr<SoundFile> file (load_sound_file(filename));
    // only keep small files
    if(file->size >= 100000)
      return;

    ALuint buffer = load_file_into_buffer(file.get());
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
    std::auto_ptr<OpenALSoundSource> source
      (intern_create_sound_source(filename));

    if(pos.x < 0 || pos.y < 0) {
      source->set_relative(true);
    } else {
      source->set_position(pos);
    }
    source->play();
    sources.push_back(source.release());
  } catch(std::exception& e) {
    log_warning << "Couldn't play sound " << filename << ": " << e.what() << std::endl;
  }
}

void
SoundManager::manage_source(SoundSource* source)
{
  assert(source != NULL);

  OpenALSoundSource* openal_source = dynamic_cast<OpenALSoundSource*> (source);
  if(openal_source != NULL) {
    sources.push_back(openal_source);
  }
}

void
SoundManager::register_for_update( StreamSoundSource* sss ){
  if( sss != NULL ){
    update_list.push_back( sss );
  }
}

void
SoundManager::remove_from_update( StreamSoundSource* sss  ){
  if( sss != NULL ){
    StreamSoundSources::iterator i = update_list.begin();
    while( i != update_list.end() ){
      if( *i == sss ){
        i = update_list.erase(i);
      } else {
        i++;
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
      delete music_source;
      music_source = NULL;
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
    delete music_source;
    music_source = NULL;
  }
  current_music = "";
}

void
SoundManager::play_music(const std::string& filename, bool fade)
{
  if(filename == current_music && music_source != NULL)
    return;
  current_music = filename;
  if(!music_enabled)
    return;

  if(filename == "") {
    delete music_source;
    music_source = NULL;
    return;
  }

  try {
    std::auto_ptr<StreamSoundSource> newmusic (new StreamSoundSource());
    newmusic->set_sound_file(load_sound_file(filename));
    newmusic->set_looping(true);
    newmusic->set_relative(true);
    if(fade)
      newmusic->set_fading(StreamSoundSource::FadingOn, .5f);
    newmusic->play();

    delete music_source;
    music_source = newmusic.release();
  } catch(std::exception& e) {
    log_warning << "Couldn't play music file '" << filename << "': " << e.what() << std::endl;
    // When this happens, previous music continued playing, stop it, just in case.
    stop_music(0);
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

  alListener3f(AL_POSITION, pos.x, pos.y, 0);
}

void
SoundManager::set_listener_velocity(const Vector& vel)
{
  alListener3f(AL_VELOCITY, vel.x, vel.y, 0);
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
    OpenALSoundSource* source = *i;

    source->update();

    if(!source->playing()) {
      delete source;
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
    s++;
  }
}

ALenum
SoundManager::get_sample_format(SoundFile* file)
{
  if(file->channels == 2) {
    if(file->bits_per_sample == 16) {
      return AL_FORMAT_STEREO16;
    } else if(file->bits_per_sample == 8) {
      return AL_FORMAT_STEREO8;
    } else {
      throw std::runtime_error("Only 16 and 8 bit samples supported");
    }
  } else if(file->channels == 1) {
    if(file->bits_per_sample == 16) {
      return AL_FORMAT_MONO16;
    } else if(file->bits_per_sample == 8) {
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
SoundManager::check_alc_error(const char* message)
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
