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

#include "file_factory.h"

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/variable.h"

#include <libssh2_sftp.h>

static void
mrb_sftp_raise_unless_connected (mrb_state *mrb, LIBSSH2_SFTP *sftp)
{
    if (sftp) return;
    mrb_raise(mrb, E_RUNTIME_ERROR, "SFTP session not connected.");
}

static LIBSSH2_SFTP*
mrb_sftp_session (mrb_state *mrb, mrb_value self)
{
    return DATA_PTR(mrb_iv_get(mrb, self, mrb_intern_static(mrb, "@session", 8)));
}

static int
mrb_sftp_stat (mrb_state *mrb, mrb_value self, LIBSSH2_SFTP_ATTRIBUTES *attrs, int type)
{
    const char *path;
    mrb_int len;
    int ret;

    LIBSSH2_SFTP *sftp = mrb_sftp_session(mrb, self);
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
        mrb_iv_set(mrb, obj, mrb_intern_static(mrb, "@atime", 6), mrb_fixnum_value(attrs.atime));
        mrb_iv_set(mrb, obj, mrb_intern_static(mrb, "@mtime", 6), mrb_fixnum_value(attrs.mtime));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE) {
        mrb_iv_set(mrb, obj, mrb_intern_static(mrb, "@size", 5), mrb_fixnum_value(attrs.filesize));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
        mrb_iv_set(mrb, obj, mrb_intern_static(mrb, "@mode", 5), mrb_fixnum_value(attrs.permissions));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_UIDGID) {
        mrb_iv_set(mrb, obj, mrb_intern_static(mrb, "@uid", 4), mrb_fixnum_value(attrs.uid));
        mrb_iv_set(mrb, obj, mrb_intern_static(mrb, "@gid", 4), mrb_fixnum_value(attrs.gid));
    }

    return obj;
}

static mrb_value
mrb_sftp_f_init (mrb_state *mrb, mrb_value self)
{
    mrb_value session;

    mrb_get_args(mrb, "o", &session);
    mrb_iv_set(mrb, self, mrb_intern_static(mrb, "@session", 8), session);

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

    LIBSSH2_SFTP *sftp = mrb_sftp_session(mrb, self);

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

void
mrb_mruby_sftp_file_factory_init (mrb_state *mrb)
{
    struct RClass *ftp, *cls;

    ftp = mrb_module_get(mrb, "SFTP");
    cls = mrb_define_class_under(mrb, ftp, "FileFactory", mrb->object_class);

    mrb_define_method(mrb, cls, "initialize", mrb_sftp_f_init,  MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "exist?",     mrb_sftp_f_exist, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "realpath",   mrb_sftp_f_rpath, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "stat",       mrb_sftp_f_stat,  MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "lstat",      mrb_sftp_f_lstat, MRB_ARGS_REQ(1));
}
