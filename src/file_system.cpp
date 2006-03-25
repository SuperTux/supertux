#include <config.h>

#include "msg.hpp"
#include "file_system.hpp"

#include <string>
#include <vector>
#include <sstream>

namespace FileSystem
{

std::string dirname(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if(p == std::string::npos)                              
    return "";
  
  return filename.substr(0, p+1);
}

std::string basename(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if(p == std::string::npos)
    return filename;

  return filename.substr(p, filename.size()-p);
}

std::string normalize(const std::string& filename)
{
  std::vector<std::string> path_stack;

  const char* p = filename.c_str();

  while(true) {
    while(*p == '/') {
      p++;
      continue;
    }

    const char* pstart = p;
    while(*p != '/' && *p != 0) {
      ++p;
    }

    size_t len = p - pstart;
    if(len == 0)
      break;
    
    std::string pathelem(pstart, p-pstart);
    if(pathelem == ".")
      continue;
    
    if(pathelem == "..") {
      if(path_stack.empty()) {

        msg_warning("Invalid '..' in path '" << filename << "'");
        // push it into the result path so that the users sees his error...
        path_stack.push_back(pathelem);
      } else {
        path_stack.pop_back();
      }
    } else {
      path_stack.push_back(pathelem);
    }
  }

  // construct path
  std::ostringstream result;
  for(std::vector<std::string>::iterator i = path_stack.begin();
      i != path_stack.end(); ++i) {
    result << '/' << *i;
  }
  if(path_stack.empty())
    result << '/';

  return result.str();
}

}

