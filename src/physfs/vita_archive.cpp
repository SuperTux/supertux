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
#include <unordered_map>
#include <vector>

#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>

namespace {

const PHYSFS_uint64 SLOT_SIZE = 128 * 1024;
const int SLOT_COUNT = 256;

struct CacheSlot
{
  std::vector<PHYSFS_uint8> data;
  PHYSFS_uint64 block;
  PHYSFS_uint64 len;
  PHYSFS_uint64 used;
  bool loading;
};

struct ArchiveFile
{
  SceKernelLwMutexWork lock;
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

struct EntryRange
{
  PHYSFS_uint64 start;
  PHYSFS_uint64 end;
};

std::unordered_map<std::string, EntryRange> s_entries;
ArchiveFile* s_archive = nullptr;

PHYSFS_Io* create_io(ArchiveFile* file);

PHYSFS_uint32 read16(const PHYSFS_uint8* p)
{
  return p[0] | (p[1] << 8);
}

PHYSFS_uint32 read32(const PHYSFS_uint8* p)
{
  return p[0] | (p[1] << 8) | (p[2] << 16) | (static_cast<PHYSFS_uint32>(p[3]) << 24);
}

void read_entries(SceUID fd, PHYSFS_uint64 size)
{
  const PHYSFS_uint64 tail_size = std::min<PHYSFS_uint64>(size, 65557);
  std::vector<PHYSFS_uint8> tail(tail_size);
  if (sceIoPread(fd, tail.data(), tail_size, size - tail_size) != static_cast<int>(tail_size))
    return;

  for (PHYSFS_sint64 i = static_cast<PHYSFS_sint64>(tail_size) - 22; i >= 0; --i)
  {
    const PHYSFS_uint8* end = tail.data() + i;
    if (read32(end) != 0x06054b50)
      continue;

    const PHYSFS_uint32 count = read16(end + 10);
    const PHYSFS_uint32 dir_size = read32(end + 12);
    const PHYSFS_uint32 dir_offset = read32(end + 16);
    if (static_cast<PHYSFS_uint64>(dir_offset) + dir_size > size)
      return;

    std::vector<PHYSFS_uint8> dir(dir_size);
    if (sceIoPread(fd, dir.data(), dir_size, dir_offset) != static_cast<int>(dir_size))
      return;

    size_t pos = 0;
    for (PHYSFS_uint32 n = 0; n < count && pos + 46 <= dir.size(); ++n)
    {
      const PHYSFS_uint8* entry = dir.data() + pos;
      if (read32(entry) != 0x02014b50)
        break;
      const PHYSFS_uint32 name_len = read16(entry + 28);
      const PHYSFS_uint32 extra_len = read16(entry + 30);
      const PHYSFS_uint32 comment_len = read16(entry + 32);
      if (pos + 46 + name_len > dir.size())
        break;
      const PHYSFS_uint64 start = read32(entry + 42);
      const PHYSFS_uint64 end = start + 30 + name_len + extra_len + read32(entry + 20) + 64;
      s_entries[std::string(reinterpret_cast<const char*>(entry + 46), name_len)] = { start, std::min(end, size) };
      pos += 46 + name_len + extra_len + comment_len;
    }
    return;
  }
}

CacheSlot* find_slot(ArchiveFile* file, PHYSFS_uint64 block)
{
  for (CacheSlot& slot : file->slots)
  {
    if ((slot.loading || slot.len > 0) && slot.block == block)
      return &slot;
  }
  return nullptr;
}

CacheSlot* oldest_slot(ArchiveFile* file)
{
  CacheSlot* oldest = nullptr;
  for (CacheSlot& slot : file->slots)
  {
    if (!slot.loading && (!oldest || slot.used < oldest->used))
      oldest = &slot;
  }
  return oldest;
}

CacheSlot* load_block(ArchiveFile* file, PHYSFS_uint64 block)
{
  CacheSlot* slot = oldest_slot(file);
  if (!slot)
    return nullptr;
  slot->data.resize(SLOT_SIZE);
  slot->block = block;
  slot->len = 0;
  slot->loading = true;

  sceKernelUnlockLwMutex(&file->lock, 1);
  const int got = sceIoPread(file->fd, slot->data.data(), SLOT_SIZE, block * SLOT_SIZE);
  sceKernelLockLwMutex(&file->lock, 1, nullptr);

  slot->loading = false;
  slot->len = got > 0 ? got : 0;
  slot->used = ++file->tick;
  return got > 0 ? slot : nullptr;
}

PHYSFS_sint64 io_read(PHYSFS_Io* io, void* buf, PHYSFS_uint64 len)
{
  ArchiveIo* data = static_cast<ArchiveIo*>(io->opaque);
  ArchiveFile* file = data->file;
  if (data->pos >= file->size)
    return 0;
  len = std::min(len, file->size - data->pos);

  sceKernelLockLwMutex(&file->lock, 1, nullptr);
  PHYSFS_uint8* out = static_cast<PHYSFS_uint8*>(buf);
  PHYSFS_uint64 done = 0;
  while (done < len)
  {
    const PHYSFS_uint64 block = data->pos / SLOT_SIZE;
    CacheSlot* slot = find_slot(file, block);
    if (slot && slot->loading)
    {
      sceKernelUnlockLwMutex(&file->lock, 1);
      sceKernelDelayThread(100);
      sceKernelLockLwMutex(&file->lock, 1, nullptr);
      continue;
    }

    if (!slot)
    {
      const PHYSFS_uint64 remaining = len - done;
      if (remaining >= SLOT_SIZE)
      {
        sceKernelUnlockLwMutex(&file->lock, 1);
        const int got = sceIoPread(file->fd, out + done, remaining, data->pos);
        sceKernelLockLwMutex(&file->lock, 1, nullptr);
        if (got <= 0)
          break;
        done += got;
        data->pos += got;
        continue;
      }

      slot = load_block(file, block);
      if (!slot)
        break;
    }

    slot->used = ++file->tick;
    const PHYSFS_uint64 offset = data->pos - block * SLOT_SIZE;
    if (offset >= slot->len)
      break;
    const PHYSFS_uint64 count = std::min(slot->len - offset, len - done);
    memcpy(out + done, slot->data.data() + offset, count);
    done += count;
    data->pos += count;
  }
  sceKernelUnlockLwMutex(&file->lock, 1);

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
  sceKernelLockLwMutex(&file->lock, 1, nullptr);
  file->refs++;
  sceKernelUnlockLwMutex(&file->lock, 1);
  return create_io(file);
}

int io_flush(PHYSFS_Io*)
{
  return 1;
}

void io_destroy(PHYSFS_Io* io)
{
  ArchiveIo* data = static_cast<ArchiveIo*>(io->opaque);
  ArchiveFile* file = data->file;
  sceKernelLockLwMutex(&file->lock, 1, nullptr);
  const int refs = --file->refs;
  sceKernelUnlockLwMutex(&file->lock, 1);
  if (refs == 0)
  {
    if (s_archive == file)
      s_archive = nullptr;
    sceIoClose(file->fd);
    sceKernelDeleteLwMutex(&file->lock);
    delete file;
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

  read_entries(fd, static_cast<PHYSFS_uint64>(size));

  ArchiveFile* file = new ArchiveFile();
  sceKernelCreateLwMutex(&file->lock, "archive", 0, 0, nullptr);
  file->fd = fd;
  file->size = static_cast<PHYSFS_uint64>(size);
  file->refs = 1;
  file->tick = 0;
  for (CacheSlot& slot : file->slots)
  {
    slot.block = 0;
    slot.len = 0;
    slot.used = 0;
    slot.loading = false;
  }
  s_archive = file;
  return create_io(file);
}

PHYSFS_sint64
vita_archive_offset(const std::string& filename)
{
  const size_t start = filename.find_first_not_of('/');
  if (start == std::string::npos)
    return -1;
  auto it = s_entries.find(filename.substr(start));
  return it == s_entries.end() ? -1 : static_cast<PHYSFS_sint64>(it->second.start);
}

void
vita_archive_prefetch(const std::string& filename)
{
  ArchiveFile* file = s_archive;
  const size_t start = filename.find_first_not_of('/');
  if (!file || start == std::string::npos)
    return;
  auto it = s_entries.find(filename.substr(start));
  if (it == s_entries.end() || it->second.end <= it->second.start)
    return;

  sceKernelLockLwMutex(&file->lock, 1, nullptr);
  for (PHYSFS_uint64 block = it->second.start / SLOT_SIZE; block <= (it->second.end - 1) / SLOT_SIZE; ++block)
  {
    if (!find_slot(file, block) && !load_block(file, block))
      break;
  }
  sceKernelUnlockLwMutex(&file->lock, 1);
}

#endif
