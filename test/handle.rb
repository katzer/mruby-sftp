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

  assert 'SFTP::Handle#pos' do
    assert_raise(RuntimeError) { dummy.pos }

    file.open_file

    file.seek(1)
    assert_equal 1, file.pos

    file.pos = 2
    assert_equal 2, file.pos
  end

  assert 'SFTP::Handle#rewind' do
    assert_raise(RuntimeError) { dummy.rewind }

    file.open_file
    file.seek(1)

    assert_equal 0, file.rewind
  end

  assert 'SFTP::Handle#read' do
    assert_raise(RuntimeError) { dummy.read }
    assert_raise(TypeError) { dummy.read("\n") }
    assert_raise(TypeError) { file.read(1.5) }

    file.open_file
    file.rewind

    assert_equal 10, file.read(10).size
    assert_equal 10, file.pos

    assert_not_equal file.read(5), file.read(5)

    assert_kind_of String, file.read
    assert_nil file.read
    assert_true file.eof?
  end

  assert 'SFTP::Handle#readline' do
    assert_raise(RuntimeError) { dummy.readline }
    assert_raise(TypeError) { dummy.readline(1) }
    assert_raise(TypeError) { dummy.readline(nil) }

    file.open_file
    file.rewind

    assert_equal     "\n", file.readline[-1]
    assert_equal     "\n", file.readline(chomp: false)[-1]
    assert_not_equal "\n", file.readline(chomp: true)[-1]

    file.read
    assert_true file.eof?
    assert_raise(RuntimeError) { file.readline }
  end

  assert 'SFTP::Handle#gets' do
    assert_raise(RuntimeError) { dummy.gets }

    file.open_file
    file.rewind

    assert_raise(TypeError) { file.gets(1.5) }

    assert_equal 10, file.gets(10).size
    assert_equal 10, file.pos

    assert_equal     "\n", file.gets[-1]
    assert_equal     "\n", file.gets(chomp: false)[-1]
    assert_not_equal "\n", file.gets(chomp: true)[-1]

    assert_not_equal file.gets(5), file.gets(5)

    assert_kind_of String, file.gets
    assert_false file.eof?

    file.gets(nil)
    assert_true file.eof?
    assert_nil file.gets
  end

  assert 'SFTP::Handle#getc' do
    assert_raise(RuntimeError) { dummy.getc }
    assert_raise(ArgumentError) { dummy.getc(2) }

    file.open_file
    file.rewind

    assert_equal 1, file.getc.size
  end

  assert 'SFTP::Handle#eof?' do
    assert_raise(RuntimeError) { dummy.eof? }

    file.open_file

    file.rewind
    assert_false file.eof?

    file.read
    assert_true file.eof?

    file.pos = 1
    assert_false file.eof?
  end

  assert 'SFTP::Handle#readlines' do
    assert_raise(RuntimeError) { dummy.readlines }
    assert_raise(TypeError) { file.readlines(1.5) }

    file.open_file

    file.rewind
    lines = file.readlines

    assert_kind_of Array, lines
    assert_equal "\n", lines[0][-1]
    assert_equal 10, lines.size
    assert_kind_of String, lines.last

    file.rewind
    lines = file.readlines(chomp: true)
    assert_kind_of Array, lines
    assert_not_equal "\n", lines[0][-1]
    assert_equal 10, lines.size
    assert_kind_of String, lines.last
  end

  assert 'SFTP::Handle#stat' do
    assert_raise(RuntimeError) { dummy.stat }
    assert_kind_of SFTP::Stat, file.stat
    assert_true file.stat.file?
  end

  assert 'SFTP::Handle#close' do
    dummy.close
    assert_true dummy.closed?

    file.close
    assert_true file.closed?

    ftp.close
    assert_true dir.closed?
  end
end
