#ifndef HEADER_SUPERTUX_UTIL_SQDICTSER_HPP
#define HEADER_SUPERTUX_UTIL_SQDICTSER_HPP

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
#include "util/sqarrayser.hpp"
#include "scripting/squirrel_error.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/reader_collection.hpp"
#include "util/sqarrayser.hpp"
#include "util/sqdictser.hpp"
#include "util/dictionary.hpp"
class sqdict;
class sqarr;
class sqdictitem {
  public:
    char* key;
    ~sqdictitem() {
        if(string != NULL) {
            free(string);
        }
        if(key != NULL) {
            free(key);
        }
    }
    SQObjectType type;
    std::unique_ptr<boost::any> item;
    char* string; // If string is used
    std::unique_ptr<sqdict> dict;
    std::unique_ptr<sqarr> arr;
    explicit sqdictitem():
        key(),
        type(),
        item(),
        string(NULL),
        dict(),
        arr() {
    };
    sqdictitem(const sqdictitem&) = delete;
    sqdictitem& operator=(const sqdictitem&) = delete;
};

class sqdict {
  public:
    void loadDict(HSQUIRRELVM vm);
    void serialize(Writer* serializer) const;
    void get(HSQUIRRELVM vm) const;
    void load(const ReaderCollection& r);
    explicit sqdict():
        m_elements() {
    }
  private:
    std::vector<std::shared_ptr<sqdictitem>> m_elements;
};
#endif
