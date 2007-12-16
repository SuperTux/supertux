#ifndef __CREATE_DOCU_H__
#define __CREATE_DOCU_H__

#include "tree.hpp"
#include "xmlwriter.hpp"

class DocuCreator
{
public:
    const char* ind;
    std::ostream& out;
    XmlWriter writer;

    DocuCreator(std::ostream& _out = std::cout)
        : out(_out), writer(out)
    {
        ind = "  ";
    }

    void create_docu(Namespace* ns);
    void create_class_docu(Class* _class);
    void create_function_docu(Class* _class, Function* function);
    std::string get_type(const Type& type);
};

#endif
