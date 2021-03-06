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
  # A wrapper around an SFTP file handle, that exposes an IO-like interface for
  # interacting with the remote file.
  class Handle
    # Creates a new SFTP::Handle instance atop the given SFTP connection.
    #
    # @param [ SFTP::Session ] session The underlying SFTP session.
    # @param [ String ]        path    The path to the file or dir.
    #
    # @return [ SFTP::Handle ]
    def initialize(session, path)
      @session = session
      @path    = path.dup.freeze
    end

    # The path to the file or dir.
    #
    # @return [ String ]
    attr_reader :path

    # Opens a file on the remote server.
    #
    # @param [ String ] flags Determines how to open the file.
    # @param [ Int ]    mode  The mode in case of the file has to be created.
    #                         Defaults to: 0o644
    #
    # @return [ Void ]
    def open(flags = 'r', mode = 0o644)
      case type = @session.stat(path).ftype
      when :directory then open_dir
      when :regular   then open_file(flags, mode)
      else raise SFTP::Exception, "Don't know how to open #{type}"
      end
    end

    # Returns true if the connection has been initialized.
    #
    # @return [ Boolean ] true if open, otherwise false.
    def open?
      !closed?
    end

    # Performs an fstat operation on the handle and returns the stats.
    #
    # @return [ SFTP::Stat ]
    def stat
      @session.fstat(self)
    end

    # Repositions the file pointer to the given offset (relative to the start of
    # the file). This will also reset the EOF flag.
    #
    # @param [ Int ] pos The offset position to set.
    #
    # @return [ Void ]
    def pos=(pos)
      seek(pos, pos < 0 ? :END : :SET)
    end

    # Positions to the beginning of input, resetting pos to zero.
    #
    # @return [ Int ]
    def rewind
      seek(0)
    end
  end
end
