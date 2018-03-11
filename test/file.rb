# # MIT License
# #
# # Copyright (c) Sebastian Katzer 2017
# #
# # Permission is hereby granted, free of charge, to any person obtaining a copy
# # of this software and associated documentation files (the "Software"), to deal
# # in the Software without restriction, including without limitation the rights
# # to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# # copies of the Software, and to permit persons to whom the Software is
# # furnished to do so, subject to the following conditions:

# # The above copyright notice and this permission notice shall be included in all
# # copies or substantial portions of the Software.

# # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# # SOFTWARE.

# assert 'SFTP::File' do
#   assert_kind_of Class, SFTP::File
# end

# SFTP.start('demo.wftpserver.com', 'demo-user', password: 'demo-user', port: 2222) do |ftp|
#   assert 'SFTP::Session.file' do
#     assert_raise(ArgumentError) { ftp.file }
#     assert_kind_of SFTP::File, ftp.file('README.txt')
#   end

#   path     = 'download/manual_en.pdf'
#   file     = ftp.file(path)
#   dir      = ftp.file('download')
#   bad_file = ftp.file('i/dont/exist')

#   assert 'SFTP::File#path' do
#     assert_equal path, file.path
#   end

#   assert 'SFTP::File#realpath' do
#     assert_equal "/#{file.path}", file.realpath
#     assert_equal "/#{dir.path}",  dir.realpath
#   end

#   assert 'SFTP::File#file?' do
#     assert_raise(RuntimeError) { bad_file.file? }
#     assert_true  file.file?
#     assert_false dir.file?
#   end

#   assert 'SFTP::File#dir?' do
#     assert_raise(RuntimeError) { bad_file.dir? }
#     assert_true  dir.dir?
#     assert_false file.dir?
#   end

#   assert 'SFTP::File#exist?' do
#     assert_nothing_raised { bad_file.exist? }
#     assert_true  dir.exist?
#     assert_true  file.exist?
#     assert_false bad_file.exist?
#   end

#   assert 'SFTP::File#mtime' do
#     mtime = file.mtime
#     assert_raise(RuntimeError) { bad_file.mtime }
#     assert_kind_of Integer, mtime
#     assert_true mtime > 0
#   end

#   assert 'SFTP::File#atime' do
#     atime = file.atime
#     assert_raise(RuntimeError) { bad_file.atime }
#     assert_kind_of Integer, atime
#     assert_true atime > 0
#   end

#   assert 'SFTP::File#uid' do
#     uid = file.uid
#     assert_raise(RuntimeError) { bad_file.uid }
#     assert_kind_of Integer, uid
#     assert_true uid >= 0
#   end

#   assert 'SFTP::File#gid' do
#     gid = file.gid
#     assert_raise(RuntimeError) { bad_file.gid }
#     assert_kind_of Integer, gid
#     assert_true gid >= 0
#   end

#   assert 'SFTP::File#size' do
#     size = file.size
#     assert_raise(RuntimeError) { bad_file.size }
#     assert_kind_of Integer, size
#     assert_true size > 0
#     assert_true dir.zero?
#     assert_false file.zero?
#   end

#   assert 'SFTP::File#permissions' do
#     perms = file.permissions
#     assert_raise(RuntimeError) { bad_file.permissions }
#     assert_kind_of Integer, perms
#     assert_true perms > 0
#     assert_false perms > 777
#   end

#   assert 'SFTP::File#stat' do
#     attrs = file.stat
#     assert_raise(RuntimeError) { bad_file.stat }
#     assert_kind_of SFTP::Attributes, attrs
#     assert_equal file.permissions, attrs.permissions
#     assert_equal file.atime, attrs.atime
#     assert_equal file.mtime, attrs.mtime
#     assert_equal file.size, attrs.size
#     assert_equal file.uid, attrs.uid
#     assert_equal file.gid, attrs.gid
#     assert_equal 0, dir.stat.size
#   end
# end
