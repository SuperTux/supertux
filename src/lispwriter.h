#ifndef __LISPWRITER_H__
#define __LISPWRITER_H__

#include <iostream>
#include <vector>

class LispWriter
{
public:
  LispWriter(std::ostream& out);
  ~LispWriter();

  void writeComment(const std::string& comment);
  
  void startList(const std::string& listname);

  void writeInt(const std::string& name, int value);
  void writeFloat(const std::string& name, float value);
  void writeString(const std::string& name, const std::string& value);
  void writeBool(const std::string& name, bool value);
  void writeIntVector(const std::string& name, const std::vector<int>& value);
  void writeIntVector(const std::string& name, const std::vector<unsigned int>& value);
  // add more write-functions when needed...
  
  void endList(const std::string& listname);

private:
  void indent();
    
  std::ostream& out;
  int indent_depth;
  std::vector<std::string> lists;
};

#endif

