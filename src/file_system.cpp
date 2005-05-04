#include <config.h>

#include "file_system.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#ifndef WIN32
#include <libgen.h>
#endif

#include "resources.h"

#ifdef WIN32
#define mkdir(dir, mode)    mkdir(dir)
#endif

namespace FileSystem
{

/* Does the given file exist and is it accessible? */
bool faccessible(const std::string& filename)
{
  FILE* f = fopen(filename.c_str(), "r");
  if(f == 0)
    return false;

  fclose(f);
  return true;
}

/* Can we write to this location? */
bool fwriteable(const std::string& filename)
{
  FILE* f = fopen(filename.c_str(), "wa");
  if (f == 0)
    return false;
  
  fclose(f);
  return true;
}

/* Makes sure a directory is created in either the SuperTux home directory or the SuperTux base directory.*/
bool fcreatedir(const std::string& relative_dir)
{
  std::string path = user_dir + "/" + relative_dir + "/";
  if(mkdir(path.c_str(),0755) == 0)
    return true;
  
  path = datadir + "/" + relative_dir + "/";
  if(mkdir(path.c_str(),0755) == 0)
    return true;
    
  return false;
}

/* Get all names of sub-directories in a certain directory. */
/* Returns the number of sub-directories found. */
/* Note: The user has to free the allocated space. */
std::set<std::string> dsubdirs(const std::string &rel_path,
    const std::string& expected_file)
{
  DIR *dirStructP;
  struct dirent *direntp;
  std::set<std::string> sdirs;
  std::string filename;
  std::string path = user_dir + "/" + rel_path;

  if((dirStructP = opendir(path.c_str())) != NULL)
    {
      while((direntp = readdir(dirStructP)) != NULL)
        {
          std::string absolute_filename;
          struct stat buf;

          absolute_filename = path + "/" + direntp->d_name;

          if (stat(absolute_filename.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode))
            {
              if(!expected_file.empty())
                {
                  filename = path + "/" + direntp->d_name + "/" + expected_file;
                  if(!faccessible(filename))
                    continue;
                }

	      sdirs.insert(direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  path = datadir + "/" + rel_path;
  if((dirStructP = opendir(path.c_str())) != NULL)
    {
      while((direntp = readdir(dirStructP)) != NULL)
        {
          std::string absolute_filename;
          struct stat buf;

          absolute_filename = path + "/" + direntp->d_name;

          if (stat(absolute_filename.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode))
            {
              if(!expected_file.empty())
                {
                  filename = path + "/" + direntp->d_name + "/" + expected_file;
                  if(!faccessible(filename.c_str()))
                    {
                      continue;
                    }
                  else
                    {
                      filename = user_dir + "/" + rel_path + "/" + direntp->d_name + "/" + expected_file;
                      if(faccessible(filename.c_str()))
                        continue;
                    }
                }

	      sdirs.insert(direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  return sdirs;
}

std::set<std::string> dfiles(const std::string& rel_path,
    const std::string& glob, const std::string& exception_str)
{
  DIR *dirStructP;
  struct dirent *direntp;
  std::set<std::string> sdirs;
  std::string path = user_dir + "/" + rel_path;

  if((dirStructP = opendir(path.c_str())) != NULL)
    {
      while((direntp = readdir(dirStructP)) != NULL)
        {
          std::string absolute_filename;
          struct stat buf;

          absolute_filename = path + "/" + direntp->d_name;

          if (stat(absolute_filename.c_str(), &buf) == 0 && S_ISREG(buf.st_mode))
            {
              if(!exception_str.empty())
                {
                  if(strstr(direntp->d_name,exception_str.c_str()) != NULL)
                    continue;
                }
              if(!glob.empty())
                if(strstr(direntp->d_name,glob.c_str()) == NULL)
                  continue;

	      sdirs.insert(direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  path = datadir + "/" + rel_path;
  if((dirStructP = opendir(path.c_str())) != NULL)
    {
      while((direntp = readdir(dirStructP)) != NULL)
        {
          std::string absolute_filename;
          struct stat buf;

          absolute_filename = path + "/" + direntp->d_name;

          if (stat(absolute_filename.c_str(), &buf) == 0 && S_ISREG(buf.st_mode))
            {
              if(!exception_str.empty())
                {
                  if(strstr(direntp->d_name,exception_str.c_str()) != NULL)
                    continue;
                }
              if(!glob.empty())
                if(strstr(direntp->d_name,glob.c_str()) == NULL)
                  continue;

	      sdirs.insert(direntp->d_name);
            }
        }
      closedir(dirStructP);
    }

  return sdirs;
}

std::string dirname(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if(p == std::string::npos)                              
    return "";
  
  return filename.substr(0, p+1);
}

std::set<std::string> read_directory(const std::string& pathname)
{
  std::set<std::string> dirnames;
  
  DIR* dir = opendir(pathname.c_str());
  if (dir)
    {
      struct dirent *direntp;
      
      while((direntp = readdir(dir)))
        {
          dirnames.insert(direntp->d_name);
        }
      
      closedir(dir);
    }

  return dirnames;
}

}

