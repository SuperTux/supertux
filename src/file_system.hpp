#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <set>
#include <string>

namespace FileSystem
{
  std::string dirname(const std::string& filename);
  std::string basename(const std::string& filename);
}

#endif

