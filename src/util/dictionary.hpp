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

#ifndef HEADER_SUPERTUX_UTIL_DICTIONARY_HPP
#define HEADER_SUPERTUX_UTIL_DICTIONARY_HPP
#include <squirrel.h>
#include <vector>
#include <map>
#include <boost/any.hpp>
#include "util/log.hpp"
#include "util/writer.hpp"
#include <memory>
#include "util/sqarrayser.hpp"
#include "util/sqdictser.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/reader_collection.hpp"

class sqdict;
class sqarr;

using std::auto_ptr;

enum dictionaryTypes {
    DICT_STRING = 0, DICT_FLOAT, DICT_INT,DICT_BOOL, DICT_ARR, DICT_TABLE
};


class dictionaryItem {
  public:
    std::string key;
    dictionaryTypes type;
    std::unique_ptr<boost::any> item;
    std::string value;
    std::unique_ptr<sqarr> valarr;
    std::unique_ptr<sqdict> valdict;
    dictionaryItem():
        key(),
        type(),
        item(),
        value(),
        valarr(),
        valdict() {
    };
};


class dictionary {
  private:
    std::map<std::string,std::shared_ptr<dictionaryItem>> m_table; // Hashtable with variable size
    dictionaryItem* add(const std::string& key,std::unique_ptr<boost::any> value);
    dictionaryItem* createTableEntry(const std::string& key);
  public:
    bool save();
    void add(const std::string& key,int value);
    void add(const std::string& key,std::string value);
    void add(const std::string& key,float value);
    void add(const std::string& key,bool value);
    void add(const std::string& key,std::unique_ptr<sqarr> value);
    void add(const std::string& key,std::unique_ptr<sqdict> value);
    // Methods for squirrel specific types (like lists, etc)
    void setFilename(const std::string& filename);
    std::string getFilename(){
      return m_filename;
    };
    dictionaryItem* get(const std::string& key);
    dictionary(const std::string& filename);
    dictionary(void); // Initialize empty
    static dictionaryTypes str2type(const std::string& type);

  private:
    static std::string getTypename(dictionaryItem* i);
    std::string m_filename;
    std::unique_ptr<Writer> m_writer;
    void saveItems();
    void saveItem(const dictionaryItem* item);
    void load();


};

#endif
