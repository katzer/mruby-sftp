# MIT License
#
# Copyright (c) Sebastian Katzer 2017
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the 'Software'), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

dummy = SFTP::Session.new(SSH::Session.new)

assert 'SFTP::Dir' do
  assert_kind_of Class, SFTP::Dir
end

assert 'SFTP::Dir#initialize' do
  assert_raise(ArgumentError) { SFTP::Dir.new }
  assert_nothing_raised { SFTP::Dir.new(dummy) }
end

SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  assert 'SFTP::Dir#foreach' do
    called = false

    assert_raise(ArgumentError) { sftp.dir.foreach }
    assert_raise(RuntimeError) { sftp.dir.foreach('i am bad') {} }
    assert_raise(RuntimeError) { sftp.dir.foreach('i/am/bad') {} }
    assert_raise(RuntimeError) { sftp.dir.foreach('readme.txt') {} }

    sftp.dir.foreach('/') do |entry|
      called = true
      assert_kind_of SFTP::Entry, entry
      assert_kind_of String, entry.name
      assert_kind_of SFTP::Stat, entry.stats
    end

    assert_true called
  end

  assert 'SFTP::Dir#entries' do
    assert_raise(ArgumentError) { sftp.dir.entries }
    assert_raise(RuntimeError) { sftp.dir.entries 'i am bad' }
    assert_raise(RuntimeError) { sftp.dir.entries 'i/am/bad' }
    assert_raise(RuntimeError) { sftp.dir.entries 'readme.txt' }
    assert_equal 5, sftp.dir.entries('/').size
    assert_include sftp.dir.entries('/').map! { |e| e.name }, 'readme.txt'
  end
end
