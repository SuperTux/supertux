#ifndef __LISPWRITER_H__
#define __LISPWRITER_H__

#include <iostream>
#include <vector>

class LispWriter
{
public:
  LispWriter(std::ostream& out);
  ~LispWriter();

  void write_comment(const std::string& comment);
  
  void start_list(const std::string& listname);

  void write_int(const std::string& name, int value);
  void write_float(const std::string& name, float value);
  void write_string(const std::string& name, const std::string& value);
  void write_bool(const std::string& name, bool value);
  void write_int_vector(const std::string& name, const std::vector<int>& value);
  void write_int_vector(const std::string& name, const std::vector<unsigned int>& value);
  // add more write-functions when needed...
  
  void end_list(const std::string& listname);

private:
  void indent();
    
  std::ostream& out;
  int indent_depth;
  std::vector<std::string> lists;
};

#endif

