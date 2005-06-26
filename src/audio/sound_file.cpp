/** Used SDL_mixer and glest source as reference */
#include <config.h>

#include "sound_file.hpp"

#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <assert.h>
#include <physfs.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

class WavSoundFile : public SoundFile
{
public:
  WavSoundFile(PHYSFS_file* file);
  ~WavSoundFile();

  size_t read(void* buffer, size_t buffer_size);
  void reset();

private:
  PHYSFS_file* file;
  
  PHYSFS_sint64 datastart;
};

static inline uint32_t read32LE(PHYSFS_file* file)
{
  uint32_t result;
  if(PHYSFS_readULE32(file, &result) == 0)
    throw std::runtime_error("file too short");

  return result;
}

static inline uint16_t read16LE(PHYSFS_file* file)
{
  uint16_t result;
  if(PHYSFS_readULE16(file, &result) == 0)
    throw std::runtime_error("file too short");

  return result;
}

WavSoundFile::WavSoundFile(PHYSFS_file* file)
{
  this->file = file;

  char magic[4];
  if(PHYSFS_read(file, magic, sizeof(magic), 1) != 1)
    throw std::runtime_error("Couldn't read file magic (not a wave file)");
  if(strncmp(magic, "RIFF", 4) != 0) {
    printf("MAGIC: %4s.\n", magic);
    throw std::runtime_error("file is not a RIFF wav file");
  }

  uint32_t wavelen = read32LE(file);
  (void) wavelen;
  
  if(PHYSFS_read(file, magic, sizeof(magic), 1) != 1)
    throw std::runtime_error("Couldn't read chunk header (not a wav file?)");
  if(strncmp(magic, "WAVE", 4) != 0)
    throw std::runtime_error("file is not a valid RIFF/WAVE file");

  char chunkmagic[4];
  uint32_t chunklen;

  // search audio data format chunk
  do {
    if(PHYSFS_read(file, chunkmagic, sizeof(chunkmagic), 1) != 1)
      throw std::runtime_error("EOF while searching format chunk");    
    chunklen = read32LE(file);
    
    if(strncmp(chunkmagic, "fmt ", 4) == 0)
      break;

    if(strncmp(chunkmagic, "fact", 4) == 0
        || strncmp(chunkmagic, "LIST", 4) == 0) {
      // skip chunk
      if(PHYSFS_seek(file, PHYSFS_tell(file) + chunklen) == 0)
        throw std::runtime_error("EOF while searching fmt chunk");
    } else {
      throw std::runtime_error("complex WAVE files not supported");
    }
  } while(true); 

  if(chunklen < 16)
    throw std::runtime_error("Format chunk too short");
 
  // parse format
  uint16_t encoding = read16LE(file);
  if(encoding != 1)
    throw std::runtime_error("only PCM encoding supported");
  channels = read16LE(file);
  rate = read32LE(file);
  uint32_t byterate = read32LE(file);
  (void) byterate;
  uint16_t blockalign = read16LE(file);
  (void) blockalign;
  bits_per_sample = read16LE(file);

  if(chunklen > 16) {
    if(PHYSFS_seek(file, PHYSFS_tell(file) + (chunklen-16)) == 0)
      throw std::runtime_error("EOF while reading reast of format chunk");
  }

  // set file offset to DATA chunk data
  do {
    if(PHYSFS_read(file, chunkmagic, sizeof(chunkmagic), 1) != 1)
      throw std::runtime_error("EOF while searching data chunk");    
    chunklen = read32LE(file);

    if(strncmp(chunkmagic, "data", 4) == 0)
      break;

    // skip chunk
    if(PHYSFS_seek(file, PHYSFS_tell(file) + chunklen) == 0)
      throw std::runtime_error("EOF while searching fmt chunk");
  } while(true);

  datastart = PHYSFS_tell(file);
  size = static_cast<size_t> (chunklen);
}

WavSoundFile::~WavSoundFile()
{
  PHYSFS_close(file);
}

void
WavSoundFile::reset()
{
  if(PHYSFS_seek(file, datastart) == 0)
    throw std::runtime_error("Couldn't seek to data start");
}

size_t
WavSoundFile::read(void* buffer, size_t buffer_size)
{
  PHYSFS_sint64 end = datastart + size;
  PHYSFS_sint64 cur = PHYSFS_tell(file);
  if(cur >= end)
    return 0;
  
  size_t readsize = std::min(static_cast<size_t> (end - cur), buffer_size);
  if(PHYSFS_read(file, buffer, readsize, 1) != 1)
    throw std::runtime_error("read error while reading samples");

  return readsize;
}

