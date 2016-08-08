//  SuperTux
//  Copyright (C) 2016 Christian Hagemeier <christian@hagemeier.ch>
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

/**
 *  Wrapper for squirrel arrays
 * This class is used to store squirrel arrays, when they are stored.
 *
 */
#ifndef HEADER_SUPERTUX_UTIL_SQARRAYSER_HPP
#define HEADER_SUPERTUX_UTIL_SQARRAYSER_HPP
#include <squirrel.h>
#include <vector>
#include <boost/any.hpp>
#include "util/log.hpp"
#include "util/writer.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <boost/shared_ptr.hpp>
#include "util/sqdictser.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/reader_collection.hpp"

class sqarr;
class sqdict;

class sqarritem {
  public:
    int index;

    SQObjectType type;
    std::unique_ptr<boost::any> item;
    char* string; // If string is used
    std::unique_ptr<sqdict> dict;
    std::unique_ptr<sqarr> array;
    explicit sqarritem():
        index(),
        type(),
        item(),
        string(NULL),
        dict(),
        array() {
    };
    sqarritem(const sqarritem&) = delete;
    sqarritem& operator=(const sqarritem&) = delete;
    ~sqarritem() {
        if(string != NULL) {
            free(string);
        }
    }
};

class sqarr {
  public:
    void loadArray(HSQUIRRELVM vm);
    void serialize(Writer* serializer) const;
    void get(HSQUIRRELVM vm) const;
    void load(const ReaderCollection& r);
    explicit sqarr():
        m_elements() {
    }
    std::vector<std::shared_ptr<sqarritem>> m_elements;
};

#endif
