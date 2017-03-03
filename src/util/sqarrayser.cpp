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

#include "util/sqarrayser.hpp"
using boost::any;
using boost::any_cast;
using std::unique_ptr;

void
sqarr::loadArray(HSQUIRRELVM vm) {
    // Iterate over the array in the vm
    // first push 0 onto the stack
    sq_pushnull(vm);
    // then begin the iteration
    while(SQ_SUCCEEDED(sq_next(vm,-2))) {
        // Pop key
        if(sq_gettype(vm, -2) != OT_INTEGER) {
            log_warning << "Array contains non int key\n";
            continue;
        }
        // Pop value
        SQInteger key;
        sq_getinteger(vm, -2, &key);
        std::shared_ptr<sqarritem> newitem(new sqarritem());
        newitem->type = sq_gettype(vm, -1);
        newitem->index = key;
        switch(sq_gettype(vm, -1)) {
        case OT_INTEGER: {
            SQInteger vali;
            sq_getinteger(vm, -1, &vali);
            std::unique_ptr<any> intp(new any((int) vali));
            newitem->item = std::move(intp);
            break;
        }
        case OT_BOOL: {
            SQBool valb;
            sq_getbool(vm, -1, &valb);
            std::unique_ptr<any> boolp(new any((bool)(valb == SQTrue)));
            newitem->item = std::move(boolp);
            break;
        }
        case OT_STRING: {
            const SQChar* str;
            sq_getstring(vm, -1, &str);
            newitem->string = (char*) malloc(sizeof(char)*(strlen(str)+1));
            strcpy(newitem->string,str);
        }
        case OT_FLOAT: {
            SQFloat valf;
            sq_getfloat(vm, -1, &valf);
            std::unique_ptr<any> valfp(new any((float) (valf)));
            newitem->item = std::move(valfp);
            break;
        }
        case OT_TABLE: {
            std::unique_ptr<sqdict> sqi(new sqdict());
            sqi->loadDict(vm);
            newitem->dict = std::move(sqi);
            break;
        }
        case OT_ARRAY: {
            std::unique_ptr<sqarr> sqi(new sqarr());
            sqi->loadArray(vm);
            newitem->array =  std::move(sqi);
            break;
        }
        default: {
            log_warning << "Array item can't be serialized!" << std::endl;
            break;
        }
        }
        m_elements.push_back(std::move(newitem));
        // Pop from stack
        sq_pop(vm,2);
    }
    // pop the iterator
    sq_pop(vm,1);
}

void
sqarr::serialize(Writer* serializer) const {
    // Method used for serializing an array
    serializer->start_list("array-items");
    for(const auto& item:m_elements) {
        serializer->start_list("item");
        serializer->write("key",item->index);
        switch(item->type) {
        case OT_INTEGER: {
            try {
                int vali =  *any_cast<int>(item->item.get());
                serializer->write("value",vali);
                serializer->write("type","int");
                break;
            } catch ( const std::exception& e ) {
                log_warning << e.what() << std::endl;
            }
            break;
        }
        case OT_BOOL: {
            try {
                bool valb =  any_cast<bool>(item->item.get());
                serializer->write("value",valb);
                serializer->write("type","bool");
                break;
            } catch ( const std::exception& e ) {
                log_warning << e.what() << std::endl;
            }
            break;
        }
        case OT_STRING: {
            try {
                serializer->write("value",item->string);
                serializer->write("type","str");
            } catch ( const std::exception& e ) {
                log_warning << e.what() << std::endl;
            }
            break;
        }
        case OT_FLOAT: {
            try {
                float vali =  *any_cast<float>(item->item.get());
                serializer->write("value",vali);
                serializer->write("type","float");
                break;
            } catch ( const std::exception& e ) {
                log_warning << e.what() << std::endl;
            }
        }
        case OT_TABLE: {
            sqdict val =  *item->dict;
            serializer->write("type","table");
            val.serialize(serializer);
            break;
        }
        case OT_ARRAY: {
            sqarr val =  *item->array;
            serializer->write("type","arr");
            val.serialize(serializer);
            break;
        }
        default: {
            log_warning << "Array item can't be serialized!" << std::endl;
            break;
        }
        }
        serializer->end_list("item");
    }
    serializer->end_list("array-items");
}

