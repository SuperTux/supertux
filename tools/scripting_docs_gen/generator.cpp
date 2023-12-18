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

#include "generator.hpp"

#include <iostream>
#include <filesystem>

#include "class.hpp"
#include "util.hpp"
#include "writer.hpp"

namespace Generator {

void generate_markdown_reference(const std::string& output_dir,
                                 const std::string& home_template_file, const std::string& page_template_file,
                                 const std::vector<Class>& classes)
{
  /** Prepare other variables **/
  std::filesystem::path output_dir_path = output_dir;

  /** Read template files **/
  const std::string home_template = read_file(home_template_file);
  const std::string page_template = read_file(page_template_file);
  const std::string home_template_filename = std::filesystem::path(home_template_file).filename();
  const std::string page_template_filename = std::filesystem::path(page_template_file).filename();

  for (const Class& cl : classes)
  {
    /** Fill in the data in the provided page template file and save as new file.
        File entries to be replaced:
          "${SRG_CLASSSUMMARY}": Insert the provided summary info of the class, if available.
          "${SRG_CLASSINSTANCES}": Insert the provided instances info of the class, if available.
          "${SRG_CLASSCONSTANTS}": Insert a table with all constants in the class and their data.
          "${SRG_FUNCDATATABLE}": Insert a table with all functions in the class and their data.
          "${SRG_CLASSNAME}": Insert the class name.
          "${SRG_REF_[class]}": Insert the name of the referenced class, as well as its reference URL.

        File formatting replacements:
          "${SRG_NEWPARAGRAPH}": Insert two new lines to start a new paragraph.
          "${SRG_TABLENEWPARAGRAPH}": Insert two HTML line breaks to start a new paragraph in a table.
          [""] (double quotation marks): Replace with [`] for code formatting.
    **/
    // Prepare target data (add a notice at the top of the generated file)
    std::string target_data = MarkdownWriter::write_file_notice(page_template_filename) + page_template;

    // Entries
    replace(target_data, "${SRG_CLASSSUMMARY}", cl.summary, "None.");
    replace(target_data, "${SRG_CLASSINSTANCES}", cl.instances, "None.");
    replace(target_data, "${SRG_CLASSCONSTANTS}", MarkdownWriter::write_constants_table(cl.constants), "None.");
    replace(target_data, "${SRG_CLASSFUNCTIONS}", MarkdownWriter::write_function_table(cl.functions), "None.");
    replace(target_data, "${SRG_CLASSNAME}", "`" + cl.name + "`");
    regex_replace(target_data, std::regex("\\$\\{SRG_REF_(.+?)\\}"), MarkdownWriter::write_class_ref("$1"));
    // Formatting
    replace(target_data, "${SRG_NEWPARAGRAPH} ", "\r\n\r\n");
    replace(target_data, "${SRG_TABLENEWPARAGRAPH}", "<br /><br />");
    replace(target_data, "\"\"", "`");

    // Write to target file
    write_file(output_dir_path / std::filesystem::path("Scripting" + cl.name + ".md"), target_data);
  }

  /** Fill in the data in the provided home page template file and save as new file.
      File entries to be replaced:
        "${SRG_CLASSLIST}": Insert a list with all classes which have been parsed.
  **/
  // Prepare target data (add a notice at the top of the generated file)
  std::string target_data = MarkdownWriter::write_file_notice(home_template_filename) + home_template;

  // Entries
  replace(target_data, "${SRG_CLASSLIST}", MarkdownWriter::write_class_list(classes));

  // Write to target file
  write_file(output_dir_path / std::filesystem::path("Scripting_Reference.md"), target_data);
}

void generate_data_file(const std::string& output_file, const std::vector<Class>& classes)
{
  // Prepare target data
  std::string target_data = SExpWriter::write_data_file(classes);

  // Entries
  regex_replace(target_data, std::regex("\\$\\{SRG_REF_(.+?)\\}"), std::string("`$1`"));
  // Formatting
  replace(target_data, "${SRG_NEWPARAGRAPH}", "");
  replace(target_data, "${SRG_TABLENEWPARAGRAPH}", "");
  replace(target_data, "\\\"\\\"", "`");

  // Write to target file
  write_file(output_file, target_data);
}

} // namespace Writer
