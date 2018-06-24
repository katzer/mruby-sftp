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

dummy = SFTP::Session.new(SSH::Session.new)

assert 'SFTP::FileFactory' do
  assert_kind_of Class, SFTP::FileFactory
end

assert 'SFTP::FileFactory#initialize' do
  assert_raise(ArgumentError) { SFTP::FileFactory.new }
  assert_nothing_raised { SFTP::FileFactory.new(dummy) }
end

SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  assert 'SFTP::FileFactory#directory?' do
    assert_raise(SFTP::NotConnected) { dummy.file.directory? '/pub' }
    assert_raise(ArgumentError) { sftp.file.directory? }
    assert_true  sftp.file.directory? '/pub'
    assert_false sftp.file.directory? 'readme.txt'
    assert_false sftp.file.directory? 'I am wrong'
  end

  assert 'SFTP::FileFactory#open' do
    assert_raise(SFTP::NotConnected) { dummy.file.open 'readme.txt' }
    assert_raise(ArgumentError) { sftp.file.open }
    assert_raise(SFTP::FileError) { sftp.file.open 'readme txt' }

    io = sftp.file.open('readme.txt')

    assert_kind_of SFTP::File, io
    assert_true io.open?

    invoked = false

    sftp.file.open('readme.txt') do |file|
      invoked = true
      io      = file
      assert_kind_of SFTP::File, file
    end

    assert_true invoked
    assert_true io.closed?
  end
end