// Pushes array onto the vm stack
void
sqarr::get(HSQUIRRELVM vm) const {
    log_debug << "Creating array" << std::endl;
    // Push array to squirrel
    sq_newarray (vm,0);
    // Array is now on the stack
    for(const auto& item:m_elements) {
        // Get type and call the appropriate get method
        switch(item->type) {
        case OT_INTEGER: {
            int val = *any_cast<int>(item->item.get());
            log_debug << "Pushing " << val << std::endl;
            sq_pushinteger(vm,val);
            sq_arrayappend(vm, -2);
            log_debug << "Pushed " << val << std::endl;
            break;
        }
        case OT_BOOL: {
            bool val = *any_cast<bool>(item->item.get());
            sq_pushbool(vm,val);
            sq_arrayappend(vm, -2);
            break;
        }
        case OT_STRING: {

            const SQChar* val = item->string;
            // -1 indicates, that the length will  be calculated by the vm
            sq_pushstring(vm,val,-1);
            sq_arrayappend(vm, -2);
            break;
        }
        case OT_FLOAT: {
            float val = *any_cast<float>(item->item.get());
            sq_pushinteger(vm,val);
            sq_arrayappend(vm, -2);
            break;
        }
        case OT_ARRAY: {
            sqarr val =  *item->array;
            log_debug << val.m_elements.size();
            val.get(vm);

            sq_arrayappend(vm, -2);

            break;
        }
        case OT_TABLE: {
            sqdict val = *item->dict;
            val.get(vm);
            sq_arrayappend(vm, -2);
            break;
        }
        default: {
            throw scripting::SquirrelError(vm, "Type not serializable");
        }

        }
    }
}

void
sqarr::load(const ReaderCollection& r) {
    for(const auto& item:r.get_objects()) {
        // Check that item title is "item"
        if(!(item.get_name() == "item")) {
            log_fatal << "Incorrect storage save data." << std::endl;
            return;
        }
        ReaderMapping imap = item.get_mapping();
        // Get key
        int key;
        imap.get("key",key);
        // Get type
        std::string typen;
        imap.get("type",typen);
        auto type = dictionary::str2type(typen);
        std::shared_ptr<sqarritem> itemp(new sqarritem());
        itemp->index = key;
        log_debug << "Hello" << std::endl;

        switch (type) {
        case DICT_INT: {
            int val;
            imap.get("value",val);
            std::unique_ptr<any> container(new any(val));
            itemp->item = std::move(container);
            itemp->type = OT_INTEGER;
            m_elements.push_back(itemp);
            break;
        }
        case DICT_STRING: {
            log_debug << "Adding string item" << std::endl;
            std::string val = "yeah";
            imap.get("value",val);
            char* str = (char*) malloc(sizeof(char)*(val.size()+1));
            str[val.size()+1] = '\0';
            strcpy(str,val.c_str());
            itemp->string = str;
            itemp->type = OT_STRING;
            m_elements.push_back(itemp);
            break;
        }
        case DICT_BOOL: {
            bool val;
            imap.get("value",val);
            std::unique_ptr<any> container(new any( (bool) val));
            itemp->item = std::move(container);
            itemp->type = OT_BOOL;
            m_elements.push_back(itemp);
            break;
        }
        case DICT_FLOAT: {
            float val;
            imap.get("value",val);
            std::unique_ptr<any> container(new any( (float) val));
            itemp->item = std::move(container);
            itemp->type = OT_FLOAT;
            m_elements.push_back(itemp);
            break;
        }
        case DICT_TABLE: {
            log_debug << "Adding table" << std::endl;
            std::unique_ptr<sqdict> table(new sqdict());
            ReaderCollection items;
            imap.get("dict-items",items);
            log_debug << "Loading objects" << std::endl;
            table->load(items);
            itemp->dict = std::move(table);
            itemp->type = OT_TABLE;
            m_elements.push_back(itemp);
            break;
        }
        case DICT_ARR: {
            std::unique_ptr<sqarr> arrp(new sqarr());
            ReaderCollection items;
            imap.get("array-items",items);
            arrp->load(items);
            itemp->array = std::move(arrp);
            itemp->type = OT_ARRAY;
            m_elements.push_back(itemp);
            break;
        }
        }
    }
}
