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
#include "mruby/error.h"
#include "mruby/variable.h"
#include "mruby/ext/sftp.h"

#include <libssh2_sftp.h>

inline void
mrb_sftp_raise_last_error (mrb_state *mrb, LIBSSH2_SFTP *sftp, const char* msg)
{
    int err = libssh2_sftp_last_error(sftp);

    mrb_sftp_raise(mrb, err, msg);
}

inline void
mrb_sftp_raise (mrb_state *mrb, int err, const char* msg)
{
    struct RClass *c;
    mrb_value exc;

    switch (err) {
    case LIBSSH2_FX_OK:
        return;
    case LIBSSH2_FX_PERMISSION_DENIED:
        c = E_SFTP_PERM_ERROR; break;
    case LIBSSH2_FX_NO_CONNECTION:
        c = E_SFTP_NOT_CONNECTED_ERROR; break;
    case LIBSSH2_FX_CONNECTION_LOST:
        c = E_SFTP_CONNECTION_LOST_ERROR; break;
    case LIBSSH2_FX_OP_UNSUPPORTED:
        c = E_SFTP_UNSUPPORTED_ERROR; break;
    case LIBSSH2_FX_NO_SUCH_FILE:
        c = E_SFTP_FILE_ERROR; break;
    case LIBSSH2_FX_NO_SUCH_PATH:
        c = E_SFTP_PATH_ERROR; break;
    case LIBSSH2_FX_NOT_A_DIRECTORY:
        c = E_SFTP_DIR_ERROR; break;
    case LIBSSH2_FX_INVALID_FILENAME:
        c = E_SFTP_NAME_ERROR; break;
    default:
        c = E_SFTP_ERROR; break;
    }

    exc = mrb_exc_new_str(mrb, c, mrb_str_new_cstr(mrb, msg));
    mrb_iv_set(mrb, exc, mrb_intern_static(mrb, "@errno", 6), mrb_fixnum_value(err));

    mrb_exc_raise(mrb, exc);
}

void
mrb_mruby_sftp_gem_init (mrb_state *mrb)
{
    struct RClass *ftp = mrb_define_module(mrb, "SFTP");

    mrb_define_const(mrb, ftp, "RENAME_OVERWRITE", mrb_fixnum_value(LIBSSH2_SFTP_RENAME_OVERWRITE));
    mrb_define_const(mrb, ftp, "RENAME_ATOMIC",    mrb_fixnum_value(LIBSSH2_SFTP_RENAME_ATOMIC));
    mrb_define_const(mrb, ftp, "RENAME_NATIVE",    mrb_fixnum_value(LIBSSH2_SFTP_RENAME_NATIVE));

    mrb_mruby_sftp_session_init(mrb);
    mrb_mruby_sftp_handle_init(mrb);
    mrb_mruby_sftp_file_init(mrb);
    mrb_mruby_sftp_stat_init(mrb);
}

void
mrb_mruby_sftp_gem_final (mrb_state *mrb)
{

}
