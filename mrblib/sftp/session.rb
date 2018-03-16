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
  class Session
    # Creates a new SFTP instance atop the given SSH connection.
    #
    # @param [ SSH::Session ] host Optional host name.
    #
    # @return [ SFTP::Session ]
    def initialize(session)
      @session = session
      connect if session.logged_in?
    end

    # A reference to the SSH session object that powers this SFTP session.
    #
    # @return [ SSH::Session ]
    attr_reader :session

    # If the socket is connected to the host.
    #
    # @return [ Boolean ]
    def connected?
      !closed?
    end

    # Returns an SFTP::FileFactory instance, which can be used to mimic
    # synchronous, IO-like file operations on a remote file via SFTP.
    #
    # @return [ SFTP::FileFactory ]
    def file
      FileFactory.new(self)
    end

    # Returns an SFTP::Dir instance, which can be used for searching and
    # enumerating entries on a remote directory via SFTP.
    #
    # @return [ SFTP::FileFactory ]
    def dir
      Dir.new(self)
    end

    # Initiates a download from remote to local. If local is omitted, downloads
    # the file to an in-memory buffer and returns the result as a string.
    #
    # @param [ String ] remote The path to the remote file to download.
    # @param [ String ] local  The path to where to save the downloaded file.
    #
    # @return [ String|Int ] The downloaded content if local was omitted.
    def download(remote, local = nil)
      file.open(remote, 'r') do |io|
        local ? io.download(local) : io.gets(nil)
      end
    end
  end
end
