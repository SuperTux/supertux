#include "lispwriter.h"
#include <iostream>

LispWriter::LispWriter(std::ostream& newout)
  : out(newout), indent_depth(0)
{
}

LispWriter::~LispWriter()
{
  if(lists.size() > 0) {
    std::cerr << "Warning: Not all sections closed in lispwriter!\n";
  }
}

void
LispWriter::writeComment(const std::string& comment)
{
  out << "; " << comment << "\n";
}

void
LispWriter::startList(const std::string& listname)
{
  indent();
  out << '(' << listname << '\n';
  indent_depth += 2;

  lists.push_back(listname);
}

void
LispWriter::endList(const std::string& listname)
{
  if(lists.size() == 0) {
    std::cerr << "Trying to close list '" << listname 
              << "', which is not open.\n";
    return;
  }
  if(lists.back() != listname) {
    std::cerr << "Warning: trying to close list '" << listname 
              << "' while list '" << lists.back() << "' is open.\n";
    return;
  }
  lists.pop_back();
  
  indent_depth -= 2;
  indent();
  out << ")\n";
}

void
LispWriter::writeInt(const std::string& name, int value)
{
  indent();
  out << '(' << name << ' ' << value << ")\n";
}

void
LispWriter::writeFloat(const std::string& name, float value)
{
  indent();
  out << '(' << name << ' ' << value << ")\n";
}

void
LispWriter::writeString(const std::string& name, const std::string& value)
{
  indent();
  out << '(' << name << " \"" << value << "\")\n";
}

void
LispWriter::writeBool(const std::string& name, bool value)
{
  indent();
  out << '(' << name << ' ' << (value ? "#t" : "#f") << ")\n";
}

void
LispWriter::writeIntVector(const std::string& name,
    const std::vector<int>& value)
{
  indent();
  out << '(' << name;
  for(std::vector<int>::const_iterator i = value.begin(); i != value.end(); ++i)
    out << " " << *i;
  out << ")\n";
}

void
LispWriter::writeIntVector(const std::string& name,
    const std::vector<unsigned int>& value)
{
  indent();
  out << '(' << name;
  for(std::vector<unsigned int>::const_iterator i = value.begin(); i != value.end(); ++i)
    out << " " << *i;
  out << ")\n";
}

void
LispWriter::indent()
{
  for(int i = 0; i<indent_depth; ++i)
    out << ' ';
}