//---------------------------------------------------------------------------

class OggSoundFile : public SoundFile
{
public:
  OggSoundFile(PHYSFS_file* file);
  ~OggSoundFile();

  size_t read(void* buffer, size_t buffer_size);
  void reset();

private:
  static size_t cb_read(void* ptr, size_t size, size_t nmemb, void* source);
  static int cb_seek(void* source, ogg_int64_t offset, int whence);
  static int cb_close(void* source);
  static long cb_tell(void* source);
  
  PHYSFS_file* file;
  OggVorbis_File vorbis_file;
};

OggSoundFile::OggSoundFile(PHYSFS_file* file)
{
  this->file = file;

  ov_callbacks callbacks = { cb_read, cb_seek, cb_close, cb_tell };
  ov_open_callbacks(file, &vorbis_file, 0, 0, callbacks);

  vorbis_info* vi = ov_info(&vorbis_file, -1);
  channels = vi->channels;
  rate = vi->rate;
  bits_per_sample = 16;
  size = static_cast<size_t> (ov_pcm_total(&vorbis_file, -1) * 2);
}

OggSoundFile::~OggSoundFile()
{
  ov_clear(&vorbis_file);
}

size_t
OggSoundFile::read(void* _buffer, size_t buffer_size)
{
  char* buffer = reinterpret_cast<char*> (_buffer);
  int section = 0;
  size_t totalBytesRead= 0;

  while(buffer_size>0){
    long bytesRead 
      = ov_read(&vorbis_file, buffer, static_cast<int> (buffer_size), 0, 2, 1,
          &section);
    if(bytesRead==0){
      break;
    }
    buffer_size -= bytesRead;
    buffer += bytesRead;
    totalBytesRead += bytesRead;
  }
  
  return totalBytesRead;
}

void
OggSoundFile::reset()
{
  ov_raw_seek(&vorbis_file, 0);
}

size_t
OggSoundFile::cb_read(void* ptr, size_t size, size_t nmemb, void* source)
{
  PHYSFS_file* file = reinterpret_cast<PHYSFS_file*> (source);
  
  PHYSFS_sint64 res 
    = PHYSFS_read(file, ptr, static_cast<PHYSFS_uint32> (size),
        static_cast<PHYSFS_uint32> (nmemb));
  if(res <= 0)
    return 0;

  return static_cast<size_t> (res);
}

int
OggSoundFile::cb_seek(void* source, ogg_int64_t offset, int whence)
{
  PHYSFS_file* file = reinterpret_cast<PHYSFS_file*> (source);

  switch(whence) {
    case SEEK_SET:
      if(PHYSFS_seek(file, static_cast<PHYSFS_uint64> (offset)) == 0)
        return -1;
      break;
    case SEEK_CUR:
      if(PHYSFS_seek(file, PHYSFS_tell(file) + offset) == 0)
        return -1;
      break;
    case SEEK_END:
      if(PHYSFS_seek(file, PHYSFS_fileLength(file) + offset) == 0)
        return -1;
      break;
    default:
#ifdef DEBUG
      assert(false);
#else
      return -1;
#endif
  }
  return 0;
}
  
int
OggSoundFile::cb_close(void* source)
{
  PHYSFS_file* file = reinterpret_cast<PHYSFS_file*> (source);
  PHYSFS_close(file);
  return 0;
}

long
OggSoundFile::cb_tell(void* source)
{
  PHYSFS_file* file = reinterpret_cast<PHYSFS_file*> (source);
  return static_cast<long> (PHYSFS_tell(file));
}

//---------------------------------------------------------------------------

#include <fstream>
SoundFile* load_sound_file(const std::string& filename)
{
  PHYSFS_file* file = PHYSFS_openRead(filename.c_str());
  if(!file) {
    std::stringstream msg;
    msg << "Couldn't open '" << filename << "': " << PHYSFS_getLastError();
    throw std::runtime_error(msg.str());
  }
    
  try {
    char magic[4];
    if(PHYSFS_read(file, magic, sizeof(magic), 1) != 1)
      throw std::runtime_error("Couldn't read magic, file too short");
    PHYSFS_seek(file, 0);
    if(strncmp(magic, "RIFF", 4) == 0)
      return new WavSoundFile(file);
    else if(strncmp(magic, "OggS", 4) == 0)
      return new OggSoundFile(file);
    else
      throw std::runtime_error("Unknown file format");
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Couldn't read '" << filename << "': " << e.what();
    throw std::runtime_error(msg.str());
  }
}

