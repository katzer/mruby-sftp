/* MIT License
 *
 * Copyright (c) Sebastian Katzer 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "session.h"
#include "handle.h"
#include "file.h"
#include "stat.h"

#include "mruby.h"

#include <libssh2_sftp.h>

void
mrb_mruby_sftp_gem_init (mrb_state *mrb)
{
    struct RClass *ftp = mrb_define_module(mrb, "SFTP");

    mrb_define_const(mrb, ftp, "NO_SUCH_FILE",   mrb_fixnum_value(LIBSSH2_FX_NO_SUCH_FILE));
    mrb_define_const(mrb, ftp, "NO_SUCH_PATH",   mrb_fixnum_value(LIBSSH2_FX_NO_SUCH_PATH));
    mrb_define_const(mrb, ftp, "PERMISSION",     mrb_fixnum_value(LIBSSH2_FX_PERMISSION_DENIED));
    mrb_define_const(mrb, ftp, "FILE_EXIST",     mrb_fixnum_value(LIBSSH2_FX_FILE_ALREADY_EXISTS));
    mrb_define_const(mrb, ftp, "WRITE_PROTECT",  mrb_fixnum_value(LIBSSH2_FX_WRITE_PROTECT));
    mrb_define_const(mrb, ftp, "WRITE_PROTECT",  mrb_fixnum_value(LIBSSH2_FX_WRITE_PROTECT));
    mrb_define_const(mrb, ftp, "OUT_OF_SPACE",   mrb_fixnum_value(LIBSSH2_FX_NO_SPACE_ON_FILESYSTEM));
    mrb_define_const(mrb, ftp, "OUT_OF_SPACE",   mrb_fixnum_value(LIBSSH2_FX_NO_SPACE_ON_FILESYSTEM));
    mrb_define_const(mrb, ftp, "DIR_NOT_EMPTY",  mrb_fixnum_value(LIBSSH2_FX_DIR_NOT_EMPTY));
    mrb_define_const(mrb, ftp, "NOT_A_DIR",      mrb_fixnum_value(LIBSSH2_FX_NOT_A_DIRECTORY));
    mrb_define_const(mrb, ftp, "INVALID_NAME",   mrb_fixnum_value(LIBSSH2_FX_INVALID_FILENAME));
    mrb_define_const(mrb, ftp, "LINK_LOOP",      mrb_fixnum_value(LIBSSH2_FX_LINK_LOOP));
    mrb_define_const(mrb, ftp, "NO_CONNECTION",  mrb_fixnum_value(LIBSSH2_FX_NO_CONNECTION));
    mrb_define_const(mrb, ftp, "EOF",            mrb_fixnum_value(LIBSSH2_FX_EOF));

    mrb_mruby_sftp_session_init(mrb);
    mrb_mruby_sftp_handle_init(mrb);
    mrb_mruby_sftp_stat_init(mrb);
    mrb_mruby_sftp_file_init(mrb);
}

void
mrb_mruby_sftp_gem_final (mrb_state *mrb)
{

}
