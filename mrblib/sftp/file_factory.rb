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
  # A factory class for opening files and returning SFTP::File instances that
  # wrap the SFTP handles that represent them.
  class FileFactory
    # Creates a new SFTP::FileFactory instance atop the given SFTP connection.
    #
    # @param [ SFTP::Session ] session The underlying SFTP session.
    #
    # @return [ Void ]
    def initialize(session)
      @session = session
    end

    # Opens a file on the remote server.
    #
    # @param [ String ] path  The path to the remote file.
    # @param [ String ] flags Determines how to open the file.
    # @param [ Int ]    mode  The mode in case of the file has to be created.
    #
    # @return [ Void ]
    def open(path, flags = 'r', mode = 0)
      io = SFTP::File.new(@session, path)

      io.open(flags, mode)

      return io unless block_given?

      begin
        yield(io)
      ensure
        io.close
      end
    end

    # Returns true if the argument refers to a directory on the remote host.
    #
    # @param [ String ] path The path to the remote file.
    #
    # @return [ Boolean ]
    def directory?(path)
      @session.lstat(path).directory?
    end
  end
end
