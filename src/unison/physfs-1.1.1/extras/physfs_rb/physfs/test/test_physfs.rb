#
# PhysicsFS test program - mimics real physfs_test
#
require 'readline'
require 'physfs'

def die msg
  puts "#{msg} - reason: #{PhysicsFS.last_error}"
end

#
# parse line to command and args
# 
def parse line
  return false if line.nil?
  
  if line.strip =~ /^(.*?) (?: (?:\s+(.*)) | $)/x
    run $1, $2
  else
    false
  end
end

#
# parse command args
# 
def parse_args args
  args.strip!

  dquoted  = /^ " (.*?) "/x
  squoted  = /^ ' (.*?) '/x
  unquoted = /^([^\s\'\"]+)/
  
  regexps = [dquoted, squoted, unquoted]
  
  result = []
  while args != ""
    regexps.each do |r|
      if args =~ r
	result << $1
	args.sub! r, ""
	args.sub!(/\s+/, "")
	break
      end
    end
  end
  result
end

def usage cmd, prefix = "usage: "
  print prefix
  args = Commands::HELP[cmd]
  if args
    print cmd
    args.scan(/\w+/).each {|x|
      print " <#{x}>"
    }
    puts
  else
    puts %|#{cmd} (no arguments)|
  end
end
  
# commands go below
module Commands
  HELP = {
    "init"           => "argv0",
    "addarchive"     => "archiveLocation append",
    "removearchive"  => "archiveLocation",
    "enumerate"      => "dirToEnumerate",
    "ls"             => "dirToEnumerate",
    "setwritedir"    => "newWriteDir",
    "permitsymlinks" => "1or0",
    "setsaneconfig"  => "org appName arcExt includeCdRoms archivesFirst",
    "mkdir"	     => "dirToMk",
    "delete"         => "dirToDelete",
    "getrealdir"     => "fileToFind",
    "exists"         => "fileToCheck",
    "isdir"          => "fileToCheck",
    "issymlink"      => "fileToCheck",
    "cat"            => "fileToCat",
    "filelength"     => "fileToCheck",
    "append"         => "fileToAppend",
    "write"          => "fileToCreateOrTrash",
    "getlastmodtime" => "fileToExamine"
  }

  def quit_cmd
    exit
  end

  alias q_cmd quit_cmd

  def help_cmd
    commands = ::Commands.instance_methods.grep(/_cmd$/).sort
    puts "Commands:"
    commands.each do |c|
      usage c.sub("_cmd", ""), "  - "
    end

    true
  end

  def e val
    if val
      puts "Successful."
    else
      puts "Failure. reason: #{PhysicsFS.last_error}"
    end
    true
  end

  def init_cmd arg
    e PhysicsFS.init(arg)
  end

  def deinit_cmd
    e PhysicsFS.deinit
  end

  def addarchive_cmd archive, append
    e PhysicsFS.add_to_search_path(archive, append)
  end

  def removearchive_cmd archive
    e PhysicsFS.remove_from_search_path archive
  end

  def enumerate_cmd path
    entries = PhysicsFS.enumerate(path)
    entries.each {|x|
      puts x
    }
    true
  end

  alias ls_cmd enumerate_cmd

  def getlasterror_cmd
    puts "Last error is [#{PhysicsFS.last_error}]"
    true
  end

  def getdirsep_cmd
    puts "Directory separator is [#{PhysicsFS.dir_separator}]"
    true
  end

  def getcdromdirs_cmd
    dirs = PhysicsFS.cdrom_dirs
    dirs.each {|x|
      puts x
    }
    puts " total [#{dirs.length}] drives."
    true
  end

  def getsearchpath_cmd
    spath = PhysicsFS.search_path
    spath.each {|x|
      puts x
    }
    puts "total [#{spath.length}] directories."
    true
  end

  def getbasedir_cmd
    dir = PhysicsFS.base_dir
    puts dir if dir
    true
  end

  def getuserdir_cmd
    puts PhysicsFS.user_dir
    true
  end

  def getwritedir_cmd
    dir = PhysicsFS.write_dir
    if dir
      puts "Write directory is [#{dir}]."
    else
      puts "No write directory defined."
    end
    true
  end

  def setwritedir_cmd dir
    e(PhysicsFS.write_dir = dir)
  end

  def permitsymlinks_cmd val
    if val.to_i == 1
      PhysicsFS.permit_symlinks true
      puts "Symlinks are now permitted"
    else
      PhysicsFS.permit_symlinks false
      puts "Symlinks are now forbidden"
    end
    true
  end

  def setsaneconfig_cmd org, appname, ext, includeCdroms, archivesFirst
    includeCdroms = includeCdroms.to_i == 1
    archiveFirst = archivesFirst == 1
    e PhysicsFS.set_sane_config(org, appname, ext, includeCdroms, archivesFirst)
  end

  def mkdir_cmd dir
    e PhysicsFS.mkdir(dir)
  end

  def delete_cmd dir
    e PhysicsFS.delete(dir)
  end

  def getrealdir_cmd file
    dir = PhysicsFS.real_dir file
    if dir
      puts "Found at [#{dir}]"
    else
      puts "Not found."
    end
    true
  end

  def exists_cmd file
    if PhysicsFS.exists? file
      puts "File exists"
    else
      puts "File does not exist"
    end
    true
  end

  def isdir_cmd file
    if PhysicsFS.is_directory? file
      puts "File is a directory"
    else
      puts "File is NOT a directory"
    end
    true
  end

  def issymlink_cmd file
    if PhysicsFS.is_symlink? file
      puts "File is a symlink"
    else
      puts "File is NOT a symlink"
    end
    true
  end

  def cat_cmd filename
    file = PhysicsFS.open_read filename
    if file.nil?
      puts "failed to open. reason: #{PhysicsFS.last_error}"
      return true
    end

    puts file.cat
    true
  end

  def filelength_cmd filename
    file = PhysicsFS.open_read filename
    if file.nil?
      puts "failed to open. reason: #{PhysicsFS.last_error}"
      return true
    end

    puts file.length
    file.close
    true
  end

  WRITE_STR = "Rubyfied PhysicsFS works just fine.\n\n"
  
  def append_cmd filename
    file = PhysicsFS.open_append filename
    if file.nil?
      puts "failed to open. reason: #{PhysicsFS.last_error}"
      return true
    end

    file.write WRITE_STR, 1, WRITE_STR.length
    file.close
    true
  end

  def write_cmd filename
    file = PhysicsFS.open_write filename
    if file.nil?
      puts "failed to open. reason: #{PhysicsFS.last_error}"
      return true
    end

    file.write_str WRITE_STR
    file.close
    true
  end

  def getlastmodtime_cmd filename
    t = PhysicsFS.last_mod_time filename
    if t == -1
      puts "failed to determin. reason: #{PhysicsFS.last_error}"
    else
      puts "Last modified: #{Time.at(t)}"
    end
    true
  end
end

include Commands

def run command, args
  if args
    args = parse_args args
  else
    args = []
  end

  begin
    cmd = method "#{command}_cmd"
    if args.length == cmd.arity
      return cmd.call *args
    else
      usage command
      true
    end
  rescue NameError
    puts 'Unknown command. Enter "help" for instructions.'
    true
  end
end

if __FILE__ == $0
  
  PhysicsFS.init($0) or die "PhysicsFS init failed"
  
  puts "PhysicsFS version: #{PhysicsFS.version}"
  puts

  puts "Supported archives: "
  puts PhysicsFS.supported_archives
  puts

  puts 'Enter commands. Enter "help" for instructions.'

  loop {
    line = Readline::readline "physfs_rb> ", true
    break unless parse line
  }
end




