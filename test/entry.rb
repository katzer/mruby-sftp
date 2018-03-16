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

assert 'SFTP::Entry' do
  assert_kind_of Class, SFTP::Entry
end

assert 'SFTP::Entry#initialize' do
  assert_raise(ArgumentError) { SFTP::Entry.new }
  assert_raise(ArgumentError) { SFTP::Entry.new 'a' }
  assert_raise(ArgumentError) { SFTP::Entry.new 'a', 'aa' }
end

entry = SFTP::Entry.new 'readme.txt', 'rwx readme.txt', SFTP::Stat.new(uid: 123)

assert 'SFTP::Entry#name' do
  assert_equal 'readme.txt', entry.name
end

assert 'SFTP::Entry#longname' do
  assert_equal 'rwx readme.txt', entry.longname
end

assert 'SFTP::Entry#stats' do
  assert_kind_of SFTP::Stat, entry.stats
  assert_equal 123, entry.stats.uid
end
