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
#include "util/dictionary.hpp"
#include <iostream>

using std::vector;
using std::cout;
using namespace boost;
using std::auto_ptr;

dictionary::dictionary(const std::string& filename):
    m_table(),
    m_filename(filename),
    m_writer() {
    this->load();
}

dictionary::dictionary(void):
    m_table(),
    m_filename(),
    m_writer() {
}

dictionaryTypes dictionary::str2type(const std::string& type) {
    if(type == "int")
        return DICT_INT;
    if(type == "arr")
        return DICT_ARR;
    if(type == "bool")
        return DICT_BOOL;
    if(type == "float")
        return DICT_FLOAT;
    if(type == "table")
        return DICT_TABLE;
    if(type == "str")
        return DICT_STRING;
    throw new std::runtime_error("No valid type string provided! (Invalid supertux storage file)");
}

void dictionary::load() {
    try {
        if(!PHYSFS_exists(m_filename.c_str())) {
            log_info << m_filename << " doesn't exist, not loading world storage" << std::endl;
        } else {
            PHYSFS_Stat statbuf;
            PHYSFS_stat(m_filename.c_str(), &statbuf);
            if(statbuf.filetype == PHYSFS_FILETYPE_DIRECTORY) {
                log_info << m_filename << " is a directory, not loading world storage" << std::endl;
                return;
            }
            auto doc = ReaderDocument::parse(m_filename);
            auto docroot = doc.get_root();

            if(!(docroot.get_name() == "save-data")) {
                log_fatal << "file is not a supertux-storage file" << std::endl;
                throw std::runtime_error("file is not a supertux-storage file");
            } else {
                auto mapping = docroot.get_mapping();
                ReaderCollection items;
                if(!mapping.get("items",items)) {
                    log_fatal << "Ill formed save-data file";
                    throw std::runtime_error("Ill formed save-data file.");
                }
                for(const auto& item:items.get_objects()) {
                    log_debug << "Next item" << std::endl;
                    if(!(item.get_name() == "item")) {
                        log_fatal << "Ill formed save-data file (Exspected Item in Items)).";
                        throw std::runtime_error("Ill formed save-data file (Exspected Item in Items)).");
                    }
                    auto imap = item.get_mapping();
                    // Get type
                    std::string type;
                    imap.get("type",type);
                    log_debug << "Loading key" << std::endl;
                    std::string key;
                    imap.get("key",key);
                    log_debug << "Got key" << key << " " << std::endl;

                    switch(dictionary::str2type(type)) {
                    case DICT_STRING: {
                        std::string value;
                        imap.get("value",value);
                        add(key,value);
                        break;
                    }
                    case DICT_INT: {
                        int val;
                        imap.get("value",val);
                        add(key,val);
                        break;
                    }
                    case DICT_FLOAT: {
                        float val;
                        imap.get("value",val);
                        add(key,val);
                        break;
                    }
                    case DICT_BOOL: {
                        bool val;
                        imap.get("value",val);
                        add(key,val);
                        break;
                    }
                    case DICT_ARR: {
                        std::unique_ptr<sqarr> arr(new sqarr());
                        ReaderCollection itemsarr;
                        imap.get("array-items",itemsarr);
                        arr->load(itemsarr);
                        add(key,std::move(arr));
                        break;
                    }
                    case DICT_TABLE: {
                      log_debug << "Loading table" << std::endl;
                        ReaderCollection itemsdict;
                        imap.get("dict-items",itemsdict);
                        std::unique_ptr<sqdict> table(new sqdict());
                        table->load(itemsdict);
                        add(key,std::move(table));
                        break;
                    }
                    }
                }
            }
        }
    } catch(const std::exception& e) {
        log_fatal << "Couldn't load storage: " << e.what() << std::endl;
    }
}

void
dictionary::setFilename(const std::string& filename) {
    m_filename = filename;
    this->load();
}

dictionaryItem*
dictionary::createTableEntry(const std::string& key) {
    // TODO Replace in 2017 with insert_or_assign (C++17)
    std::shared_ptr<dictionaryItem> foundn(new dictionaryItem());
    foundn->key = key;
    m_table[key] = foundn;
    return foundn.get();
}

