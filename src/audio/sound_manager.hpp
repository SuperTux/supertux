#ifndef __SOUND_MANAGER_H__
#define __SOUND_MANAGER_H__

#include <string>
#include <vector>
#include <map>

#include <AL/alc.h>
#include <AL/al.h>
#include "math/vector.hpp"

typedef void* SoundHandle;

class SoundFile;
class SoundSource;
class StreamSoundSource;

class SoundManager
{
public:
  SoundManager();
  virtual ~SoundManager();

  void enable_sound(bool sound_enabled);
  /**
   * Creates a new sound source object which plays the specified soundfile.
   * You are responsible for deleting the sound source later (this will stop the
   * sound).
   * This function might throw exceptions. It returns 0 if no audio device is
   * available.
   */
  SoundSource* create_sound_source(const std::string& filename);
  /**
   * Convenience function to simply play a sound at a given position.
   */
  void play(const std::string& name, const Vector& pos = Vector(-1, -1));

  void set_listener_position(const Vector& position);
  void set_listener_velocity(const Vector& velocity);

  void enable_music(bool music_enabled);
  void play_music(const std::string& filename, bool fade = false);
  void stop_music(float fadetime = 0);
  
  bool is_sound_enabled() { return sound_enabled; }

  void update();

private:
  friend class SoundSource;
  friend class StreamSoundSource;

  static ALuint load_file_into_buffer(const std::string& filename);
  static ALenum get_sample_format(SoundFile* file);

  void print_openal_version();
  void check_alc_error(const char* message);
  static void check_al_error(const char* message);

  ALCdevice* device;
  ALCcontext* context;
  bool sound_enabled;

  typedef std::map<std::string, ALuint> SoundBuffers;
  SoundBuffers buffers;
  typedef std::vector<SoundSource*> SoundSources;
  SoundSources sources;

  StreamSoundSource* music_source;

  bool music_enabled;
  std::string current_music;
};

extern SoundManager* sound_manager;

#endif

