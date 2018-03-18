# SFTP client for mruby <br> [![Build Status](https://travis-ci.org/katzer/mruby-sftp.svg?branch=master)](https://travis-ci.org/katzer/mruby-sftp) [![Build status](https://ci.appveyor.com/api/projects/status/pggp2jp2287ogqjm/branch/master?svg=true)](https://ci.appveyor.com/project/katzer/mruby-sftp/branch/master) [![codebeat badge](https://codebeat.co/badges/6314e973-d325-4366-a227-48e2023db7c2)](https://codebeat.co/projects/github-com-katzer-mruby-sftp-master)

Inspired by [Net::SFTP][net_sftp], empowers [mruby][mruby], a work in progress!

The SFTP client is based on [mruby-ssh][mruby_ssh] and [libssh2][libssh2]. The API design follows Net::SFTP as much as possible.

The resulting binary will be statically linked agains _libssh2_ and _mbedtls_. There are no external runtime dependencies and the code should run fine for Linux, Unix and Windows platform.

The snippet demontrates how to read a remote file line by line:

```ruby
SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  sftp.file.open('readme.txt', 'r') do |file|
    file.each_line(chomp: true) { |line| puts line }
  end
end
```

## Installation

Add the line below to your `build_config.rb`:

```ruby
MRuby::Build.new do |conf|
  # ... (snip) ...
  conf.gem 'mruby-sftp'
end
```

Or add this line to your aplication's `mrbgem.rake`:

```ruby
MRuby::Gem::Specification.new('your-mrbgem') do |spec|
  # ... (snip) ...
  spec.add_dependency 'mruby-sftp'
end
```

## Usage

To initiate a SFTP session it is recommended to use either `SFTP.open`

```ruby
SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  sftp # => SFTP::Session
end
```

or `SSH::Session#sftp`

```ruby
SSH.start('test.rebex.net', 'demo', key: '~/.ssh/id_rsa') do |session|
  session.sftp # => SFTP::Session
end
```

_SFTP.open_ works the same way like _SSH.open_. See the doc for [mruby-ssh][mruby_ssh] for how to connect and login to a SSH server.

### SFTP::Session

The Session class encapsulates a single SFTP channel on a SSH connection. Instances of this class are what most applications will interact with most, as it provides access to both low-level (mkdir, rename, remove, symlink, etc.) and high-level (upload, download, etc.) SFTP operations.

```ruby
SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  sftp.download('remote/file', 'local/file') if sftp.exist? 'remote/file'
  sftp.upload('local/file', ''remote/file'')
end
```

See [session.rb](mrblib/sftp/session.rb) and [session.c](src/session.c) for a complete list of available methods.

### SFTP::Stat

A class representing the attributes of a file or directory on the server. It may be used to specify new attributes, or to query existing attributes.

```ruby
SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  sftp.stat('remote/file').size
  sftp.lstat('remote/file').file?
  sftp.fstat(file).uid
  sftp.setstat('remote/file', gid: 104)
end
```

See [stat.rb](mrblib/sftp/stat.rb) and [stat.c](src/stat.c) for a complete list of available methods.

### SFTP::Dir

A convenience class for working with remote directories. It provides methods for searching and enumerating directory entries, similarly to the standard ::Dir class.

```ruby
SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  sftp.dir.foreach('/') do |entry|
    entry.name       # => 'readme.txt'
    entry.longname   # => '-rw------- 1 demo users        403 Apr 08  2014 readme.txt'
    entry.stats.size # => 403
  end
end
```

See [dir.rb](mrblib/sftp/dir.rb) for a complete list of available methods.

### SFTP::File

A wrapper around an SFTP file handle, that exposes an IO-like interface for interacting with the remote file.

```ruby
SFTP.start('test.rebex.net', 'demo', password: 'password') do |sftp|
  sftp.file.open('readme.txt', 'r') do |file|
    file.readline  # => Read first line
    file.gets(5)   # => Read next 5 characters
    file.gets(nil) # => Read until end of file
    file.eof?      # => true
  end
end
```

See [file_factory.rb](mrblib/sftp/file_factory.rb), [file.rb](mrblib/sftp/file.rb), [handle.rb](mrblib/sftp/handle.rb) and [handle.c](src/handle.c) for a complete list of available methods.


## TODO

- Upload/Download directories
- Asynchronous IO operations

## Development

Clone the repo:
    
    $ git clone https://github.com/katzer/mruby-sftp.git && cd mruby-sftp/

Compile the source:

    $ rake compile

Run the tests:

    $ rake test

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/katzer/mruby-sftp.

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request

## Authors

- Sebastián Katzer, Fa. appPlant GmbH

## License

The mgem is available as open source under the terms of the [MIT License][license].

Made with :yum: from Leipzig

© 2017 [appPlant GmbH][appplant]

[mruby]: https://github.com/mruby/mruby
[net_sftp]: https://github.com/net-ssh/net-sftp
[mruby_ssh]: https://github.com/katzer/mruby-ssh
[libssh2]: https://www.libssh2.org
[license]: http://opensource.org/licenses/MIT
[appplant]: www.appplant.de
