//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include <cassert>
#include <cstdio>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#ifndef WIN32
#include <libgen.h>
#endif

#include <cctype>

#include "globals.h"
#include "setup.h"

using namespace SuperTux;

#ifdef WIN32
#define mkdir(dir, mode)    mkdir(dir)
#endif

/* Does the given file exist and is it accessible? */
bool FileSystem::faccessible(const std::string& filename)
{
  struct stat filestat;
  if (stat(filename.c_str(), &filestat) == -1)
    {
      return false;
    }
  else
    {
      if(S_ISREG(filestat.st_mode))
        return true;
      else
        return false;
    }
}

/* Can we write to this location? */
bool FileSystem::fwriteable(const std::string& filename)
{
  FILE* fi;
  fi = fopen(filename.c_str(), "wa");
  if (fi == NULL)
    {
      return false;
    }
  fclose(fi);
  return true;
}

/* Makes sure a directory is created in either the SuperTux home directory or the SuperTux base directory.*/
bool FileSystem::fcreatedir(const std::string& relative_dir)
{
  std::string path = user_dir + "/" + relative_dir + "/";
  if(mkdir(path.c_str(),0755) != 0)
    {
      path = datadir + "/" + relative_dir + "/";
      if(mkdir(path.c_str(),0755) != 0)
        {
          return false;
        }
      else
        {
          return true;
        }
    }
  else
    {
      return true;
    }
}

/* Get all names of sub-directories in a certain directory. */
/* Returns the number of sub-directories found. */
/* Note: The user has to free the allocated space. */
std::set<std::string> FileSystem::dsubdirs(const std::string &rel_path,const  std::string& expected_file)
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

std::set<std::string> FileSystem::dfiles(const std::string& rel_path, const  std::string& glob, const  std::string& exception_str)
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

std::string FileSystem::dirname(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if(p == std::string::npos)                              
    return "";
  
  return filename.substr(0, p+1);
}

std::set<std::string> FileSystem::read_directory(const std::string& pathname)
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
