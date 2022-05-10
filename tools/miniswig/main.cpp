//#include <config.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include "tree.hpp"
#include "globals.hpp"
#include "create_wrapper.hpp"
#include "create_docu.hpp"

extern int yyparse();
extern int yylex();

CompilationUnit* unit = 0;
std::istream* input = 0;
std::string inputfile;
std::string selected_namespace;
std::string modulename = "wrapper";

void usage()
{
    std::cout << "Usage: miniswig --input FILE --output-cpp FILE --output-hpp FILE [--module NAME] [--select-namespace NAME]\n";
    std::cout << "\n";
}

int main(int argc, char** argv)
{
    std::string outputcpp;
    std::string outputhpp;
    std::string output_doc;
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "--module") == 0) {
            if(i+1 >= argc) {
                std::cerr << "Need to specify a module name.\n";
                usage();
                return 1;
            }
            modulename = argv[++i];
        } else if(strcmp(argv[i], "--input") == 0) {
            if(i+1 >= argc) {
                std::cerr << "Need to specify input file name.\n";
                usage();
                return 1;
            }
            inputfile = argv[++i];
        } else if(strcmp(argv[i], "--output-cpp") == 0) {
            if(i+1 >= argc) {
                std::cerr << "Need to specify output cpp name.\n";
                usage();
                return 1;
            }
            outputcpp = argv[++i];
        } else if(strcmp(argv[i], "--output-hpp") == 0) {
            if(i+1 >= argc) {
                std::cerr << "Need to specify output hpp name.\n";
                usage();
                return 1;
            }
            outputhpp = argv[++i];
        } else if(strcmp(argv[i], "--select-namespace") == 0) {
            if(i+1 >= argc) {
                std::cerr << "Need to specify a namespace.\n";
                usage();
                return 1;
            }
            selected_namespace = argv[++i];
        } else if(strcmp(argv[i], "--output-doc") == 0) {
          if(i+1 >= argc) {
            std::cerr << "Need to specify document xml file.\n";
            usage();
            return 1;
          }
          output_doc = argv[++i];
        } else if(argv[i][0] == '-') {
            std::cerr << "Unknown option '" << argv[i] << "'.\n";
            usage();
            return 1;
        } else {
        }
    }
    if( inputfile == "" || (
            (outputcpp == "" || outputhpp == "") && output_doc == "")) {
        std::cerr << "Not all options specified.\n";
        usage();
        return 1;
    }

    try {
        input = new std::ifstream(inputfile.c_str());
        if(!input->good()) {
            std::cerr << "Couldn't open file '" << inputfile << "' for reading.\n";
            return 1;
        }
        current_file = inputfile;
        unit = new CompilationUnit();
        Namespace* std_namespace = new Namespace();
        std_namespace->name = "std";
        std_namespace->types.push_back(new StringType());
        unit->namespaces.push_back(std_namespace);
        unit->types.push_back(new HSQUIRRELVMType());
        unit->types.push_back(new SQIntegerType());

        yyparse();

        Namespace* ns = unit;
        if(selected_namespace != "") {
            ns = ns->findNamespace(selected_namespace);
        }

        if(outputcpp != "") {
            std::ofstream cppout(outputcpp.c_str());
            if(!cppout.good()) {
                std::cerr << "Couldn't open file '"
                          << outputcpp << "' for writing.\n";
                return 1;
            }
            std::ofstream hppout(outputhpp.c_str());
            if(!hppout.good()) {
                std::cerr << "Couldn't open file '" << outputhpp
                          << "' for writing.\n";
                return 1;
            }

            WrapperCreator creator(cppout, hppout);
            creator.create_wrapper(ns);
        }

        if(output_doc != "") {
            std::ofstream dout(output_doc.c_str());
            if(!dout.good()) {
                std::cerr << "Couldn't open file '"
                    << output_doc << "' for writing.\n";
                return 1;
            }
            DocuCreator creator(dout);
            creator.create_docu(ns);
        }
    } catch(std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
