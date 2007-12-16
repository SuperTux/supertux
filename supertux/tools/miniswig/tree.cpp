#include <config.h>
#include "tree.hpp"

BasicType BasicType::VOID("void");
BasicType BasicType::BOOL("bool");
BasicType BasicType::CHAR("char");
BasicType BasicType::SHORT("short");
BasicType BasicType::INT("int");
BasicType BasicType::LONG("long");
BasicType BasicType::FLOAT("float");
BasicType BasicType::DOUBLE("double");

StringType* StringType::_instance = NULL;
HSQUIRRELVMType* HSQUIRRELVMType::_instance = NULL;
SQIntegerType* SQIntegerType::_instance = NULL;
