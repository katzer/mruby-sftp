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

dummy = SFTP::Session.new(SSH::Session.new)

assert 'SFTP::Session#session' do
  assert_kind_of SSH::Session, dummy.session
end

assert 'SFTP::Session#file' do
  assert_kind_of SFTP::FileFactory, dummy.file
end

assert 'SFTP::Session#dir' do
  assert_kind_of SFTP::Dir, dummy.dir
end

assert 'SFTP.start' do
  session = nil

  SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
    session = sftp
    assert_kind_of SFTP::Session, sftp
    assert_true sftp.connected?
  end

  assert_true session.closed?
end

assert 'SSH#sftp' do
  SSH.start('test.rebex.net', 'demo', password: 'password') do |ssh|
    sftp = ssh.sftp
    assert_kind_of SFTP::Session, sftp
    assert_equal sftp, ssh.sftp
    assert_true sftp.connected?
  end
end

assert 'SFTP#connect' do
  ssh  = SSH::Session.new
  sftp = SFTP::Session.new(ssh)

  assert_false sftp.connected?
  assert_raise(RuntimeError) { sftp.connect }
  assert_false sftp.connected?

  ssh.connect('test.rebex.net')
  assert_raise(RuntimeError) { sftp.connect }
  assert_false sftp.connected?

  ssh.login('demo', 'password')
  assert_nothing_raised { sftp.connect }
  assert_true sftp.connected?

  SSH.shutdown
  assert_false sftp.connected?
end

SSH.start('test.rebex.net', 'demo', password: 'password') do |ssh|
  sftp = ssh.sftp

  assert 'SSH#sftp' do
    assert_kind_of SFTP::Session, sftp
    assert_equal sftp, ssh.sftp
    assert_true sftp.connected?
  end

  assert 'SFTP::Session#exist?' do
    assert_raise(RuntimeError) { dummy.exist? '/pub' }
    assert_raise(ArgumentError) { sftp.exist? }
    assert_true  sftp.exist? '/pub'
    assert_true  sftp.exist? 'readme.txt'
    assert_false sftp.exist? 'I am wrong'
  end

  assert 'SFTP::Session#realpath' do
    assert_raise(RuntimeError) { dummy.realpath('pub') }
    assert_raise(ArgumentError) { sftp.realpath }
    assert_equal '/pub', sftp.realpath('pub')
  end

  assert 'SFTP::Session#stat' do
    assert_raise(RuntimeError) { dummy.stat('/pub') }
    assert_raise(ArgumentError) { sftp.stat }
    assert_kind_of SFTP::Stat, sftp.stat('/pub')
  end

  assert 'SFTP::Session#lstat' do
    assert_raise(RuntimeError) { dummy.lstat('/pub') }
    assert_raise(ArgumentError) { sftp.lstat }
    assert_kind_of SFTP::Stat, sftp.lstat('/pub')
  end

  assert 'SFTP::Session#fstat' do
    assert_raise(RuntimeError) { dummy.fstat('/pub') }
    assert_raise(ArgumentError) { sftp.fstat }
    assert_kind_of SFTP::Stat, sftp.fstat('/pub')
    assert_true sftp.fstat('/pub').directory?
  end

  assert 'SFTP::Session#last_errno' do
    sftp.exist? 'I/am/wrong'
    assert_equal SFTP::NO_SUCH_FILE, sftp.last_errno
  end

  assert 'SFTP::Session#setstat', 'readonly server :(' do
    assert_raise(RuntimeError) { dummy.setstat('readme.txt', uid: 1) }
    assert_raise(ArgumentError) { sftp.setstat }
    assert_raise(ArgumentError) { sftp.setstat 'readme.txt' }
    assert_false sftp.setstat('readme.txt', uid: 1)
    assert_equal SFTP::PERMISSION_DENIED, sftp.last_errno
  end

  assert 'SFTP::Session#delete', 'readonly server :(' do
    assert_raise(RuntimeError) { dummy.delete('readme.txt') }
    assert_raise(ArgumentError) { sftp.delete }
    assert_false sftp.delete('readme.txt')
    assert_equal SFTP::PERMISSION_DENIED, sftp.last_errno
  end

  assert 'SFTP::Session#mkdir', 'readonly server :(' do
    assert_raise(RuntimeError) { dummy.mkdir('/dir') }
    assert_raise(ArgumentError) { sftp.mkdir }
    assert_false sftp.mkdir('/dir')
    assert_false sftp.mkdir('/dir', 755)
    assert_equal SFTP::PERMISSION_DENIED, sftp.last_errno
  end

  assert 'SFTP::Session#rmdir', 'readonly server :(' do
    assert_raise(RuntimeError) { dummy.rmdir('/dir') }
    assert_raise(ArgumentError) { sftp.rmdir }
    assert_false sftp.rmdir('/pub')
    assert_equal SFTP::PERMISSION_DENIED, sftp.last_errno
  end

  assert 'SFTP::Session#symlink', 'readonly server :(' do
    assert_raise(RuntimeError) { dummy.symlink('readme.txt', 'link') }
    assert_raise(ArgumentError) { sftp.symlink }
    assert_raise(ArgumentError) { sftp.symlink('readme.txt') }
    assert_false sftp.symlink('/readme.txt', '/readme_link.txt')
  end

  assert 'SFTP::Session#rename', 'readonly server :(' do
    assert_raise(RuntimeError) { dummy.rename('readme.txt', 'link') }
    assert_raise(ArgumentError) { sftp.rename }
    assert_raise(ArgumentError) { sftp.rename('readme.txt') }
    assert_false sftp.rename('readme.txt', 'readme2.txt')
    assert_equal SFTP::PERMISSION_DENIED, sftp.last_errno
    assert_false sftp.rename('readme.txt', 'readme2.txt', SFTP::RENAME_ATOMIC)
    assert_equal SFTP::PERMISSION_DENIED, sftp.last_errno
  end

  assert 'SFTP#close' do
    ssh.close
    assert_false sftp.connected?
  end
end