dictionaryItem*
dictionary::add(const std::string& key,std::unique_ptr<boost::any> value) {
    auto found = this->createTableEntry(key);
    found->item = std::move(value);
    return found;
}

void
dictionary::add(const std::string& key,int value) {
    // Add to dictionary
    std::unique_ptr<boost::any> itemp(new boost::any(value));
    auto it = add(key, std::move(itemp));
    it->type = DICT_INT;
}

void
dictionary::add(const std::string& key,std::string value) {
    // Add to dictionary
    auto it = this->createTableEntry(key);
    it->value = value;
    it->type = DICT_STRING;
    log_debug << "Added " << key << std::endl;
}

void
dictionary::add(const std::string& key,float value) {
    // Add to dictionary
    std::unique_ptr<boost::any> itemp(new boost::any(value));
    auto it = add(key, std::move(itemp));
    log_debug << "Added";
    it->type = DICT_FLOAT;
}

void
dictionary::add(const std::string& key,std::unique_ptr<sqarr> value) {
    // Add to dictionary
    auto it = this->createTableEntry(key);
    it->valarr = std::move(value);
    log_debug << "Added " << key << std::endl;
    it->type = DICT_ARR;
}

void
dictionary::add(const std::string& key,std::unique_ptr<sqdict> value) {
    // Add to dictionary
    auto it = this->createTableEntry(key);
    it->valdict = std::move(value);
    it->type = DICT_TABLE;
    log_debug << "Added " << key << std::endl;
}

void
dictionary::add(const std::string& key,bool value) {
    // Add to dictionary
    std::unique_ptr<boost::any> itemp(new boost::any(value));
    auto it = add(key, std::move(itemp));
    it->type = DICT_BOOL;
}

dictionaryItem*
dictionary::get(const std::string& key) {
    auto ptr = m_table[key];
    if(ptr) {
        return ptr.get();
    } else {
        return NULL;
    }
}
/**
 *  Function is called, when a game_session is ended
 */
bool
dictionary::save() {
    log_warning << "Saving " << m_filename << " size:" << m_table.size() << std::endl;
    // open writer
    std::unique_ptr<Writer> writer(new Writer(m_filename));
    m_writer = std::move(writer);
    m_writer->start_list("save-data");
    m_writer->start_list("items");
    m_writer->write_comment("Supertux Simple Storage File");
    this->saveItems();
    m_writer->end_list("items");
    m_writer->end_list("save-data");
    // close writer
    return true;
}

void dictionary::saveItems() {
    // Iterate through hashtable
    for(auto const & pair:m_table) {
        auto item = pair.second;
        log_warning << item->key << std::endl;
        m_writer->start_list("item");
        m_writer->write("key",item->key);
        this->saveItem(item.get());
        m_writer->write("type",dictionary::getTypename(item.get()));
        m_writer->end_list("item");
    }
}

void dictionary::saveItem(const dictionaryItem* item) {
    if(!(item->item) && item->type != DICT_STRING && item->type != DICT_ARR && item->type != DICT_TABLE) {
        log_warning << "Error: Empty variable" << std::endl;
        //return;
    }
    switch(item->type) {
    case DICT_STRING:
        m_writer->write("value",(item->value));
        break;
    case DICT_BOOL:
        m_writer->write("value",*any_cast<bool>(item->item.get()));
        break;
    case DICT_INT:
        m_writer->write("value",*any_cast<int>(item->item.get()));
        break;
    case DICT_FLOAT:
        m_writer->write("value",*any_cast<float>(item->item.get()));
        break;
    case DICT_ARR:
        item->valarr->serialize(m_writer.get());
        break;
    case DICT_TABLE:
        item->valdict->serialize(m_writer.get());
        break;
    }
}

std::string dictionary::getTypename(dictionaryItem* i) {
    switch(i->type) {
    case DICT_STRING:
        return "str";
    case DICT_BOOL:
        return "bool";
    case DICT_INT:
        return "int";
    case DICT_FLOAT:
        return "float";
    case DICT_ARR:
        return "arr";
    case DICT_TABLE:
        return "table";
    }
    return "";
}
