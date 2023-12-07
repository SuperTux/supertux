//  SuperTux - Scripting reference generator
//  Copyright (C) 2023 Vankata453
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

/** This program converts Doxygen XML output files into 
    Markdown scripting reference documentation, using templates. **/

/* Requirements: C++17, TinyXML2 library */

#include <algorithm>
#include <vector>
#include <iostream>
#include <filesystem>

#include <tinyxml2.h>

#include "class.hpp"
#include "generator.hpp"
#include "parser.hpp"
#include "util.hpp"

int main(int argc, char** argv)
{
  /** Get required parameters **/
  std::string input_dir, home_template_file, page_template_file, output_dir("."), output_data_file;
  for (int i = 1; i < argc; i++)
  {
    if (param_matches(argc, argv, i, "-d", "--dir", "--directory")) // Input directory has been provided
      input_dir = argv[i + 1];
    else if (param_matches(argc, argv, i, "-h", "--home", "--home-template")) // Home template has been provided
      home_template_file = argv[i + 1];
    else if (param_matches(argc, argv, i, "-p", "--page", "--page-template")) // Page template has been provided
      page_template_file = argv[i + 1];
    else if (param_matches(argc, argv, i, "-o", "--output", "--output-directory")) // Output directory has been provided
      output_dir = argv[i + 1];
    else if (param_matches(argc, argv, i, "-od", "--output-data", "--output-data-file")) // Output data file has been provided
      output_data_file = argv[i + 1];
  }
  const bool generate_markdown_reference = (!home_template_file.empty() && !page_template_file.empty());
  if (input_dir.empty() || (!generate_markdown_reference && output_data_file.empty())) // Do not allow empty parameters
  {
    std::cout << "Usage: [\"-d\", \"--dir\", \"--directory\"] (string [REQUIRED])" << std::endl;
    std::cout << "       [\"-h\", \"--home\", \"--home-template\"] (string [REQUIRED for .md output])" << std::endl;
    std::cout << "       [\"-p\", \"--page\", \"--page-template\"] (string [REQUIRED for .md output])" << std::endl;
    std::cout << "       [\"-o\", \"--output\", \"--output-directory\"] (string [DEFAULT = \".\" (current)])" << std::endl;
    std::cout << "       [\"-od\", \"--output-data\", \"--output-data-file\"] (string [REQUIRED for .stsr output])" << std::endl;
    return 1;
  }

  std::vector<Class> classes;

  /** Loop through all files in the provided directory and parse ones containing XML class data **/
  for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(input_dir))
  {
    std::filesystem::path fspath = dir_entry;
    const std::string filename = fspath.filename();

    if (!(std::filesystem::is_regular_file(dir_entry) &&
          starts_with(filename, "classscripting_1_1"))) continue; // Make sure the current file is about "scripting" namespace member class

    /** Read data from current XML class data file **/
    tinyxml2::XMLDocument doc;
    doc.LoadFile(fspath.c_str());

    /** Parse the class and its functions **/
    Class cl; // Store class data
    Parser::parse_compounddef(doc.RootElement(), cl);

    if (cl.constants.empty() && cl.functions.empty()) continue; // Exclude empty classes

    std::cout << "Successfully parsed scripting class \"" << cl.name << "\"." << std::endl;

    // Push to class stack
    classes.push_back(std::move(cl));
  }

  // Sort classes by their names (A-Z)
  std::sort(classes.begin(), classes.end(),
            [](const Class& lhs, const Class& rhs) { return lhs.name < rhs.name; });

  /** Generate markdown reference, if template files have been provided */
  if (generate_markdown_reference)
    Generator::generate_markdown_reference(output_dir, home_template_file, page_template_file, classes);

  /** Generate data (.stsr) file, if target file destination has been provided */
  if (!output_data_file.empty())
    Generator::generate_data_file(output_data_file, classes);

  return 0;
}
