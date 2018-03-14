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
      @path    = path.freeze
    end

    # The path to the file or dir.
    #
    # @return [ String ]
    attr_reader :path

    # Opens a file on the remote server.
    #
    # @param [ String ] flags Determines how to open the file.
    # @param [ Int ]    mode  The mode in case of the file has to be created.
    #
    # @return [ Void ]
    def open(flags = 'r', mode = 0)
      if @session.file.lstat(path).directory?
        open_dir
      else
        open_file(flags, mode)
      end
    end

    # Returns true if the connection has been initialized.
    #
    # @return [ Boolean ] true if open, otherwise false.
    def open?
      !closed?
    end

    # Repositions the file pointer to the given offset (relative to the start of
    # the file). This will also reset the EOF flag.
    #
    # @param [ Int ] pos The offset position to set.
    #
    # @return [ Void ]
    def pos=(pos)
      seek(pos)
    end

    # Positions to the beginning of input, resetting pos to zero.
    #
    # @return [ Int ]
    def rewind
      seek(0)
    end

    # Reads a one-character string. Returns nil if called at end of file.
    #
    # @return [ String ]
    def getc
      read(1)
    end

    # Reads up to n bytes of data from the stream. Fewer bytes will be returned
    # if EOF is encountered before the requested number of bytes could be read.
    # Without an argument (or with a nil argument) all data to the end of the
    # file will be read and returned.
    #
    # @param [ Int ] bytes Number of bytes to read.
    #
    # @return [ String ]
    def read(bytes = nil)
      raise TypeError if bytes && !bytes.is_a?(Integer)
      gets(bytes)
    end

    # Same as gets, but raises EOFError if EOF is encountered before any data
    # could be read.
    #
    # @param [ String ] sep  The string where to seperate from next line.
    # @param [ Hash ]   opts Optional config settings { chomp: true }
    #
    # @return [ String ]
    def readline(sep = "\n", opts = nil)
      raise TypeError unless sep.is_a?(String) || sep.is_a?(Hash)
      line = gets(sep, opts)
      raise RuntimeError unless line
      line
    end

    # Reads all of the lines and returns them in an array.
    #
    # @param [ Object] sep Lines are separated by the optional sep. If sep is
    #                      nil, the rest of the stream is returned as a single
    #                      record. If the first argument is an integer, or an
    #                      optional second argument is given, the returning
    #                      string would not be longer than the given value in
    #                      bytes.
    # @param [ Hash ] opts Optional config settings { chomp: true }
    #
    # @return [ Array<String> ]
    def readlines(sep = "\n", opts = nil)
      lines = []
      loop { break unless (line = gets(sep, opts)) && lines << line }
      lines
    end
  end
end
