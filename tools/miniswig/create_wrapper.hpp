#ifndef __CREATE_WRAPPER_H__
#define __CREATE_WRAPPER_H__

#include "tree.hpp"

class WrapperCreator
{
public:
    /// this is used for indentation
    const char* ind;
    // output stream
    std::ostream& out;
    std::ostream& hppout;

    WrapperCreator(std::ostream& _out = std::cout, std::ostream& _hppout = std::cout) :
        ind("  "),
        out(_out),
        hppout(_hppout),
        ns_prefix()
    { }

    void create_wrapper(Namespace* ns);

private:
    std::string ns_prefix;

    void create_register_functions_code(Namespace* ns);
    void create_register_function_code(Function* function, Class* _class);
    void create_register_classes_code(Namespace* ns);
    void create_register_class_code(Class* _class);
    void create_register_constant_code(Field* field);
    void create_register_constants_code(Namespace* ns);
    void create_register_slot_code(const std::string& what,
                                   const std::string& name);

    void create_function_list(Namespace* ns);
    void create_const_lists(Namespace* ns);
    void create_class_const_lists(Class* _class);
    void create_class_wrapper(Class* _class);
    void create_class_release_hook(Class* _class);
    void create_squirrel_instance(Class* _class);
    void create_function_wrapper(Class* _class, Function* function);
    void prepare_argument(const Type& type, size_t idx, const std::string& var);
    void push_to_stack(const Type& type, const std::string& var);

private:
    WrapperCreator(const WrapperCreator&);
    WrapperCreator& operator=(const WrapperCreator&);
};

#endif
