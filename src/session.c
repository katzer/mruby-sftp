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

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/ext/ssh.h"
#include "mruby/variable.h"

#include <libssh2_sftp.h>

static void
mrb_sftp_session_free (mrb_state *mrb, void *p)
{
    if (!p) return;
    libssh2_sftp_shutdown((LIBSSH2_SFTP *)p);
}

static mrb_data_type const mrb_sftp_session_type = { "SFTP::Session", mrb_sftp_session_free };

static mrb_ssh_t *
mrb_sftp_ssh_session (mrb_state *mrb, mrb_value self)
{
    return DATA_PTR(mrb_iv_get(mrb, self, mrb_intern_static(mrb, "@session", 8)));
}

static mrb_value
mrb_sftp_f_connect (mrb_state *mrb, mrb_value self)
{
    mrb_ssh_t *ssh = mrb_sftp_ssh_session(mrb, self);
    LIBSSH2_SFTP *sftp;
    char *err;

    if (DATA_PTR(self)) return mrb_nil_value();

    if (!ssh) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "SSH session not connected.");
    }

    if (!libssh2_userauth_authenticated(ssh->session)) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "SSH session not authenticated.");
    }

    sftp = libssh2_sftp_init(ssh->session);

    if (!sftp) {
        libssh2_session_last_error(ssh->session, &err, NULL, 0);
        mrb_raise(mrb, E_RUNTIME_ERROR, err);
    }

    mrb_data_init(self, sftp, &mrb_sftp_session_type);

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_close (mrb_state *mrb, mrb_value self)
{
    mrb_ssh_t *ssh = mrb_sftp_ssh_session(mrb, self);

    if (ssh) {
        mrb_sftp_session_free(mrb, DATA_PTR(self));
    }

    DATA_PTR(self)  = NULL;
    DATA_TYPE(self) = NULL;

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_closed (mrb_state *mrb, mrb_value self)
{
    return mrb_bool_value(DATA_PTR(self) ? FALSE : TRUE);
}

static mrb_value
mrb_sftp_f_last_errno (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP *sftp = DATA_PTR(self);
    int err;

    if (!sftp) return mrb_nil_value();

    err = libssh2_sftp_last_error(sftp);

    return mrb_fixnum_value(err);
}

void
mrb_mruby_sftp_session_init (mrb_state *mrb)
{
    struct RClass *ftp, *cls;

    ftp = mrb_module_get(mrb, "SFTP");
    cls = mrb_define_class_under(mrb, ftp, "Session", mrb->object_class);

    MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);

    mrb_define_method(mrb, cls, "connect", mrb_sftp_f_connect, MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "close",   mrb_sftp_f_close,   MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "closed?", mrb_sftp_f_closed,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "last_errno", mrb_sftp_f_last_errno, MRB_ARGS_NONE());
}
