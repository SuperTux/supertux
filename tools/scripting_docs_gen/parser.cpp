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

#include "parser.hpp"

#include <string>
#include <iostream>

#include "util.hpp"

namespace Parser {

Class* cl = nullptr;


void parse_compounddef(tinyxml2::XMLElement* p_root, Class& cls)
{
  tinyxml2::XMLElement* p_compounddef = p_root->FirstChildElement("compounddef");

  // Get general class info
  cls.name = p_compounddef->FirstChildElement("compoundname")->GetText();
  replace(cls.name, "scripting::", ""); // Leave only the class name

  // Get additional info
  tinyxml2::XMLElement* p_detaileddescpara = p_compounddef->FirstChildElement("detaileddescription")->FirstChildElement("para");
  if (p_detaileddescpara) // Detailed description (possibly containing additional info) is available
  {
    tinyxml2::XMLElement* p_xrefsect = p_detaileddescpara->FirstChildElement("xrefsect");
    while (p_xrefsect)
    {
      // Check if the "xrefsect" title matches any of the additional info titles
      const std::string title = p_xrefsect->FirstChildElement("xreftitle")->GetText();
      if (title == "Summary")
        parse_xrefsect_desc(p_xrefsect, cls.summary);
      else if (title == "Instances")
        parse_xrefsect_desc(p_xrefsect, cls.instances);
      else if (title == "Scope")
      {
        std::string scope;
        parse_xrefsect_desc(p_xrefsect, scope);

        cls.global = (scope == "global");
      }

      p_xrefsect = p_xrefsect->NextSiblingElement("xrefsect");
    }
  }

  cl = &cls;
  parse_sectiondef(p_compounddef);
  cl = nullptr;
}

void parse_sectiondef(tinyxml2::XMLElement* p_compounddef)
{
  if (!cl) return;

  tinyxml2::XMLElement* p_sectiondef = p_compounddef->FirstChildElement("sectiondef");
  while (p_sectiondef)
  {
    if (attr_equal(p_sectiondef, "kind", "public-func") ||
        attr_equal(p_sectiondef, "kind", "public-static-attrib")) // Is public, contains public class functions or static attributes
    {
      parse_memberdef(p_sectiondef);
    }
    p_sectiondef = p_sectiondef->NextSiblingElement("sectiondef");
  }
}

void parse_memberdef(tinyxml2::XMLElement* p_sectiondef)
{
  if (!cl) return;

  tinyxml2::XMLElement* p_memberdef = p_sectiondef->FirstChildElement("memberdef");
  while (p_memberdef)
  {
    if (attr_equal(p_memberdef, "kind", "function") &&
        attr_equal(p_memberdef, "static", "no") &&
        !el_equal(p_memberdef, "type", "")) // Try searching for non-static, typed functions
    {
      /** Parse the function **/
      Function func;

      // Get general info
      func.type = p_memberdef->FirstChildElement("type")->GetText();
      if (starts_with(func.type, "std::")) // Remove a potential "std::" part from the type string
        func.type.erase(0, 5);
      func.name = p_memberdef->FirstChildElement("name")->GetText();
      if (starts_with(func.name, cl->name + "_")) // If function name starts with the string "{CLASS}_", remove it
        func.name.erase(0, cl->name.size() + 1);
      tinyxml2::XMLElement* p_descpara = p_memberdef->FirstChildElement("briefdescription")->FirstChildElement("para");
      tinyxml2::XMLElement* p_detaileddescpara = p_memberdef->FirstChildElement("detaileddescription")->FirstChildElement("para");
      if (p_descpara) // Brief description has been provided
      {
        XMLTextReader read(func.description);
        p_descpara->Accept(&read);
      }
      if (p_detaileddescpara && p_detaileddescpara->GetText()) // The description may also continue in the detailed description. Check if more text is available.
      {
        func.detailed_description = p_detaileddescpara->GetText();
        while (!func.detailed_description.empty() && func.detailed_description.back() == ' ')
          func.detailed_description.pop_back();
      }

      /** Parse function parameters **/
      parse_parameterlist(p_memberdef, func);

      // Add to function list
      cl->functions.push_back(func);
    }
    else if (attr_equal(p_memberdef, "kind", "variable") &&
             !el_equal(p_memberdef, "type", "") &&
             starts_with(p_memberdef->FirstChildElement("type")->GetText(), "const ")) // Otherwise, try searching for constant variables
    {
      /** Parse the constant **/
      Constant con;

      // Get general info
      con.type = p_memberdef->FirstChildElement("type")->GetText();
      con.type.erase(0, 6); // Remove the "const " part from the type string
      if (starts_with(con.type, "std::")) // Remove a potential "std::" part from the type string
        con.type.erase(0, 5);
      con.name = p_memberdef->FirstChildElement("name")->GetText();
      tinyxml2::XMLElement* p_descpara = p_memberdef->FirstChildElement("briefdescription")->FirstChildElement("para");
      tinyxml2::XMLElement* p_detaileddescpara = p_memberdef->FirstChildElement("detaileddescription")->FirstChildElement("para");
      if (p_descpara) // Brief description has been provided
      {
        XMLTextReader read(con.description);
        p_descpara->Accept(&read);
      }
      if (p_detaileddescpara && p_detaileddescpara->GetText()) // The description may also continue in the detailed description. Check if more text is available.
      {
        con.detailed_description = p_detaileddescpara->GetText();
        while (!con.detailed_description.empty() && con.detailed_description.back() == ' ')
          con.detailed_description.pop_back();
      }

      // Add to constants list
      cl->constants.push_back(con);
    }
    p_memberdef = p_memberdef->NextSiblingElement("memberdef");
  }
}

void parse_parameterlist(tinyxml2::XMLElement* p_memberdef, Function& func)
{
  if (!cl) return;

  tinyxml2::XMLElement* p_detaileddescpara = p_memberdef->FirstChildElement("detaileddescription")->FirstChildElement("para");
  if (!p_detaileddescpara) return; // There is no detailed description, hence no parameter list

  tinyxml2::XMLElement* p_parameterlist = p_detaileddescpara->FirstChildElement("parameterlist");
  if (!p_parameterlist) return; // No parameter list

  tinyxml2::XMLElement* p_parameteritem = p_parameterlist->FirstChildElement("parameteritem");
  while (p_parameteritem)
  {
    /** Parse the parameter **/
    Parameter param;

    tinyxml2::XMLElement* p_parameternamelist = p_parameteritem->FirstChildElement("parameternamelist");
    tinyxml2::XMLElement* p_parametertype = p_parameternamelist->FirstChildElement("parametertype");
    if (p_parametertype) // Type has been provided
    {
      param.type = p_parametertype->GetText();
      if (starts_with(param.type, "std::")) // Remove a potential "std::" part from the type string
        param.type.erase(0, 5);
    }

    tinyxml2::XMLElement* p_parametername = p_parameternamelist->FirstChildElement("parametername");
    if (p_parametername) // Name has been provided
      param.name = p_parametername->GetText();
    if (starts_with(param.name, "$")) // Parameter names may start with "$" so they can have types assigned. Remove the "$" if so
      param.name.erase(0, 1);

    tinyxml2::XMLElement* p_parameterdescpara = p_parameteritem->FirstChildElement("parameterdescription")->FirstChildElement("para");
    if (p_parameterdescpara) // Description has been provided
    {
      XMLTextReader read(param.description);
      p_parameterdescpara->Accept(&read);
    }

    // Add to parameter list
    func.parameters.push_back(param);

    p_parameteritem = p_parameteritem->NextSiblingElement("parameteritem");
  }
}


void parse_xrefsect_desc(tinyxml2::XMLElement* p_xrefsect, std::string& dest)
{
  tinyxml2::XMLElement* p_xrefsectdescpara = p_xrefsect->FirstChildElement("xrefdescription")->FirstChildElement("para");
  if (p_xrefsectdescpara) // Description has been provided
  {
    XMLTextReader read(dest);
    p_xrefsectdescpara->Accept(&read);
  }
}

} // namespace Parser
