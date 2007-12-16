#!/usr/local/bin/ruby

if __FILE__ == $0
  require 'slimb/installer'
  files = ["physfs.rb", "physfs_so.so"]
  installer = Slimb::Installer.new.run files, ARGV
end
