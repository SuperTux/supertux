//#include <config.h>

#include "tree.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "create_docu.hpp"
#include "globals.hpp"

void
DocuCreator::create_docu(Namespace* ns)
{
    writer.openTag("documentation");
    writer.openTag("namespace");
    writer.writeAttribute("name", ns->name);

    for(std::vector<AtomicType*>::iterator i = ns->types.begin();
            i != ns->types.end(); ++i) {
        AtomicType* type = *i;
        Class* _class = dynamic_cast<Class*> (type);
        if(_class != 0)
            create_class_docu(_class);
    }
    for(std::vector<Function*>::iterator i = ns->functions.begin();
            i != ns->functions.end(); ++i) {
        create_function_docu(0, *i);
    }

    writer.closeTag("namespace");
    writer.closeTag("documentation");
}

void
DocuCreator::create_class_docu(Class* _class)
{
    writer.openTag("class");
    writer.writeAttribute("name", _class->name);

    if(_class->docu_comment != "") {
        writer.writeTag("documentation");
        writer.write(_class->docu_comment);
    }

    for(std::vector<ClassMember*>::iterator i = _class->members.begin();
            i != _class->members.end(); ++i) {
        ClassMember* member = *i;
        if(member->visibility != ClassMember::PUBLIC)
            continue;
        Function* function = dynamic_cast<Function*> (member);
        if(!function)
            continue;
        if(function->type != Function::FUNCTION)
            continue;
        create_function_docu(_class, function);
    }

    writer.closeTag("class");
}

void
DocuCreator::create_function_docu(Class* _class, Function* function)
{
    writer.openTag("function");

    writer.writeAttribute("return_type",
            get_type(function->return_type));
    writer.writeAttribute("name", function->name);

    if(function->docu_comment != "") {
        writer.writeTag("documentation");
        writer.write(function->docu_comment);
    }

    for(std::vector<Parameter>::iterator p = function->parameters.begin();
            p != function->parameters.end(); ++p) {
        if(p == function->parameters.begin()
                && p->type.atomic_type == HSQUIRRELVMType::instance())
            continue;

        writer.writeTag("param");
        writer.writeAttribute("type", get_type(p->type));
        writer.writeAttribute("name", p->name);
    }

    writer.closeTag("function");
}

std::string
DocuCreator::get_type(const Type& type)
{
    if(type.ref > 0 && type.atomic_type != StringType::instance())
        throw std::runtime_error("References not handled yet");
    if(type.pointer > 0)
        throw std::runtime_error("Pointers not handled yet");
    if(type.atomic_type == &BasicType::VOID) {
        return "void";
    } else if(type.atomic_type == &BasicType::INT) {
        return "int";
    } else if(type.atomic_type == &BasicType::FLOAT) {
        return "float";
    } else if(type.atomic_type == &BasicType::BOOL) {
        return "bool";
    } else if(type.atomic_type == StringType::instance()) {
        return "string";
    }

    std::ostringstream msg;
    msg << "Type '" << type.atomic_type->name << "' not supported yet.";
    throw std::runtime_error(msg.str());
}
