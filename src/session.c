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
#include "mruby/ext/sftp.h"
#include "mruby/variable.h"

#include <stdlib.h>
#include <libssh2_sftp.h>

static mrb_sym ATIME;
static mrb_sym MTIME;
static mrb_sym SIZE;
static mrb_sym MODE;
static mrb_sym UID;
static mrb_sym GID;

static void
mrb_sftp_session_free (mrb_state *mrb, void *p)
{
    mrb_sftp_t *data;

    if (!p) return;

    data = (mrb_sftp_t *)p;

    if (data->sftp && data->session->data && mrb_ssh_initialized()) {
        libssh2_sftp_shutdown(data->sftp);
    }

    free(data);
}

static mrb_data_type const mrb_sftp_session_type = { "SFTP::Session", mrb_sftp_session_free };

LIBSSH2_SFTP*
mrb_sftp_session (mrb_value self)
{
    mrb_sftp_t *data = DATA_PTR(self);
    return (data) ? data->sftp : NULL;
}

static void
mrb_sftp_raise_unless_connected (mrb_state *mrb, LIBSSH2_SFTP *sftp)
{
    if (sftp && mrb_ssh_initialized()) return;
    mrb_raise(mrb, E_RUNTIME_ERROR, "SFTP session not connected.");
}

static int
mrb_sftp_stat (mrb_state *mrb, mrb_value self, LIBSSH2_SFTP_ATTRIBUTES *attrs, int type)
{
    const char *path;
    mrb_int len;
    int ret;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "s", &path, &len);

    while ((ret = libssh2_sftp_stat_ex(sftp, path, len, type, attrs)) == LIBSSH2_ERROR_EAGAIN);

    return ret;
}

static mrb_value
mrb_sftp_stat_obj (mrb_state *mrb, LIBSSH2_SFTP_ATTRIBUTES attrs)
{
    struct RClass *ftp, *cls;
    mrb_value obj;

    ftp = mrb_module_get(mrb, "SFTP");
    cls = mrb_class_get_under(mrb, ftp, "Stat");
    obj = mrb_obj_new(mrb, cls, 0, NULL);

    if (attrs.flags & LIBSSH2_SFTP_ATTR_ACMODTIME) {
        mrb_iv_set(mrb, obj, ATIME, mrb_fixnum_value(attrs.atime));
        mrb_iv_set(mrb, obj, MTIME, mrb_fixnum_value(attrs.mtime));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE) {
        mrb_iv_set(mrb, obj, SIZE, mrb_fixnum_value(attrs.filesize));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
        mrb_iv_set(mrb, obj, MODE, mrb_fixnum_value(attrs.permissions));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_UIDGID) {
        mrb_iv_set(mrb, obj, UID, mrb_fixnum_value(attrs.uid));
        mrb_iv_set(mrb, obj, GID, mrb_fixnum_value(attrs.gid));
    }

    return obj;
}

static mrb_value
mrb_sftp_f_connect (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP *sftp;
    mrb_sftp_t *data;
    mrb_ssh_t *ssh;
    mrb_value session;
    char *err;

    if (DATA_PTR(self)) return mrb_nil_value();

    session = mrb_attr_get(mrb, self, mrb_intern_static(mrb, "@session", 8));
    ssh     = DATA_PTR(session);

    if (!(ssh && mrb_ssh_initialized())) {
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

    data          = malloc(sizeof(mrb_sftp_t));
    data->session = mrb_ptr(session);
    data->sftp    = sftp;

    mrb_data_init(self, data, &mrb_sftp_session_type);

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_exist (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int err = mrb_sftp_stat(mrb, self, &attrs, LIBSSH2_SFTP_STAT);

    switch(err) {
        case LIBSSH2_FX_OK:
            return mrb_true_value();
        case LIBSSH2_FX_NO_SUCH_FILE:
        case LIBSSH2_FX_NO_SUCH_PATH:
        case LIBSSH2_FX_INVALID_FILENAME:
            return mrb_false_value();
    }

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_rpath (mrb_state *mrb, mrb_value self)
{
    const char *path;
    char rpath[256];
    mrb_int len;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);

    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "s", &path, &len);

    libssh2_sftp_symlink_ex(sftp, path, len, rpath, 256, LIBSSH2_SFTP_REALPATH);

    return mrb_str_new_cstr(mrb, rpath);
}

static mrb_value
mrb_sftp_f_stat (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    mrb_sftp_stat(mrb, self, &attrs, LIBSSH2_SFTP_STAT);

    return mrb_sftp_stat_obj(mrb, attrs);
}

static mrb_value
mrb_sftp_f_lstat (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    mrb_sftp_stat(mrb, self, &attrs, LIBSSH2_SFTP_LSTAT);

    return mrb_sftp_stat_obj(mrb, attrs);
}

static mrb_value
mrb_sftp_f_close (mrb_state *mrb, mrb_value self)
{
    mrb_sftp_session_free(mrb, DATA_PTR(self));

    DATA_PTR(self)  = NULL;
    DATA_TYPE(self) = NULL;

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_closed (mrb_state *mrb, mrb_value self)
{
    mrb_sftp_t *data = DATA_PTR(self);

    if (!(data && mrb_ssh_initialized()))
        return mrb_true_value();

    return mrb_bool_value(data->session->data ? FALSE : TRUE);
}

static mrb_value
mrb_sftp_f_last_errno (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
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

    ATIME = mrb_intern_static(mrb, "@atime", 6);
    MTIME = mrb_intern_static(mrb, "@mtime", 6);
    SIZE  = mrb_intern_static(mrb, "@size", 5);
    MODE  = mrb_intern_static(mrb, "@mode", 5);
    UID   = mrb_intern_static(mrb, "@uid", 4);
    GID   = mrb_intern_static(mrb, "@gid", 4);

    mrb_define_method(mrb, cls, "connect",  mrb_sftp_f_connect, MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "exist?",   mrb_sftp_f_exist,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "realpath", mrb_sftp_f_rpath,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "stat",     mrb_sftp_f_stat,    MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "lstat",    mrb_sftp_f_lstat,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "close",    mrb_sftp_f_close,   MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "closed?",  mrb_sftp_f_closed,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "last_errno", mrb_sftp_f_last_errno, MRB_ARGS_NONE());
}
