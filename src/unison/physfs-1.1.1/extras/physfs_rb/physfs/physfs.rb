#
# PhysicsFS - ruby interface
#
# Author: Ed Sinjiashvili (slimb@vlinkmail.com)
# License: LGPL
#

require 'physfs_so'

module PhysicsFS

  class Version
    def initialize major, minor, patch
      @major = major
      @minor = minor
      @patch = patch
    end

    attr_reader :major, :minor, :patch

    def to_s
      "#@major.#@minor.#@patch"
    end
  end

  class ArchiveInfo
    def initialize ext, desc, author, url
      @extension = ext
      @description = desc
      @author = author
      @url = url
    end

    attr_reader :extension, :description
    attr_reader :author, :url

    def to_s
      " * #@extension: #@description\n    Written by #@author.\n    #@url\n"
    end
  end

  #
  # convenience methods
  #
  class << self  

    def init argv0 = $0
      init_internal argv0
    end

    def append_search_path str
      add_to_search_path str, 1
      self
    end

    def prepend_search_path str
      add_to_search_path str, 0
      self
    end

    alias_method :<<,      :append_search_path
    alias_method :push,    :append_search_path
    alias_method :unshift, :prepend_search_path

    def ls path = ""
      enumerate path
    end
  end

  #
  # File - PhysicsFS abstract file - can be drawn from various sources
  # 
  class File
    def write_str str
      write str, 1, str.length
    end
    
    def cat
      prev_pos = tell
      seek 0
      r = read length, 1
      seek prev_pos
      r
    end
    
    alias_method :size, :length
  end

  #
  # RWops - general stdio like operations on file-like creatures
  #
  class RWops
    SEEK_SET = 0
    SEEK_CUR = 1
    SEEK_END = 2

    # tell current position of RWopted entity
    def tell
      seek 0, SEEK_CUR
    end

    # length of RWops abstracted entity
    def length
      cur = tell
      r = seek 0, SEEK_END
      seek cur, SEEK_SET
      r
    end

    alias_method :size, :length

    #
    # create rwops from PhysicsFS file object
    # 
    def self.from_physfs file
      file.to_rwops
    end
  end
end

# physfs.rb ends here #
