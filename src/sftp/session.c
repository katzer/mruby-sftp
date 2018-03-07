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

#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <libssh2_sftp.h>

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/ext/ssh.h"
#include "mruby/variable.h"

static mrb_ssh_t *
mrb_sftp_ssh_session (mrb_state *mrb, mrb_value self)
{
    return DATA_PTR(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@session")));
}

static void
mrb_sftp_session_free(mrb_state *mrb, void *p)
{
    if (!p) return;
    libssh2_sftp_shutdown((LIBSSH2_SFTP *)p);
}

static mrb_data_type const mrb_sftp_session_type = { "SFTP::Session", mrb_sftp_session_free };

// static char*
// get_path_ex(mrb_state *mrb, mrb_value self, int *path_len, int opt_path)
// {
//     mrb_int dir_len;
//     mrb_bool dir_given;
//     char *dir;

//     if (opt_path == 1) {
//         mrb_get_args(mrb, "|s?", &dir, &dir_len, &dir_given);
//     } else {
//         mrb_get_args(mrb, "s?", &dir, &dir_len, &dir_given);
//     }

//     if (!dir_given) {
//         dir     = (char *)"";
//         dir_len = 0;
//     }

//     if (dir_len > 1 && dir[dir_len - 1] == '/') {
//         dir[--dir_len] = 0;
//     }

//     if (path_len) {
//         *path_len = dir_len;
//     }

//     return dir;
// }

// static char*
// get_path(mrb_state *mrb, mrb_value self, int *path_len)
// {
//     return get_path_ex(mrb, self, path_len, 1);
// }

// static LIBSSH2_SFTP_HANDLE*
// get_handle (mrb_state *mrb, mrb_value self, char **path, int *path_len)
// {
//     char *dir;
//     SFTP_SESSION *session = DATA_PTR(self);
//     LIBSSH2_SFTP *sftp_session;
//     LIBSSH2_SFTP_HANDLE *sftp_handle;

//     raise_if_not_authenticated(mrb, self);

//     dir          = get_path(mrb, self, path_len);
//     sftp_session = session->sftp_session;
//     sftp_handle  = libssh2_sftp_opendir(sftp_session, dir);

//     if (!sftp_handle) {
//         mrb_raise(mrb, E_RUNTIME_ERROR, "The system cannot find the dir specified.");
//     }

//     if (path) {
//         *path = dir;
//     }

//     return sftp_handle;
// }

// static LIBSSH2_SFTP_ATTRIBUTES
// get_attrs (mrb_state *mrb, mrb_value self, int type)
// {
//     int path_len;
//     SFTP_SESSION *session = DATA_PTR(self);
//     LIBSSH2_SFTP_ATTRIBUTES attrs;
//     char *path;

//     raise_if_not_authenticated(mrb, self);
//     path = get_path_ex(mrb, self, &path_len, 0);

//     if (libssh2_sftp_stat_ex(session->sftp_session, path, path_len, type, &attrs) < 0) {
//         mrb_raise(mrb, E_RUNTIME_ERROR, "Unable to get status about the file.");
//     }

//     return attrs;
// }

// static int
// join_path (char *buf, char *pref, int pref_len, char *path)
// {
//     if (pref_len == 0) {
//         return sprintf(buf, "%s", path);
//     } else
//     if (pref_len == 1 && pref[0] == '/') {
//         return sprintf(buf, "/%s", path);
//     } else {
//         return sprintf(buf, "%s/%s", pref, path);
//     }
// }

// static void
// raise_unless_connected (mrb_state *mrb, mrb_sftp_t *sftp)
// {
//     if (sftp && sftp->session) return;
//     mrb_raise(mrb, E_RUNTIME_ERROR, "SFTP session not connected.");
// }

static mrb_value
mrb_sftp_f_connect (mrb_state *mrb, mrb_value self)
{
    mrb_ssh_t *ssh = mrb_sftp_ssh_session(mrb, self);
    LIBSSH2_SFTP *sftp;
    char *err;

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
    return DATA_PTR(self) ? mrb_false_value() : mrb_true_value();
}

// static mrb_value
// mrb_sftp_f_list (mrb_state *mrb, mrb_value self)
// {
//     LIBSSH2_SFTP_HANDLE *handle;
//     mrb_value entries;

//     handle  = get_handle(mrb, self, NULL, NULL);
//     entries = mrb_ary_new(mrb);

//     do {
//         char mem[512];
//         char longentry[512];
//         int mem_len;

//         mem_len = libssh2_sftp_readdir_ex(handle, mem, sizeof(mem), longentry, sizeof(longentry), NULL);

//         if (mem_len <= 0)
//             break;

//         if (mem_len == 1 && mem[0] == '.')
//             continue;

//         if (mem_len == 2 && mem[0] == '.' && mem[0] == mem[1])
//             continue;

//         mrb_ary_push(mrb, entries, mrb_str_new_cstr(mrb, longentry));
//     } while (1);

//     libssh2_sftp_closedir(handle);

//     return entries;
// }

// static mrb_value
// mrb_sftp_f_entries (mrb_state *mrb, mrb_value self)
// {
//     char *dir;
//     int dir_len;
//     mrb_value entries;
//     LIBSSH2_SFTP_HANDLE *handle;

//     handle  = get_handle(mrb, self, &dir, &dir_len);
//     entries = mrb_ary_new(mrb);

//     do {
//         char mem[512];
//         int  mem_len, path_len;

//         mem_len = libssh2_sftp_readdir(handle, mem, sizeof(mem), NULL);

//         if (mem_len <= 0)
//             break;

//         if (mem_len == 1 && mem[0] == '.')
//             continue;

//         if (mem_len == 2 && mem[0] == '.' && mem[0] == mem[1])
//             continue;

//         char path[mem_len + 1 + dir_len];
//         path_len = join_path(path, dir, dir_len, mem);

//         mrb_ary_push(mrb, entries, mrb_str_new(mrb, path, path_len));
//     } while (1);

//     libssh2_sftp_closedir(handle);

//     return entries;
// }

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
//     mrb_define_method(mrb, ftp, "list",    mrb_sftp_f_list,    MRB_ARGS_OPT(1));
//     mrb_define_method(mrb, ftp, "entries", mrb_sftp_f_entries, MRB_ARGS_OPT(1));
    mrb_define_method(mrb, cls, "close",   mrb_sftp_f_close,   MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "closed?", mrb_sftp_f_closed,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "last_errno", mrb_sftp_f_last_errno, MRB_ARGS_NONE());
}
