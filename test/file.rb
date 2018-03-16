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

assert 'SFTP::File' do
  assert_kind_of Class, SFTP::File
end

SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  dummy = SFTP::File.new(dummy_sftp, 'readme.txt')
  file  = SFTP::File.new(sftp, 'readme.txt')

  assert 'SFTP::File#each' do
    called = 0

    file.each do |line|
      called += 1
      assert_kind_of String, line
      assert_equal "\n", line[-1]
    end

    assert_equal 10, called

    file.each(chomp: true) do |line|
      called += 1
      assert_kind_of String, line
      assert_not_equal "\n", line[-1]
    end

    assert_equal 20, called
  end

  assert 'SFTP::File#getc' do
    assert_raise(RuntimeError) { dummy.getc }
    assert_raise(ArgumentError) { dummy.getc(2) }

    file.open_file
    file.rewind

    assert_equal 1, file.getc.size
  end

  assert 'SFTP::File#read' do
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

  assert 'SFTP::File#readline' do
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

  assert 'SFTP::File#eof?' do
    assert_raise(RuntimeError) { dummy.eof? }

    file.open_file

    file.rewind
    assert_false file.eof?

    file.read
    assert_true file.eof?

    file.pos = 1
    assert_false file.eof?
  end

  assert 'SFTP::File#readlines' do
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

  assert 'SFTP::File#sync' do
    file.close
    assert_raise(RuntimeError) { file.sync }

    file.open
    assert_false file.sync
    assert_equal SFTP::UNSUPPORTED, sftp.last_errno
    file.close
  end
end
