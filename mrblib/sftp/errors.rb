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
  # A general exception class, to act as the ancestor of all other SFTP
  # exception classes.
  class Exception < ::RuntimeError
    # Construct a new Exception object, optionally passing in a message
    # and a SFTP specific error code.
    #
    # @param [ String ] msg  Optional error message.
    # @param [ Int ]    code Optional SFTP error code.
    #
    # @return [ Void ]
    def initialize(msg = nil, errno = 0)
      super(msg)
      @errno = errno
    end

    # The SFTP error code.
    #
    # @return [ Int ]
    attr_reader :errno
  end

  # The user does not have sufficient permissions to perform the operation.
  class PermissionError < SFTP::Exception; end

  # An attempted operation could not be completed by the server because the
  # server does not support the operation. It may be returned by the server
  # if the server does not implement an operation.
  class Unsupported < SFTP::Exception; end

  # Raised if the SFTP handle is not opened to perform the action.
  class HandleNotOpened < SFTP::Exception; end

  # There is no SFTP connection to the server.
  class NotConnected < SFTP::Exception; end

  # The connection to the server was lost.
  class ConnectionLost < SFTP::Exception; end

  # A reference was made to a file which does not exist.
  class FileError < SFTP::Exception; end

  # The specified file is not a directory.
  class DirError < SFTP::Exception; end

  # The file path does not exist or is invalid.
  class PathError < SFTP::Exception; end

  # The filename is not valid.
  class NameError < SFTP::Exception; end
end
