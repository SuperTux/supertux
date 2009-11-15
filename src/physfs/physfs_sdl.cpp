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

//#include <config.h>

#include "physfs/physfs_sdl.hpp"

#include <physfs.h>

//#include <iostream>
//#include <sstream>
#include <stdexcept>

#include <assert.h>

#include "util/log.hpp"

static int funcSeek(struct SDL_RWops* context, int offset, int whence)
{
    PHYSFS_file* file = (PHYSFS_file*) context->hidden.unknown.data1;
    int res;
    switch(whence) {
        case SEEK_SET:
            res = PHYSFS_seek(file, offset);
            break;
        case SEEK_CUR:
            res = PHYSFS_seek(file, PHYSFS_tell(file) + offset);
            break;
        case SEEK_END:
            res = PHYSFS_seek(file, PHYSFS_fileLength(file) + offset);
            break;
        default:
            res = 0;
            assert(false);
            break;
    }
    if(res == 0) {
        log_warning << "Error seeking in file: " << PHYSFS_getLastError() << std::endl;
        return -1;
    }

    return (int) PHYSFS_tell(file);
}

static int funcRead(struct SDL_RWops* context, void* ptr, int size, int maxnum)
{
    PHYSFS_file* file = (PHYSFS_file*) context->hidden.unknown.data1;

    int res = PHYSFS_read(file, ptr, size, maxnum);
    return res;
}

static int funcClose(struct SDL_RWops* context)
{
    PHYSFS_file* file = (PHYSFS_file*) context->hidden.unknown.data1;

    PHYSFS_close(file);
    delete context;

    return 0;
}

SDL_RWops* get_physfs_SDLRWops(const std::string& filename)
{
    // check this as PHYSFS seems to be buggy and still returns a
    // valid pointer in this case
    if(filename == "") {
        throw std::runtime_error("Couldn't open file: empty filename");
    }

    PHYSFS_file* file = (PHYSFS_file*) PHYSFS_openRead(filename.c_str());
    if(!file) {
        std::stringstream msg;
        msg << "Couldn't open '" << filename << "': "
            << PHYSFS_getLastError();
        throw std::runtime_error(msg.str());
    }

    SDL_RWops* ops = new SDL_RWops();
    ops->type = 0;
    ops->hidden.unknown.data1 = file;
    ops->seek = funcSeek;
    ops->read = funcRead;
    ops->write = 0;
    ops->close = funcClose;
    return ops;
}
