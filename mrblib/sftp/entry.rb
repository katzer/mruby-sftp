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
  # Represents a single named item on the remote server.
  # This includes the name and the attributes.
  class Entry
    # Initializes a new instance of SFTP::Entry.
    #
    # @param [ String ]     name The name of the dir entry.
    # @param [ SFTP::Stat ] stat The stat object.
    #
    # @return [ Void ]
    def initialize(name, stat)
      @name  = name
      @stats = stat
    end

    # The name of the dir entry.
    #
    # @return [ String ]
    attr_reader :name

    # The stats of the dir entry.
    #
    # @return [ SFTP::Stat ]
    attr_reader :stats

    # Returns true if these entry appear to describe a file.
    #
    # @return [ Boolean ]
    def file?
      @stats.file?
    end

    # Returns true if these entry appear to describe a directory.
    #
    # @return [ Boolean ]
    def directory?
      @stats.directory?
    end

    # To look like a simple string.
    #
    # @return [ String ]
    def to_s
      name.to_s
    end
  end
end
