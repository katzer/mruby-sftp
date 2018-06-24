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
#include "stat.h"

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/ext/ssh.h"
#include "mruby/ext/sftp.h"

#include <stdlib.h>
#include <libssh2_sftp.h>

static void
mrb_sftp_session_free (mrb_state *mrb, void *p)
{
    mrb_sftp_t *data;

    if (!p) return;

    data = (mrb_sftp_t *)p;

    if (data->sftp && data->session->data && mrb_ssh_initialized()) {
        libssh2_sftp_shutdown(data->sftp);
    }

    mrb_free(mrb, data);
}

static mrb_data_type const mrb_sftp_session_type = { "SFTP::Session", mrb_sftp_session_free };

LIBSSH2_SFTP *
mrb_sftp_session (mrb_value self)
{
    mrb_sftp_t *data = DATA_PTR(self);
    return (data) ? data->sftp : NULL;
}

mrb_ssh_t *
mrb_sftp_ssh_session (mrb_value self)
{
    mrb_sftp_t *data = DATA_PTR(self);
    return data && data->session->data ? (mrb_ssh_t *)data->session->data : NULL;
}

static void
mrb_sftp_raise_unless_connected (mrb_state *mrb, LIBSSH2_SFTP *sftp)
{
    if (sftp && mrb_ssh_initialized()) return;
    mrb_raise(mrb, E_SFTP_NOT_CONNECTED_ERROR, "SFTP session not connected.");
}

static int
mrb_sftp_stat (mrb_state *mrb, mrb_value self, LIBSSH2_SFTP_ATTRIBUTES *attrs, int type)
{
    mrb_value obj;
    int ret;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
    LIBSSH2_SFTP_HANDLE *handle;

    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "o", &obj);

    if (mrb_string_p(obj)) {
        while ((ret = libssh2_sftp_stat_ex(sftp, RSTRING_PTR(obj), RSTRING_LEN(obj), type, attrs)) == LIBSSH2_ERROR_EAGAIN);
        goto done;
    }

    handle = mrb_sftp_handle(mrb, obj);

    if (!handle) {
        mrb_raise(mrb, E_SFTP_HANDLE_CLOSED_ERROR, "SFTP handle not opened.");
    }

    while ((ret = libssh2_sftp_fstat(handle, attrs)) == LIBSSH2_ERROR_EAGAIN);

  done:

    if (ret == LIBSSH2_ERROR_SFTP_PROTOCOL) {
        ret = libssh2_sftp_last_error(sftp);
    }

    return ret;
}

static mrb_value
mrb_sftp_f_connect (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP *sftp;
    mrb_sftp_t *data;
    mrb_ssh_t *ssh;
    mrb_value session;

    if (DATA_PTR(self)) return mrb_nil_value();

    session = mrb_attr_get(mrb, self, mrb_intern_static(mrb, "@session", 8));
    ssh     = DATA_PTR(session);

    if (!(ssh && mrb_ssh_initialized())) {
        mrb_raise(mrb, E_SSH_NOT_CONNECTED_ERROR, "SSH session not connected.");
    }

    if (!libssh2_userauth_authenticated(ssh->session)) {
        mrb_raise(mrb, E_SSH_NOT_AUTH_ERROR, "SSH session not authenticated.");
    }

    do {
        sftp = libssh2_sftp_init(ssh->session);

        if (sftp) break;

        if (libssh2_session_last_errno(ssh->session) == LIBSSH2_ERROR_EAGAIN) {
            mrb_ssh_wait_socket(ssh);
        } else {
            mrb_ssh_raise_last_error(mrb, ssh);
        }
    } while (!sftp);

    data          = mrb_malloc(mrb, sizeof(mrb_sftp_t));
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

    while (libssh2_sftp_symlink_ex(sftp, path, len, rpath, 256, LIBSSH2_SFTP_REALPATH) == LIBSSH2_ERROR_EAGAIN);

    return mrb_str_new_cstr(mrb, rpath);
}

static mrb_value
mrb_sftp_f_stat (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int ret = mrb_sftp_stat(mrb, self, &attrs, LIBSSH2_SFTP_STAT);

    return mrb_sftp_stat_obj(mrb, ret == LIBSSH2_FX_OK ? &attrs : NULL);
}

static mrb_value
mrb_sftp_f_lstat (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int ret = mrb_sftp_stat(mrb, self, &attrs, LIBSSH2_SFTP_LSTAT);

    return mrb_sftp_stat_obj(mrb, ret == LIBSSH2_FX_OK ? &attrs : NULL);
}

static mrb_value
mrb_sftp_f_fstat (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int ret = mrb_sftp_stat(mrb, self, &attrs, LIBSSH2_SFTP_STAT);

    return mrb_sftp_stat_obj(mrb, ret == LIBSSH2_FX_OK ? &attrs : NULL);
}

