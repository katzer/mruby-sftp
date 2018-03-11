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
  assert 'SFTP::FileFactory#exist?' do
    assert_raise(RuntimeError) { dummy.file.exist? '/pub' }
    assert_raise(ArgumentError) { sftp.file.exist? }
    assert_true  sftp.file.exist? '/pub'
    assert_true  sftp.file.exist? 'readme.txt'
    assert_false sftp.file.exist? 'I am wrong'
  end

  assert 'SFTP::FileFactory#realpath' do
    assert_raise(RuntimeError) { dummy.file.realpath('pub') }
    assert_raise(ArgumentError) { sftp.file.realpath }
    assert_equal '/pub', sftp.file.realpath('pub')
  end

  assert 'SFTP::FileFactory#stat' do
    assert_raise(RuntimeError) { dummy.file.stat('/pub') }
    assert_raise(ArgumentError) { sftp.file.stat }
    assert_kind_of SFTP::Stat, sftp.file.stat('/pub')
  end

  assert 'SFTP::FileFactory#lstat' do
    assert_raise(RuntimeError) { dummy.file.lstat('/pub') }
    assert_raise(ArgumentError) { sftp.file.lstat }
    assert_kind_of SFTP::Stat, sftp.file.lstat('/pub')
  end
end
