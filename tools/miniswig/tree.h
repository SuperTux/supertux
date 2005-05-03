#ifndef __TREE_H__
#define __TREE_H__

#include <vector>
#include <string>
#include <iostream>

class AtomicType {
public:
    std::string name;
    virtual ~AtomicType()
    { }

    virtual void write_c(std::ostream& out)
    {
        out << name;
    }
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
        : atomic_type(0), _const(false), _static(false), pointer(0), ref(0)
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
        if(atomic_type == &BasicType::VOID && pointer == 0)
            return true;
        return false;
    }

    AtomicType* atomic_type;
    bool _const;
    bool _static;
    // number of '*' in the type declaration...
    int pointer;
    // number of '&' in the type declaration...
    int ref;
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

class Parameter {
public:
    std::string name;
    Type type;
};

class ClassMember {
public:
    virtual ~ClassMember()
    { }

    enum Visbility {
        PUBLIC,
        PROTECTED,
        PRIVATE
    };
    Visbility visibility;
};

class Function : public ClassMember {
public:
    enum FuncType {
        FUNCTION,
        CONSTRUCTOR,
        DESTRUCTOR
    };
    FuncType type;
    std::string name;
    Type return_type;
    std::vector<Parameter> parameters;
};

class Class : public AtomicType {
public:
    ~Class() {
        for(std::vector<ClassMember*>::iterator i = members.begin();
                i != members.end(); ++i)
            delete *i;
    }
    
    std::vector<ClassMember*> members;
};

class Namespace {
public:
    std::string name;
};

class CompilationUnit {
public:
    ~CompilationUnit() {
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
 
    std::vector<Function*> functions;
    std::vector<AtomicType*> types;
    std::vector<Namespace*> namespaces;
};

#endif

