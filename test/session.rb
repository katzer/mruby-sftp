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
  assert_raise(SSH::NotConnected) { sftp.connect }
  assert_false sftp.connected?
end

dummy   = SFTP::Session.new(SSH::Session.new)
tmp_dir = TEST_ARGS['TMP']
rand    = TEST_ARGS['RAND']

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

assert 'SFTP#connect' do
  ssh  = SSH::Session.new
  sftp = SFTP::Session.new(ssh)

  assert_false sftp.connected?
  assert_raise(SSH::NotConnected) { sftp.connect }
  assert_false sftp.connected?

  ssh.connect('test.rebex.net')
  assert_raise(SSH::NotAuthentificated) { sftp.connect }
  assert_false sftp.connected?

  ssh.login('demo', password: 'password')
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

  assert 'SFTP::Session#host' do
    assert_nil dummy.host
    assert_equal ssh.host, sftp.host
  end

  assert 'SFTP::Session#exist?' do
    assert_raise(SFTP::NotConnected) { dummy.exist? '/pub' }
    assert_raise(ArgumentError) { sftp.exist? }
    assert_true  sftp.exist? '/pub'
    assert_true  sftp.exist? 'readme.txt'
    assert_false sftp.exist? 'I am wrong'
  end

  assert 'SFTP::Session#realpath' do
    assert_raise(SFTP::NotConnected) { dummy.realpath('pub') }
    assert_raise(ArgumentError) { sftp.realpath }
    assert_equal '/pub', sftp.realpath('pub')
  end

  assert 'SFTP::Session#stat' do
    assert_raise(SFTP::NotConnected) { dummy.stat('/pub') }
    assert_raise(ArgumentError) { sftp.stat }
    assert_kind_of SFTP::Stat, sftp.stat('/pub')
  end

  assert 'SFTP::Session#lstat' do
    assert_raise(SFTP::NotConnected) { dummy.lstat('/pub') }
    assert_raise(ArgumentError) { sftp.lstat }
    assert_kind_of SFTP::Stat, sftp.lstat('/pub')
  end

  assert 'SFTP::Session#fstat' do
    assert_raise(SFTP::NotConnected) { dummy.fstat('/pub') }
    assert_raise(ArgumentError) { sftp.fstat }
    assert_kind_of SFTP::Stat, sftp.fstat('/pub')
    assert_true sftp.fstat('/pub').directory?
  end

  assert 'SFTP::Session#last_errno' do
    sftp.exist? 'I/am/wrong'
    assert_equal 2, sftp.last_errno
  end

  assert 'SFTP::Session#setstat' do
    assert_raise(SFTP::NotConnected) { dummy.setstat('readme.txt', uid: 1) }
    assert_raise(ArgumentError) { sftp.setstat }
    assert_raise(ArgumentError) { sftp.setstat 'readme.txt' }

    sftp.setstat('readme.txt', mode: 0o777)
  rescue SFTP::PermissionError => e
    skip(e)
  end

  assert 'SFTP::Session#delete' do
    assert_raise(SFTP::NotConnected) { dummy.delete('readme.txt') }
    assert_raise(ArgumentError) { sftp.delete }

    path = "#{rand}.txt"

    sftp.file.open(path, 'w+')
    assert_true sftp.exist? path

    sftp.delete(path)
    assert_false sftp.exist? path
  rescue SFTP::PermissionError => e
    skip(e)
  end

  assert 'SFTP::Session#mkdir' do
    assert_raise(SFTP::NotConnected) { dummy.mkdir('/dir') }
    assert_raise(ArgumentError) { sftp.mkdir }

    path = rand

    sftp.mkdir(path)
    assert_true sftp.stat(path).directory?
  rescue SFTP::PermissionError => e
    skip(e)
  end

  assert 'SFTP::Session#rmdir' do
    assert_raise(SFTP::NotConnected) { dummy.rmdir('/dir') }
    assert_raise(ArgumentError) { sftp.rmdir }

    path = rand

    sftp.mkdir(path) unless sftp.exist? path
    assert_true sftp.exist? path

    sftp.rmdir(path)
    assert_false sftp.exist? path
  rescue SFTP::PermissionError => e
    skip(e)
  end

  assert 'SFTP::Session#symlink' do
    assert_raise(SFTP::NotConnected) { dummy.symlink('readme.txt', 'link') }
    assert_raise(ArgumentError) { sftp.symlink }
    assert_raise(ArgumentError) { sftp.symlink('readme.txt') }

    path = rand
    link = "#{path}.link"

    sftp.file.open(path, 'w+')

    assert_false sftp.exist? link
    sftp.symlink(path, link)
    assert_true sftp.exist? link
    assert_true sftp.lstat(link).symlink?
    assert_false sftp.fstat(link).symlink?

    sftp.delete(path)
    sftp.delete(link)
  rescue SFTP::Exception => e
    skip(e)
  end

  assert 'SFTP::Session#rename' do
    assert_raise(SFTP::NotConnected) { dummy.rename('readme.txt', 'link') }
    assert_raise(ArgumentError) { sftp.rename }
    assert_raise(ArgumentError) { sftp.rename('readme.txt') }

    path     = rand
    path_new = "#{path}.new"

    sftp.file.open(path, 'w+')

    assert_false sftp.exist? path_new
    sftp.rename(path, path_new)
    assert_true  sftp.exist? path_new
    assert_false sftp.exist? path

    sftp.delete(path_new)
  rescue SFTP::PermissionError => e
    skip(e)
  end

  assert 'SFTP::Session#download' do
    assert_raise(SFTP::NotConnected) { dummy.download('readme.txt') }
    assert_raise(ArgumentError) { sftp.download }
    assert_raise(RuntimeError) { sftp.download('bad path', 'readme.txt') }
    assert_raise(RuntimeError) { sftp.download('readme.txt', 'bad/path') }

    content = sftp.download('readme.txt')
    size    = sftp.stat('readme.txt').size

    assert_kind_of String, content
    assert_equal size, content.size
    assert_equal size, sftp.download('readme.txt', "#{tmp_dir}/readme.tmp")
  end

  assert 'SFTP::Session#read' do
    assert_raise(SFTP::NotConnected) { dummy.read('readme.txt') }
    assert_raise(ArgumentError) { sftp.download }

    content = sftp.read('readme.txt')
    size    = sftp.stat('readme.txt').size

    assert_equal size, content.size
    assert_equal content[0, 5],  sftp.read('readme.txt', 5)
    assert_equal content[0, 5],  sftp.read('readme.txt', 5, 0)
    assert_equal content[1, 5],  sftp.read('readme.txt', 5, 1)
    assert_equal content[-5, 5], sftp.read('readme.txt', 5, -5)
    assert_equal content[1, 5],  sftp.read('readme.txt', 5, 1, mode: 'r')
  end

  assert 'SFTP#close' do
    ssh.close
    assert_false sftp.connected?
  end
end
