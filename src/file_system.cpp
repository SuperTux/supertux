#include <config.h>

#include "file_system.hpp"

#include <string>

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

}