static mrb_value
mrb_sftp_f_setstat (mrb_state *mrb, mrb_value self)
{
    const char *path;
    mrb_int path_len;
    mrb_value opts;
    int ret;

    LIBSSH2_SFTP_ATTRIBUTES attrs;
    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "sH", &path, &path_len, &opts);

    mrb_sftp_hash_to_stat(mrb, opts, &attrs);

    while ((ret = libssh2_sftp_stat_ex(sftp, path, path_len, LIBSSH2_SFTP_SETSTAT, &attrs)) == LIBSSH2_ERROR_EAGAIN);

    if (ret != 0) {
        mrb_sftp_raise_last_error(mrb, sftp, "Failed to set the stats as specified.");
    }

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_rename (mrb_state *mrb, mrb_value self)
{
    const char *source, *dest;
    mrb_int source_len, dest_len;
    mrb_int flags = LIBSSH2_SFTP_RENAME_OVERWRITE | LIBSSH2_SFTP_RENAME_ATOMIC | LIBSSH2_SFTP_RENAME_NATIVE;
    int ret;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "ss|i", &source, &source_len, &dest, &dest_len, &flags);

    while ((ret = libssh2_sftp_rename_ex(sftp, source, source_len, dest, dest_len, flags)) == LIBSSH2_ERROR_EAGAIN);

    if (ret != 0) {
        mrb_sftp_raise_last_error(mrb, sftp, "Failed to rename the file or dir as specified.");
    }

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_symlink (mrb_state *mrb, mrb_value self)
{
    const char *path;
    char *target;
    mrb_int path_len, target_len;
    int ret;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "ss", &path, &path_len, &target, &target_len);

    while ((ret = libssh2_sftp_symlink_ex(sftp, path, path_len, target, target_len, LIBSSH2_SFTP_SYMLINK)) == LIBSSH2_ERROR_EAGAIN);

    if (ret != 0) {
        mrb_sftp_raise_last_error(mrb, sftp, "Failed to create the symlink specified.");
    }

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_rmdir (mrb_state *mrb, mrb_value self)
{
    const char *path;
    mrb_int path_len;
    int ret;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "s", &path, &path_len);

    while ((ret = libssh2_sftp_rmdir_ex(sftp, path, path_len)) == LIBSSH2_ERROR_EAGAIN);

    if (ret != 0) {
        mrb_sftp_raise_last_error(mrb, sftp, "Failed to remove the dir specified.");
    }

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_mkdir (mrb_state *mrb, mrb_value self)
{
    const char *path;
    mrb_int path_len, mode = 0000733;
    int ret;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "s|i", &path, &path_len, &mode);

    while ((ret = libssh2_sftp_mkdir_ex(sftp, path, path_len, mode)) == LIBSSH2_ERROR_EAGAIN);

    if (ret != 0) {
        mrb_sftp_raise_last_error(mrb, sftp, "Failed to create the dir specified.");
    }

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_delete (mrb_state *mrb, mrb_value self)
{
    const char *path;
    mrb_int path_len;
    int ret;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(self);
    mrb_sftp_raise_unless_connected(mrb, sftp);

    mrb_get_args(mrb, "s", &path, &path_len);

    while ((ret = libssh2_sftp_unlink_ex(sftp, path, path_len)) == LIBSSH2_ERROR_EAGAIN);

    if (ret != 0) {
        mrb_sftp_raise_last_error(mrb, sftp, "Failed to delete the file specified.");
    }

    return mrb_nil_value();
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

    mrb_define_method(mrb, cls, "connect",  mrb_sftp_f_connect, MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "exist?",   mrb_sftp_f_exist,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "realpath", mrb_sftp_f_rpath,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "stat",     mrb_sftp_f_stat,    MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "lstat",    mrb_sftp_f_lstat,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "fstat",    mrb_sftp_f_fstat,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "setstat",  mrb_sftp_f_setstat, MRB_ARGS_REQ(2));
    mrb_define_method(mrb, cls, "rename",   mrb_sftp_f_rename,  MRB_ARGS_ARG(2,1));
    mrb_define_method(mrb, cls, "symlink",  mrb_sftp_f_symlink, MRB_ARGS_REQ(2));
    mrb_define_method(mrb, cls, "rmdir",    mrb_sftp_f_rmdir,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "mkdir",    mrb_sftp_f_mkdir,   MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, cls, "delete",   mrb_sftp_f_delete,  MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "close",    mrb_sftp_f_close,   MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "closed?",  mrb_sftp_f_closed,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "last_errno", mrb_sftp_f_last_errno, MRB_ARGS_NONE());
}
