#ifndef __TREE_H__
#define __TREE_H__

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <assert.h>

class Namespace;

class AtomicType {
public:
    AtomicType() :
      name(),
      parent(0)
    { }
    virtual ~AtomicType()
    { }

    virtual void write_c(std::ostream& out)
    {
        out << name;
    }

    std::string name;
    Namespace* parent;

private:
    AtomicType(const AtomicType&);
    AtomicType& operator=(const AtomicType&);
};

class BasicType : public AtomicType {
public:
    static BasicType VOID;
    static BasicType BOOL;
    static BasicType CHAR;
    static BasicType SHORT;
    static BasicType INT;
    static BasicType LONG;
    static BasicType FLOAT;
    static BasicType DOUBLE;

private:
    BasicType(const std::string& name)
    {
        this->name = name;
    }
};

class Type {
public:
    Type()
        : atomic_type(0), _unsigned(false), _const(false), _static(false),
        pointer(0), ref(0)
    { }

    void write_c_type(std::ostream& out)
    {
        if(_static)
            out << "static ";
        if(_const)
            out << "const ";
        atomic_type->write_c(out);
        for(int i = 0; i < pointer; ++i)
            out << "*";
        for(int i = 0; i < ref; ++i)
            out << "&";
    }

    bool is_void() const
    {
        if(atomic_type == 0)
            return true;
        if(atomic_type == &BasicType::VOID && pointer == 0)
            return true;
        return false;
    }

    AtomicType* atomic_type;
    bool _unsigned;
    bool _const;
    bool _static;
    // number of '*' in the type declaration...
    int pointer;
    // number of '&' in the type declaration...
    int ref;
};

class SQIntegerType : public AtomicType {
public:
    SQIntegerType()
    {
        this->name = "SQInteger";
        assert(_instance == 0);
        _instance = this;
    }
    virtual ~SQIntegerType()
    {
        assert(_instance == this);
        _instance = NULL;
    }

    static SQIntegerType* instance()
    {
        return _instance;
    }
private:
    static SQIntegerType* _instance;
};

class HSQUIRRELVMType : public AtomicType {
public:
    HSQUIRRELVMType()
    {
        this->name = "HSQUIRRELVM";
        assert(_instance == 0);
        _instance = this;
    }
    virtual ~HSQUIRRELVMType()
    {
        assert(_instance == this);
        _instance = NULL;
    }

    static HSQUIRRELVMType* instance()
    {
        return _instance;
    }
private:
    static HSQUIRRELVMType* _instance;
};

class StringType : public AtomicType {
public:
    StringType()
    {
        this->name = "string";
        assert(_instance == 0);
        _instance = this;
    }
    virtual ~StringType()
    {
        assert(_instance == this);
        _instance = 0;
    }

    static StringType* instance()
    {
        return _instance;
    }

    virtual void write_c(std::ostream& out)
    {
        out << "std::string";
    }

private:
    static StringType* _instance;
};

class Parameter
{
public:
    Parameter() :
        name(),
        type()
    { }

    std::string name;
    Type type;
};

class ClassMember {
public:
    ClassMember() :
        visibility()
    { }
    virtual ~ClassMember()
    { }

    enum Visibility {
        PUBLIC,
        PROTECTED,
        PRIVATE
    };
    Visibility visibility;
};

class Function : public ClassMember {
public:
    Function() :
        type(),
        suspend(),
        custom(),
        parameter_spec(),
        docu_comment(),
        name(),
        return_type(),
        parameters()
    {
        type = FUNCTION;
        suspend = false;
        custom = false;
    }

    enum FuncType {
        FUNCTION,
        CONSTRUCTOR,
        DESTRUCTOR
    };
    FuncType type;
    /// function should suspend squirrel VM after execution
    bool suspend;
    /// a custom wrapper (just pass along HSQUIRRELVM)
    bool custom;
    std::string parameter_spec;
    std::string docu_comment;
    std::string name;
    Type return_type;
    std::vector<Parameter> parameters;
};

class Field : public ClassMember {
public:
    Field() :
        type(),
        docu_comment(),
        name(),
        has_const_value(),
        const_float_value(),
        const_int_value(),
        const_string_value()
    {
        has_const_value = false;
    }

    Type* type;
    std::string docu_comment;
    std::string name;
    bool has_const_value;

    float const_float_value;
    int const_int_value;
    std::string const_string_value;

private:
    Field(const Field&);
    Field& operator=(const Field&);
};

class Class : public AtomicType {
public:
    Class() :
        members(),
        super_classes(),
        sub_classes(),
        docu_comment()
    { }
    ~Class() {
        for(std::vector<ClassMember*>::iterator i = members.begin(); i != members.end(); ++i)
            delete *i;
    }

    std::vector<ClassMember*> members;
    std::vector<Class*> super_classes;
    std::vector<Class*> sub_classes;
    std::string docu_comment;
};

class Namespace {
public:
    Namespace() :
        functions(),
        fields(),
        types(),
        namespaces(),
        parent(),
        name()
    {
        parent = 0;
    }
    virtual ~Namespace() {
        for(std::vector<Function*>::iterator i = functions.begin();
                i != functions.end(); ++i)
            delete *i;
        for(std::vector<AtomicType*>::iterator i = types.begin();
                i != types.end(); ++i)
            delete *i;
        for(std::vector<Namespace*>::iterator i = namespaces.begin();
                i != namespaces.end(); ++i)
            delete *i;
    }
    void add_type(AtomicType* type)
    {
        types.push_back(type);
        type->parent = this;
    }
    void add_namespace(Namespace* ns)
    {
        namespaces.push_back(ns);
        ns->parent = this;
    }
    AtomicType* _findType(const std::string& name, bool godown = false) {
        for(std::vector<AtomicType*>::iterator i = types.begin();
                i != types.end(); ++i) {
            AtomicType* type = *i;
            if(type->name == name)
                return type;
        }
        if(godown && parent)
            return parent->_findType(name, true);

        return 0;
    }

    Namespace* _findNamespace(const std::string& name, bool godown = false) {
        for(std::vector<Namespace*>::iterator i = namespaces.begin();
                i != namespaces.end(); ++i) {
            Namespace* ns = *i;
            if(ns->name == name)
                return ns;
        }
        if(godown && parent)
            return parent->_findNamespace(name, true);

        return 0;
    }

    Namespace* findNamespace(const std::string& name, bool godown = false) {
        Namespace* ret = _findNamespace(name, godown);
        if(!ret) {
            std::ostringstream msg;
            msg << "Couldn't find namespace '" << name << "'.";
            throw std::runtime_error(msg.str());
        }

        return ret;
    }

    std::vector<Function*> functions;
    std::vector<Field*> fields;
    std::vector<AtomicType*> types;
    std::vector<Namespace*> namespaces;

    Namespace* parent;
    std::string name;

private:
    Namespace(const Namespace&);
    Namespace& operator=(const Namespace&);
};

class CompilationUnit : public Namespace {
public:
};

#endif
