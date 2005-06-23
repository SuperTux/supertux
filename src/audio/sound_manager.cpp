#include "sound_manager.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>

#include "sound_file.h"
#include "sound_source.h"
#include "stream_sound_source.h"

SoundManager::SoundManager()
  : device(0), context(0), sound_enabled(false), music_source(0)
{
  try {
    device = alcOpenDevice(0);
    if(device == 0) {
      print_openal_version();
      throw std::runtime_error("Couldn't open audio device.");
    }

    int attributes[] = { 0 };
    context = alcCreateContext(device, attributes);
    check_alc_error("Couldn't create audio context: ");
    alcMakeContextCurrent(context);
    check_alc_error("Couldn't select audio context: ");

    check_al_error("Audio error after init: ");
    sound_enabled = true;
  } catch(std::exception& e) {
    device = 0;
    context = 0;
    std::cerr << "Couldn't initialize audio device:" << e.what() << "\n";
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

  if(context != 0) {
    alcMakeContextCurrent(0);
    alcDestroyContext(context);
  }
  if(device != 0) {
    alcCloseDevice(device);
  }
}

ALuint
SoundManager::load_file_into_buffer(const std::string& filename)
{
  // open sound file
  std::auto_ptr<SoundFile> file (load_sound_file(filename));
  
  ALenum format = get_sample_format(file.get());
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

SoundSource*
SoundManager::create_sound_source(const std::string& filename)
{
  if(!sound_enabled)
    return 0;

  ALuint buffer;
  
  // reuse an existing static sound buffer            
  SoundBuffers::iterator i = buffers.find(filename);
  if(i != buffers.end()) {
    buffer = i->second;
  } else {
    buffer = load_file_into_buffer(filename);
    buffers.insert(std::make_pair(filename, buffer));
  }
  
  SoundSource* source = new SoundSource();
  alSourcei(source->source, AL_BUFFER, buffer);
  return source;  
}

void
SoundManager::play(const std::string& soundname, const Vector& pos)
{
  std::string filename = "sounds/";
  filename += soundname;
  filename += ".wav";
  try {
    SoundSource* source = create_sound_source(filename);
    if(source == 0)
      return;
    if(pos == Vector(-1, -1)) {
      alSourcef(source->source, AL_ROLLOFF_FACTOR, 0);
    } else {
      source->set_position(pos);
    }
    source->play();
    sources.push_back(source);
  } catch(std::exception& e) {
    std::cout << "Couldn't play sound " << filename << ": " << e.what() << "\n";
  }
}

void
SoundManager::enable_sound(bool enable)
{
  if(device == 0)
    return;
  sound_enabled = enable;
}

void
SoundManager::enable_music(bool enable)
{
  if(device == 0)
    return;
  music_enabled = enable;
  if(music_enabled) {
    play_music(current_music);
  } else {
    if(music_source) {
      delete music_source;
      music_source = 0;
    }
  }
}

void
SoundManager::play_music(const std::string& filename)
{
  if(filename == current_music)
    return;
  current_music = filename;
  if(!music_enabled)
    return;

  try {
    StreamSoundSource* newmusic 
      = new StreamSoundSource(load_sound_file(filename));

    alSourcef(newmusic->source, AL_ROLLOFF_FACTOR, 0);
    newmusic->play();
 
    delete music_source;
    music_source = newmusic;
  } catch(std::exception& e) {
    std::cerr << "Couldn't play music file '" << filename << "': "
      << e.what() << "\n";
  }
}

void
SoundManager::set_listener_position(Vector pos)
{
  alListener3f(AL_POSITION, pos.x, pos.y, 0);
}

void
SoundManager::set_listener_velocity(Vector vel)
{
  alListener3f(AL_VELOCITY, vel.x, vel.y, 0);
}

void
SoundManager::update()
{
  // check for finished sound sources
  for(SoundSources::iterator i = sources.begin(); i != sources.end(); ) {
    SoundSource* source = *i;
    if(!source->playing()) {
      delete source;
      i = sources.erase(i);
    } else {
      ++i;
    }
  }
  // check streaming sounds
  if(music_source)
    music_source->update();
  
  alcProcessContext(context);
  check_alc_error("Error while processing audio context: ");
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
  std::cout << "OpenAL Vendor: " << alGetString(AL_VENDOR) << "\n"
            << "OpenAL Version: " << alGetString(AL_VERSION) << "\n" 
            << "OpenAL Renderer: " << alGetString(AL_RENDERER) << "\n"
            << "OpenAl Extensions: " << alGetString(AL_RENDERER) << "\n";
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

