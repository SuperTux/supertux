//#include <config.h>
#include "tree.hpp"

BasicType BasicType::VOID("void");
BasicType BasicType::BOOL("bool");
BasicType BasicType::CHAR("char");
BasicType BasicType::SHORT("short");
BasicType BasicType::INT("int");
BasicType BasicType::LONG("long");
BasicType BasicType::FLOAT("float");
BasicType BasicType::DOUBLE("double");

StringType* StringType::_instance = nullptr;
HSQUIRRELVMType* HSQUIRRELVMType::_instance = nullptr;
SQIntegerType* SQIntegerType::_instance = nullptr;
