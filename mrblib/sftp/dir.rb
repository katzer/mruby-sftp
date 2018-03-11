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
  # A convenience class for working with remote directories. It provides methods
  # for searching and enumerating directory entries.
  class Dir
    include Enumerable

    # Creates a new SFTP::Dir instance atop the given SFTP connection.
    #
    # @param [ SFTP::Session ] session The underlying SFTP session.
    #
    # @return [ Void ]
    def initialize(session)
      @session = session
    end

    # Calls the block once for each entry in the named directory on the remote
    # server. Yields the file name to the block.
    #
    # @param [ String ] path The path of the remote directory.
    # @param [ Proc ]   proc The block to yield.
    #
    # @return [ Void ]
    def foreach(path, &block)
      return to_enum(:each, path) unless block

      io = Handle.new(@session, path)
      io.open_dir

      loop { break unless (line = io.gets) && yield(line) }
    ensure
      io.close if io
    end

    # To be enumerable
    alias each foreach

    # Returns an array of names representing the items in the given remote dir.
    #
    # @param [ String ] path The path of the remote directory.
    #
    # @return [ Array<String> ]
    def entries(path)
      result = []

      foreach(path) { |name| result << name }

      result
    end
  end
end
