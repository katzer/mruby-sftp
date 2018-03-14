# MIT License
#
# Copyright (c) Sebastian Katzer 2017
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

module SFTP
  # A class representing the attributes of a file or directory on the server.
  # It may be used to specify new attributes, or to query existing attributes.
  class Stat
    # Initializes a new instance of SFTP::Stat.
    #
    # @param [ Hash<Symbol, Object> ] attrs Optional attributes to set.
    #
    # @return [ Void ]
    def initialize(attrs = nil)
      return unless attrs
      @uid   = attrs[:uid]
      @gid   = attrs[:gid]
      @atime = attrs[:atime]
      @mtime = attrs[:mtime]
      @mode  = attrs[:mode]
    end

    # Fixed attributes
    attr_reader :size

    # Attributes than can be used for setstat
    attr_accessor :uid, :gid, :mode, :atime, :mtime

    # Returns true if the named file exists and has a zero size.
    #
    # @return [ Boolean ]
    def zero?
      size.nil? || size == 0
    end

    # Returns the type as a symbol, rather than an integer, for easier use in
    # Ruby programs.
    #
    # @return [ Symbol ]
    def ftype
      case type
      when T_SOCKET       then :socket
      when T_SYMLINK      then :symlink
      when T_REGULAR      then :regular
      when T_BLOCK_DEVICE then :block_device
      when T_DIRECTORY    then :directory
      when T_CHAR_DEVICE  then :char_device
      when T_FIFO         then :fifo
      when T_UNKNOWN      then :unknown
      end
    end

    # Returns true if these attributes appear to describe a directory.
    #
    # @return [ Boolean ]
    def directory?
      case type
      when T_DIRECTORY then true
      when T_UNKNOWN   then nil
      else false
      end
    end

    # Returns true if these attributes appear to describe a symlink.
    #
    # @return [ Boolean ]
    def symlink?
      case type
      when T_SYMLINK then true
      when T_UNKNOWN then nil
      else false
      end
    end

    # Returns true if these attributes appear to describe a regular file.
    #
    # @return [ Boolean ]
    def file?
      case type
      when T_REGULAR then true
      when T_UNKNOWN then nil
      else false
      end
    end

    # Returns the unix file mode.
    #
    # @return [ Int ]
    def umode
      perm = 0
      perm += 400 if mode & P_USR_R != 0
      perm += 200 if mode & P_USR_W != 0
      perm += 100 if mode & P_USR_X != 0
      perm += 40  if mode & P_GRP_R != 0
      perm += 20  if mode & P_GRP_W != 0
      perm += 10  if mode & P_GRP_X != 0
      perm += 4   if mode & P_OTH_R != 0
      perm += 2   if mode & P_OTH_W != 0
      perm += 1   if mode & P_OTH_X != 0
      perm
    end

    # Returns true if stat is executable.
    #
    # @return [ Boolean ]
    def executable?
      (mode & P_USR_X) != 0 || (mode & P_GRP_X) != 0 || (mode & P_OTH_X) != 0
    end

    # Returns true if stat is readable.
    #
    # @return [ Boolean ]
    def readable?
      (mode & P_USR_R) != 0 || (mode & P_GRP_R) != 0 || (mode & P_OTH_R) != 0
    end

    # Returns true if stat is writable.
    #
    # @return [ Boolean ]
    def writable?
      (mode & P_USR_W) != 0 || (mode & P_GRP_W) != 0 || (mode & P_OTH_W) != 0
    end
  end
end
