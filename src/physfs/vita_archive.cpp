//  SuperTux
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

#ifdef __vita__

#include "physfs/vita_archive.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

#include <psp2/io/fcntl.h>

namespace {

const PHYSFS_uint64 SLOT_SIZE = 64 * 1024;
const int SLOT_COUNT = 128;

struct CacheSlot
{
  std::vector<PHYSFS_uint8> data;
  PHYSFS_uint64 start;
  PHYSFS_uint64 len;
  PHYSFS_uint64 used;
};

struct ArchiveFile
{
  SceUID fd;
  PHYSFS_uint64 size;
  int refs;
  CacheSlot slots[SLOT_COUNT];
  PHYSFS_uint64 tick;
};

struct ArchiveIo
{
  ArchiveFile* file;
  PHYSFS_uint64 pos;
};

PHYSFS_Io* create_io(ArchiveFile* file);

CacheSlot* find_slot(ArchiveFile* file, PHYSFS_uint64 pos)
{
  for (CacheSlot& slot : file->slots)
  {
    if (slot.len > 0 && pos >= slot.start && pos < slot.start + slot.len)
      return &slot;
  }
  return nullptr;
}

CacheSlot* fill_slot(ArchiveFile* file, PHYSFS_uint64 pos)
{
  CacheSlot* oldest = &file->slots[0];
  for (CacheSlot& slot : file->slots)
  {
    if (slot.used < oldest->used)
      oldest = &slot;
  }

  oldest->data.resize(SLOT_SIZE);
  const int got = sceIoPread(file->fd, oldest->data.data(), SLOT_SIZE, pos);
  if (got <= 0)
  {
    oldest->len = 0;
    return nullptr;
  }
  oldest->start = pos;
  oldest->len = got;
  return oldest;
}

PHYSFS_sint64 io_read(PHYSFS_Io* io, void* buf, PHYSFS_uint64 len)
{
  ArchiveIo* data = static_cast<ArchiveIo*>(io->opaque);
  ArchiveFile* file = data->file;
  if (data->pos >= file->size)
    return 0;
  len = std::min(len, file->size - data->pos);

  PHYSFS_uint8* out = static_cast<PHYSFS_uint8*>(buf);
  PHYSFS_uint64 done = 0;
  while (done < len)
  {
    CacheSlot* slot = find_slot(file, data->pos);
    if (!slot)
    {
      const PHYSFS_uint64 remaining = len - done;
      if (remaining >= SLOT_SIZE)
      {
        const int got = sceIoPread(file->fd, out + done, remaining, data->pos);
        if (got <= 0)
          break;
        done += got;
        data->pos += got;
        continue;
      }

      slot = fill_slot(file, data->pos);
      if (!slot)
        break;
    }

    slot->used = ++file->tick;
    const PHYSFS_uint64 offset = data->pos - slot->start;
    const PHYSFS_uint64 count = std::min(slot->len - offset, len - done);
    memcpy(out + done, slot->data.data() + offset, count);
    done += count;
    data->pos += count;
  }

  if (done == 0 && len > 0)
  {
    PHYSFS_setErrorCode(PHYSFS_ERR_IO);
    return -1;
  }
  return done;
}

PHYSFS_sint64 io_write(PHYSFS_Io*, const void*, PHYSFS_uint64)
{
  PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
  return -1;
}

int io_seek(PHYSFS_Io* io, PHYSFS_uint64 offset)
{
  ArchiveIo* data = static_cast<ArchiveIo*>(io->opaque);
  if (offset > data->file->size)
  {
    PHYSFS_setErrorCode(PHYSFS_ERR_PAST_EOF);
    return 0;
  }
  data->pos = offset;
  return 1;
}

PHYSFS_sint64 io_tell(PHYSFS_Io* io)
{
  return static_cast<ArchiveIo*>(io->opaque)->pos;
}

PHYSFS_sint64 io_length(PHYSFS_Io* io)
{
  return static_cast<ArchiveIo*>(io->opaque)->file->size;
}

PHYSFS_Io* io_duplicate(PHYSFS_Io* io)
{
  ArchiveFile* file = static_cast<ArchiveIo*>(io->opaque)->file;
  file->refs++;
  return create_io(file);
}

int io_flush(PHYSFS_Io*)
{
  return 1;
}

void io_destroy(PHYSFS_Io* io)
{
  ArchiveIo* data = static_cast<ArchiveIo*>(io->opaque);
  if (--data->file->refs == 0)
  {
    sceIoClose(data->file->fd);
    delete data->file;
  }
  delete data;
  delete io;
}

PHYSFS_Io* create_io(ArchiveFile* file)
{
  ArchiveIo* data = new ArchiveIo();
  data->file = file;
  data->pos = 0;

  PHYSFS_Io* io = new PHYSFS_Io();
  io->version = 0;
  io->opaque = data;
  io->read = io_read;
  io->write = io_write;
  io->seek = io_seek;
  io->tell = io_tell;
  io->length = io_length;
  io->duplicate = io_duplicate;
  io->flush = io_flush;
  io->destroy = io_destroy;
  return io;
}

} // namespace

PHYSFS_Io* vita_open_archive_io(const char* path)
{
  const SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
  if (fd < 0)
    return nullptr;

  const SceOff size = sceIoLseek(fd, 0, SCE_SEEK_END);
  if (size <= 0)
  {
    sceIoClose(fd);
    return nullptr;
  }

  ArchiveFile* file = new ArchiveFile();
  file->fd = fd;
  file->size = static_cast<PHYSFS_uint64>(size);
  file->refs = 1;
  file->tick = 0;
  for (CacheSlot& slot : file->slots)
  {
    slot.start = 0;
    slot.len = 0;
    slot.used = 0;
  }
  return create_io(file);
}

#endif
