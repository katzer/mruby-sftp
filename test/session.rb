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

assert 'SFTP::Session' do
  assert_kind_of Class, SFTP::Session
end

assert 'SFTP::Session.new' do
  assert_kind_of Class, SFTP::Session
  assert_raise(ArgumentError) { SFTP::Session.new }

  sftp = SFTP::Session.new(SSH::Session.new)
  assert_raise(RuntimeError) { sftp.connect }
  assert_false sftp.connected?
end

assert 'SSH::sftp' do
  SSH.start('test.rebex.net', 'demo', password: 'password') do |ssh|
    assert_kind_of SFTP::Session, ssh.sftp
    assert_true ssh.sftp.connected?
  end
end

assert 'SFTP::start' do
  session = nil

  SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
    session = sftp
    assert_kind_of SFTP::Session, sftp
    assert_true sftp.connected?
  end

  assert_true session.closed?
end

assert 'SFTP::Session#file' do
  sftp = SFTP::Session.new(SSH::Session.new)
  assert_kind_of SFTP::FileFactory, sftp.file
end

assert 'SFTP::Session#dir' do
  sftp = SFTP::Session.new(SSH::Session.new)
  assert_kind_of SFTP::Dir, sftp.dir
end
