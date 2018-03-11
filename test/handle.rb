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

dummy_sftp = SFTP::Session.new(SSH::Session.new)

assert 'SFTP::Handle' do
  assert_kind_of Class, SFTP::Handle
end

assert 'SFTP::Handle.new' do
  assert_raise(ArgumentError) { SFTP::Handle.new }
  assert_raise(ArgumentError) { SFTP::Handle.new dummy_sftp }
  assert_nothing_raised { SFTP::Handle.new dummy_sftp, '/' }
end

SFTP.start('test.rebex.net', 'demo', password: 'password') do |ftp|
  dummy = SFTP::Handle.new(dummy_sftp, 'readme.txt')
  file  = SFTP::Handle.new(ftp, 'readme.txt')
  dir   = SFTP::Handle.new(ftp, '/pub')

  assert 'SFTP::Handle#open_dir' do
    assert_false dummy.open?
    assert_raise(RuntimeError) { dummy.open_dir }
    assert_true dummy.closed?

    assert_false dir.open?
    assert_nothing_raised { dir.open_dir }
    assert_false dir.closed?
  end

  assert 'SFTP::Handle#open_file' do
    assert_false dummy.open?
    assert_raise(RuntimeError) { dummy.open_file }
    assert_true dummy.closed?

    assert_false file.open?
    assert_raise(RuntimeError) { file.open_file 'unknown flag' }
    assert_false file.open?
    assert_nothing_raised { file.open_file 'r' }
    assert_false file.closed?
  end

  assert 'SFTP::Handle#seek' do
    assert_raise(RuntimeError) { dummy.seek }

    file.open_file

    assert_equal 0,   file.seek(0)
    assert_equal 100, file.seek(100, :SET)
    assert_equal 200, file.seek(100, :CUR)
    assert_equal 190, file.seek(-10, :CUR)
    assert_equal 1,   file.seek(1)
  end

  assert 'SFTP::Handle#tell' do
    assert_raise(RuntimeError) { dummy.tell }

    file.open_file
    file.seek(1)

    assert_equal 1, file.tell
  end

  assert 'SFTP::Handle#rewind' do
    assert_raise(RuntimeError) { dummy.rewind }

    file.open_file
    file.seek(1)

    assert_equal 0, file.rewind
  end

  assert 'SFTP::Handle#close' do
    assert_nothing_raised { dummy.close }
    assert_nothing_raised { dir.close }
    assert_nothing_raised { file.close }
    assert_true dir.closed?
    assert_true file.closed?
  end
end
