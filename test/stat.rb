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

assert 'SFTP::Stat' do
  assert_kind_of Class, SFTP::Stat
  assert_include SFTP::Stat.constants, :T_DIRECTORY
  assert_include SFTP::Stat.constants, :T_REGULAR
end

assert 'SFTP::Stat#initialize' do
  assert_nothing_raised { SFTP::Stat.new }
  assert_nothing_raised { SFTP::Stat.new uid: 1 }
  assert_equal SFTP::Stat::T_UNKNOWN, SFTP::Stat.new.type

  attrs = { gid: 1, uid: 2, mtime: 3, atime: 4, mode: 5, type: 6, size: 7 }
  stats = SFTP::Stat.new(attrs)

  assert_equal attrs[:gid], stats.gid
  assert_equal attrs[:uid], stats.uid
  assert_equal attrs[:mtime], stats.mtime
  assert_equal attrs[:atime], stats.atime
  assert_equal attrs[:mode], stats.mode
  assert_equal SFTP::Stat::T_UNKNOWN, stats.type
  assert_nil stats.size
end

assert 'SFTP::Stat#gid=' do
  stats = SFTP::Stat.new

  assert_nil stats.gid
  stats.gid = 0
  assert_equal 0, stats.gid
end

assert 'SFTP::Stat#uid=' do
  stats = SFTP::Stat.new

  assert_nil stats.uid
  stats.uid = 0
  assert_equal 0, stats.uid
end

assert 'SFTP::Stat#mtime=' do
  stats = SFTP::Stat.new

  assert_nil stats.mtime
  stats.mtime = 0
  assert_equal 0, stats.mtime
end

assert 'SFTP::Stat#atime=' do
  stats = SFTP::Stat.new

  assert_nil stats.atime
  stats.atime = 0
  assert_equal 0, stats.atime
end

assert 'SFTP::Stat#mode=' do
  stats = SFTP::Stat.new

  assert_nil stats.mode
  stats.mode = 0
  assert_equal 0, stats.mode
end

assert 'SFTP::Stat#size=' do
  stats = SFTP::Stat.new

  assert_nil stats.size
  assert_raise(NoMethodError) { stats.size = 1 }
  assert_nil stats.size
end

assert 'SFTP::Stat#type=' do
  stats = SFTP::Stat.new

  assert_equal SFTP::Stat::T_UNKNOWN, stats.type
  assert_raise(NoMethodError) { stats.type = SFTP::Stat::T_REGULAR }
  assert_equal SFTP::Stat::T_UNKNOWN, stats.type
end

assert 'SFTP::Stat#type' do
  stats = SFTP::Stat.new

  assert_equal SFTP::Stat::T_UNKNOWN, stats.type
  stats.mode = 0o140000
  assert_equal SFTP::Stat::T_SOCKET, stats.type
  stats.mode = 0o120000
  assert_equal SFTP::Stat::T_SYMLINK, stats.type
  stats.mode = 0o100000
  assert_equal SFTP::Stat::T_REGULAR, stats.type
  stats.mode = 0o60000
  assert_equal SFTP::Stat::T_BLOCK_DEVICE, stats.type
  stats.mode = 0o40000
  assert_equal SFTP::Stat::T_DIRECTORY, stats.type
  stats.mode = 0o20000
  assert_equal SFTP::Stat::T_CHAR_DEVICE, stats.type
  stats.mode = 0o10000
  assert_equal SFTP::Stat::T_FIFO, stats.type
end

assert 'SFTP::Stat#ftype' do
  stats = SFTP::Stat.new

  assert_equal :unknown, stats.ftype
  stats.mode = 0o140000
  assert_equal :socket, stats.ftype
  stats.mode = 0o120000
  assert_equal :symlink, stats.ftype
  stats.mode = 0o100000
  assert_equal :regular, stats.ftype
  stats.mode = 0o60000
  assert_equal :block_device, stats.ftype
  stats.mode = 0o40000
  assert_equal :directory, stats.ftype
  stats.mode = 0o20000
  assert_equal :char_device, stats.ftype
  stats.mode = 0o10000
  assert_equal :fifo, stats.ftype
end

assert 'SFTP::Stat#directory?' do
  stats = SFTP::Stat.new

  assert_nil stats.directory?
  stats.mode = 0o40000
  assert_true stats.directory?
  stats.mode = 0o10000
  assert_false stats.directory?
  stats.mode = 0o0
  assert_nil stats.directory?
end

assert 'SFTP::Stat#file?' do
  stats = SFTP::Stat.new

  assert_nil stats.file?
  stats.mode = 0o100000
  assert_true stats.file?
  stats.mode = 0o10000
  assert_false stats.file?
  stats.mode = 0o0
  assert_nil stats.file?
end

assert 'SFTP::Stat#symlink?' do
  stats = SFTP::Stat.new

  assert_nil stats.symlink?
  stats.mode = 0o120000
  assert_true stats.symlink?
  stats.mode = 0o10000
  assert_false stats.symlink?
  stats.mode = 0o0
  assert_nil stats.symlink?
end

assert 'SFTP::Stat#zero?' do
  stats = SFTP::Stat.new

  assert_true stats.zero?
  stats.instance_variable_set :@size, 240
  assert_false stats.zero?
  stats.instance_variable_set :@size, 0
  assert_true stats.zero?
end

assert 'SFTP::Stat#readable?' do
  stats = SFTP::Stat.new mode: SFTP::Stat::P_USR_X

  assert_false stats.readable?
  stats.mode = SFTP::Stat::P_USR_R
  assert_true stats.readable?
  stats.mode = SFTP::Stat::P_GRP_R
  assert_true stats.readable?
  stats.mode = SFTP::Stat::P_OTH_R
  assert_true stats.readable?
  stats.mode = SFTP::Stat::P_OTH_RWX
  assert_true stats.readable?
end

assert 'SFTP::Stat', 'SFTP' do
  SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
    dummy = sftp.file.stat('I am bad')
    assert_equal SFTP::Stat::T_UNKNOWN, dummy.type

    dir = sftp.file.stat('/pub')
    assert_true dir.directory?
    assert_equal 0, dir.size

    file = sftp.file.stat('readme.txt')
    assert_true file.file?
    assert_equal 600, file.umode
    assert_equal 403, file.size
    assert_true file.readable?
    assert_true file.writable?
    assert_false file.executable?
  end
end
