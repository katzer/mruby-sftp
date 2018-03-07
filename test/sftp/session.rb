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

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

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
    ssh.close
    ssh.sftp.close
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

# # assert 'SFTP#list' do
# #   SFTP.open('test.rebex.net') do |ftp|
# #     ftp.login('demo', 'password')
# #     assert_raise(TypeError)    { ftp.list 123 }
# #     assert_raise(RuntimeError) { ftp.list '~' }
# #     assert_nothing_raised { ftp.list }
# #     assert_nothing_raised { ftp.list '/' }
# #     assert_kind_of Array, ftp.list
# #     assert_false ftp.list.empty?
# #   end
# # end

# # assert 'SFTP#entries' do
# #   SFTP.open('test.rebex.net') do |ftp|
# #     ftp.login('demo', 'password')
# #     assert_raise(TypeError)    { ftp.entries 123 }
# #     assert_raise(RuntimeError) { ftp.entries '~' }
# #     assert_nothing_raised { ftp.entries }
# #     assert_nothing_raised { ftp.entries '/' }
# #     assert_kind_of Array, ftp.entries
# #     assert_false ftp.entries.empty?
# #     assert_not_include ftp.entries, '.'
# #     assert_not_include ftp.entries, '..'
# #     assert_not_equal '/', ftp.entries.first[0]
# #     assert_equal     '/', ftp.entries('/').first[0]
# #     assert_not_equal '/', ftp.entries('/').first[1]
# #     assert_not_equal ftp.entries('pub'), ftp.entries('pub/example')
# #     assert_equal 'pub/example/', ftp.entries('pub/example').first[0...12]
# #   end
# # end
