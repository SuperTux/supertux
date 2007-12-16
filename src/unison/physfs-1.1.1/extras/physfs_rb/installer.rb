# $Id: installer.rb 585 2003-07-21 03:46:50Z icculus $

require 'rbconfig'
require 'find'
require 'ftools'

include Config

module Slimb
  class Installer
    def initialize target_dir = "", &user_skip 
      @user_skip = user_skip or proc {|f| false}
      
      @version = CONFIG["MAJOR"] + "." + CONFIG["MINOR"]
      @libdir = File.join(CONFIG["libdir"], "ruby", @version)
      @sitedir = CONFIG["sitedir"] || File.join(@libdir, "site_ruby")
      @dest = File.join @sitedir, target_dir

      File::makedirs @dest
      File::chmod 0755, @dest, true
    end

    def skip? file
      @user_skip[file] or
	file[0] == ?. or file[-1] == ?~ or file[-1] == ?#
    end
    
    def install_dir dir 
      File::makedirs(File.join(@dest, dir))
      File::chmod(0755, File.join(@dest, dir), true)
      Dir.foreach(dir) {|file|
	next if skip? file
	
	if File.ftype(File.join(dir, file)) == "directory"
	  install_dir File.join(dir, file)
	else
	  install_file File.join(dir, file)
	end
      }
    end

    def install_file file
      if file =~ /\.so$/
	install_so file
      else
	File::install file, File.join(@dest, file), 0644, true
      end
    end

    def install_so file
      File::install file, File.join(CONFIG["sitearchdir"], file), 0644, true
    end

    def uninstall_so file
      file = File.join(CONFIG["sitearchdir"], file)
      File::safe_unlink file
    end

    def install something
      case something
      when Array
	something.each {|x|
	  install x if x.is_a? String
	}
      when String
	if File.ftype(something) == "directory"
	  install_dir something
	else
	  install_file something
	end
      end
    end

    def uninstall what = "*"
      case what
      when Array
	files = what.map {|x| File.join(@dest, x)}
      when String
	files = Dir[File.join(@dest, what)]
      end
      
      files.each {|x|
	# FIXME: recursive uninstall is a must
	next if FileTest.directory? x
	File::safe_unlink x
      }
    end

    def run files, argv
      if !argv.grep(/--uninstall/).empty?
	uninstall files
      else
	install files
      end	
    end
  end
end

# self-installation 
if $0 == __FILE__
  $stderr.puts "Installing slimb installer..."
  Slimb::Installer.new("slimb").install File.basename(__FILE__)
end
