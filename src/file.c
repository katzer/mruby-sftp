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

#include "file.h"

// #include "mruby.h"
// #include "mruby/data.h"
// #include "mruby/variable.h"

// #include <libssh2_sftp.h>

// static void
// mrb_sftp_raise_unless_connected (mrb_state *mrb, LIBSSH2_SFTP *sftp)
// {
//     if (sftp) return;
//     mrb_raise(mrb, E_RUNTIME_ERROR, "SFTP session not connected.");
// }

// static const char*
// mrb_sftp_file_path (mrb_state *mrb, mrb_value self, int *len)
// {
//     mrb_value path = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@path"));

//     if (len) {
//         *len = mrb_string_value_len(mrb, path);
//     }

//     return mrb_string_value_ptr(mrb, path);
// }

// static LIBSSH2_SFTP*
// mrb_sftp_file_sftp (mrb_state *mrb, mrb_value self)
// {
//     return ((struct RData *)DATA_PTR(self))->data;
// }

// static int
// mrb_sftp_file_attrs (mrb_state *mrb, mrb_value self, LIBSSH2_SFTP_ATTRIBUTES *attrs, int type, int raise)
// {
//     LIBSSH2_SFTP *sftp;
//     const char *path;
//     int len, ret;

//     sftp = mrb_sftp_file_sftp(mrb, self);
//     mrb_sftp_raise_unless_connected(mrb, sftp);
//     path = mrb_sftp_file_path(mrb, self, &len);

//     while ((ret = libssh2_sftp_stat_ex(sftp, path, len, type, attrs)) == LIBSSH2_ERROR_EAGAIN);

//     if (ret && raise) {
//         mrb_raise(mrb, E_RUNTIME_ERROR, "Unable to get status about the file.");
//     }

//     return ret;
// }

// static mrb_value
// mrb_sftp_file_fill_attrs (mrb_state *mrb, LIBSSH2_SFTP_ATTRIBUTES attrs)
// {
//     struct RClass *ftp, *cls;
//     mrb_value obj;

//     ftp = mrb_module_get(mrb, "SFTP");
//     cls = mrb_class_get_under(mrb, ftp, "Stat");
//     obj = mrb_obj_new(mrb, cls, 0, NULL);

//     if (attrs.flags & LIBSSH2_SFTP_ATTR_ACMODTIME) {
//         mrb_iv_set(mrb, obj, mrb_intern_lit(mrb, "@atime"), mrb_fixnum_value(attrs.atime));
//         mrb_iv_set(mrb, obj, mrb_intern_lit(mrb, "@mtime"), mrb_fixnum_value(attrs.mtime));
//     }

//     if (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE) {
//         mrb_iv_set(mrb, obj, mrb_intern_lit(mrb, "@size"), mrb_fixnum_value(attrs.filesize));
//     }

//     if (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
//         mrb_iv_set(mrb, obj, mrb_intern_lit(mrb, "@mode"), mrb_fixnum_value(attrs.permissions));
//     }

//     if (attrs.flags & LIBSSH2_SFTP_ATTR_UIDGID) {
//         mrb_iv_set(mrb, obj, mrb_intern_lit(mrb, "@uid"), mrb_fixnum_value(attrs.uid));
//         mrb_iv_set(mrb, obj, mrb_intern_lit(mrb, "@gid"), mrb_fixnum_value(attrs.gid));
//     }

//     return obj;
// }

// static mrb_value
// mrb_sftp_file_perm (mrb_state *mrb, mrb_value self, int type, int perm)
// {
//     LIBSSH2_SFTP_ATTRIBUTES attrs;
//     mrb_sftp_file_attrs(mrb, self, &attrs, type, 1);

//     if (!(attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS))
//         return mrb_nil_value();

//     if (attrs.permissions & perm)
//         return mrb_true_value();

//     return mrb_false_value();
// }

// static mrb_value
// mrb_sftp_file_f_init (mrb_state *mrb, mrb_value self)
// {
//     mrb_value session, path;

//     mrb_get_args(mrb, "oS", &session, &path);

//     mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@path"), path);
//     mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@session"), session);

//     DATA_PTR(self) = RDATA(session);

//     return mrb_nil_value();
// }

// static mrb_value
// mrb_sftp_file_f_exist (mrb_state *mrb, mrb_value self)
// {
//     LIBSSH2_SFTP_ATTRIBUTES attrs;
//     int err;

//     err = mrb_sftp_file_attrs(mrb, self, &attrs, LIBSSH2_SFTP_STAT, 0);

//     switch(err) {
//         case LIBSSH2_FX_OK:
//             return mrb_true_value();
//         case LIBSSH2_FX_NO_SUCH_FILE:
//         case LIBSSH2_FX_NO_SUCH_PATH:
//         case LIBSSH2_FX_INVALID_FILENAME:
//             return mrb_false_value();
//     }

//     return mrb_nil_value();
// }

// static mrb_value
// mrb_sftp_file_f_rpath (mrb_state *mrb, mrb_value self)
// {
//     LIBSSH2_SFTP *sftp;
//     const char *path;
//     char rpath[256];
//     int len;

//     sftp = mrb_sftp_file_sftp(mrb, self);
//     mrb_sftp_raise_unless_connected(mrb, sftp);
//     path = mrb_sftp_file_path(mrb, self, &len);

//     libssh2_sftp_symlink_ex(sftp, path, len, rpath, 256, LIBSSH2_SFTP_REALPATH);

//     return mrb_str_new_cstr(mrb, rpath);
// }

// static mrb_value
// mrb_sftp_file_f_stat (mrb_state *mrb, mrb_value self)
// {
//     LIBSSH2_SFTP_ATTRIBUTES attrs;
//     mrb_sftp_file_attrs(mrb, self, &attrs, LIBSSH2_SFTP_STAT, 1);

//     return mrb_sftp_file_fill_attrs(mrb, attrs);
// }

// static mrb_value
// mrb_sftp_file_f_lstat (mrb_state *mrb, mrb_value self)
// {
//     LIBSSH2_SFTP_ATTRIBUTES attrs;
//     mrb_sftp_file_attrs(mrb, self, &attrs, LIBSSH2_SFTP_LSTAT, 1);

//     return mrb_sftp_file_fill_attrs(mrb, attrs);
// }

void
mrb_mruby_sftp_file_init (mrb_state *mrb)
{
    // struct RClass *ftp, *cls;

    // ftp = mrb_module_get(mrb, "SFTP");
    // cls = mrb_define_class_under(mrb, ftp, "File", mrb->object_class);

    // mrb_define_method(mrb, cls, "initialize", mrb_sftp_file_f_init,  MRB_ARGS_REQ(2));
    // mrb_define_method(mrb, cls, "exist?",     mrb_sftp_file_f_exist, MRB_ARGS_NONE());
    // mrb_define_method(mrb, cls, "realpath",   mrb_sftp_file_f_rpath, MRB_ARGS_REQ(1));
    // mrb_define_method(mrb, cls, "stat",       mrb_sftp_file_f_stat,  MRB_ARGS_REQ(1));
    // mrb_define_method(mrb, cls, "lstat",      mrb_sftp_file_f_lstat, MRB_ARGS_REQ(1));
}
