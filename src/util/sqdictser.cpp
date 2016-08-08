#include "util/sqdictser.hpp"
using boost::any;
using boost::any_cast;

void
sqdict::loadDict(HSQUIRRELVM vm) {
    // Iterate over the array in the vm
    // first push 0 onto the stack
    sq_pushnull(vm);
    // then begin the iteration
    while(SQ_SUCCEEDED(sq_next(vm,-2))) {
        // Pop key
        if(sq_gettype(vm, -2) != OT_STRING) {
            log_warning << "Dict may only contain string keys!\n";
            sq_throwerror(vm, _SC("Dict may only contain string keys!\n"));
            continue;
        }
        // Pop value
        const SQChar* key;
        sq_getstring(vm, -2, &key);
        std::shared_ptr<sqdictitem> newitem(new sqdictitem());
        newitem->type = sq_gettype(vm, -1);
        newitem->key = (char*) malloc(sizeof(char)*(strlen(key)+1));
        strcpy(newitem->key,key);
        newitem->key[strlen(key)] = '\0';
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
            if(SQ_SUCCEEDED(sq_getbool(vm, -1, &valb))) {
                std::unique_ptr<any> boolp(new any(valb == SQTrue));
                newitem->item = std::move(boolp);
            }
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
            std::unique_ptr<any> floatp(new any(static_cast<float> (valf)));
            newitem->item = std::move(floatp);
            break;
        }
        case OT_TABLE: {
            std::unique_ptr<sqdict> sqi(new sqdict());
            log_warning << "Recload "<< std::endl;
            sqi->loadDict(vm);
            newitem->dict =  std::move(sqi);
            break;
        }
        case OT_ARRAY: {
            std::unique_ptr<sqarr> sqi(new sqarr());
            log_warning << "Recload "<< std::endl;
            sqi->loadArray(vm);
            newitem->arr =  std::move(sqi);
            break;
        }
        default:
            log_warning << "Array item can't be serialized!" << std::endl;
            break;
        }
        m_elements.push_back(newitem);
        // Pop from stack
        sq_pop(vm,2);
    }
    // pop the iterator
    sq_pop(vm,1);
}

void
sqdict::serialize(Writer* serializer) const {
    // Method used for serializing an array
    serializer->start_list("dict-items");
    for(const auto& item:m_elements) {
        //continue;
        serializer->start_list("item");
        serializer->write("key",item->key);
        switch(item->type) {
        case OT_INTEGER: {
            try {
                const int vali =  *any_cast<int>(item->item.get());
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
                const bool valb =  any_cast<bool>(item->item.get());
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
                const int vali =  *any_cast<float>(item->item.get());
                serializer->write("value",vali);
                serializer->write("type","float");
                break;
            } catch ( const std::exception& e ) {
                log_warning << e.what() << std::endl;
            }
        }
        case OT_TABLE: {
            auto val =  *item->dict;
            serializer->write("type","table");
            val.serialize(serializer);
            break;
        }
        case OT_ARRAY: {
            auto val =  *item->arr;
            serializer->write("type","arr");
            val.serialize(serializer);
            break;
        }
        default:
            log_warning << "Array item can't be serialized!" << std::endl;
            break;
        }
        serializer->end_list("item");
    }
    serializer->end_list("dict-items");
}
/**
 *  Method for loading a dictionary (table) stored in memory onto the vm stack
 *
 */
void
sqdict::get(HSQUIRRELVM vm) const {
    // Create the dictionary
    sq_newtable(vm);
    // Create the slots
    for(const auto& elem:m_elements) {
        // Push key with unknown length (-1 is used for indicating strlen call)
        sq_pushstring(vm,elem->key,-1);
        switch(elem->type) {
        case OT_INTEGER: {
            const int val = *any_cast<int>(elem->item.get());
            log_debug << "Pushing " << val << std::endl;
            sq_pushinteger(vm,val);
            break;
        }
        case OT_BOOL: {
            const bool val = *any_cast<bool>(elem->item.get());
            sq_pushbool(vm,val == SQTrue);
            break;
        }
        case OT_STRING: {

            const SQChar* val = elem->string;
            // -1 indicates, that the length will  be calculated by the vm
            sq_pushstring(vm,val,-1);
            break;
        }
        case OT_FLOAT: {
            const int val = *any_cast<float>(elem->item.get());
            sq_pushinteger(vm,val);
            break;
        }
        case OT_ARRAY: {
            auto val =  *elem->arr;
            log_debug << val.m_elements.size();
            val.get(vm);
            break;
        }
        case OT_TABLE: {
            auto val = *elem->dict;
            val.get(vm);
            break;
        }

        default: {
            throw scripting::SquirrelError(vm, "Type not serializable");
        }
        }
        if(SQ_FAILED(sq_createslot(vm, -3)))
            throw scripting::SquirrelError(vm, "Couldn't create new index");
    }
}

void sqdict::load(const ReaderCollection& coll) {
    // Iterate over items
    for(const auto& item:coll.get_objects()) {
        // Check that item title is "item"
        log_debug << "Adding to dict " << std::endl;
        if(!(item.get_name() == "item")) {
            log_fatal << "Incorrect storage save data." << std::endl;
            return;
        }
        ReaderMapping imap = item.get_mapping();
        // Get key
        std::string key;
        imap.get("key",key);
        // Get type
        std::string typen;
        imap.get("type",typen);
        auto type = dictionary::str2type(typen);
        std::shared_ptr<sqdictitem> itemp(new sqdictitem());
        char* tk = (char *) malloc(sizeof(char)*(key.size()+1));
        tk[key.size()+1] = '\0';
        strcpy(tk,key.c_str());
        itemp->key = tk;
        log_debug << "Hello";
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
            std::string val;
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
            std::unique_ptr<sqdict> table(new sqdict());
            ReaderCollection items;
            imap.get("dict-items",items);
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
            itemp->arr = std::move(arrp);
            itemp->type = OT_ARRAY;
            m_elements.push_back(itemp);
            break;
        }
        }
    }
}
