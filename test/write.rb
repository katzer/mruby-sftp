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
# #
# # The above copyright notice and this permission notice shall be included in all
# # copies or substantial portions of the Software.
# #
# # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# # SOFTWARE.

# SFTP.start('demo.wftpserver.com', 'demo-user', password: 'demo-user', port: 2222) do |sftp|
#   path = "upload/#{sftp.dir.entries('upload').first.name}"

#   assert 'SFTP::File#write' do
#     sftp.file.open(path, 'w') { |io| assert_equal 5, io.write('Hello') }
#     sftp.file.open(path) { |io| assert_equal 'Hello', io.gets(nil) }
#   end

#   assert 'SFTP::File#<<' do
#     sftp.file.open(path, 'w') { |io| assert_equal io, io << 'Hello' }
#     sftp.file.open(path) { |io| assert_equal 'Hello', io.gets(nil) }
#   end

#   assert 'SFTP::File#print' do
#     sftp.file.open(path, 'w') { |io| assert_nil io.print('Hello', 'World!') }
#     sftp.file.open(path) { |io| assert_equal 'HelloWorld!', io.gets(nil) }
#   end

#   assert 'SFTP::File#puts' do
#     sftp.file.open(path, 'w') { |io| assert_nil io.puts('Hello', 'World!') }
#     sftp.file.open(path) { |io| assert_equal "Hello\nWorld!\n", io.gets(nil) }
#   end

#   assert 'SFTP::Session#write' do
#     assert_equal 13, sftp.write(path, 'Session#write')
#     assert_equal 'Session#write', sftp.read(path)
#     assert_equal 5, sftp.write(path, 'write', nil, mode: 'a')
#     assert_equal 'Session#writewrite', sftp.read(path)
#     assert_equal 5, sftp.write(path, 'write', 1)
#     assert_equal 'Swriten#writewrite', sftp.read(path)
#   end
# end
