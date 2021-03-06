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

#include "stat.h"
#include "handle.h"

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/version.h"
#include "mruby/variable.h"
#include "mruby/ext/sftp.h"

#include <string.h>
#include <libssh2_sftp.h>

#define SYM(name, len) mrb_intern_static(mrb, name, len)

#if MRUBY_RELEASE_NO < 10400
static inline mrb_int
mrb_str_index(mrb_state *mrb, mrb_value str, const char *lit, mrb_int len, mrb_int off)
{
    mrb_value pos = mrb_funcall(mrb, str, "index", 2, mrb_str_new_static(mrb, lit, len), mrb_fixnum_value(off));
    return mrb_nil_p(pos) ? -1 : mrb_fixnum(pos);
}
#endif

static void
mrb_sftp_handle_free (mrb_state *mrb, void *p)
{
    mrb_sftp_handle_t *data;

    if (!p) return;

    data = (mrb_sftp_handle_t *)p;

    if (data->handle && data->session->data && mrb_ssh_initialized()) {
        libssh2_sftp_close_handle(data->handle);
    }

    mrb_free(mrb, data);
}

static mrb_data_type const mrb_sftp_handle_type = { "SFTP::Handle", mrb_sftp_handle_free };

inline LIBSSH2_SFTP_HANDLE *
mrb_sftp_handle (mrb_state *mrb, mrb_value self)
{
    mrb_sftp_handle_t *data = DATA_PTR(self);
    return data ? data->handle : NULL;
}

static inline void
mrb_sftp_raise_unless_opened (mrb_state *mrb, LIBSSH2_SFTP_HANDLE *handle)
{
    if (handle && mrb_ssh_initialized()) return;
    mrb_raise(mrb, E_SFTP_HANDLE_CLOSED_ERROR, "SFTP handle not opened.");
}

inline LIBSSH2_SFTP_HANDLE *
mrb_sftp_handle_bang (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    mrb_sftp_raise_unless_opened(mrb, handle);
    return handle;
}

static inline int
mrb_sftp_type (mrb_state *mrb, mrb_value self)
{
    return mrb_fixnum(mrb_attr_get(mrb, self, SYM("type", 4)));
}

static inline const char*
mrb_sftp_path (mrb_state *mrb, mrb_value self, int *len)
{
    mrb_value path = mrb_iv_get(mrb, self, SYM("@path", 5));

    if (len) {
        *len = mrb_string_value_len(mrb, path);
    }

    return mrb_string_value_ptr(mrb, path);
}

static void
mrb_sftp_open (mrb_state *mrb, mrb_value self, long flags, long mode, int type)
{
    const char *path;
    int len, err;

    LIBSSH2_SFTP *sftp;
    mrb_ssh_t *ssh;
    LIBSSH2_SFTP_HANDLE *handle;
    mrb_sftp_handle_t *data;
    mrb_value session;

    if (DATA_PTR(self)) return;

    session = mrb_attr_get(mrb, self, SYM("@session", 8));
    ssh     = mrb_sftp_ssh_session(session);
    sftp    = mrb_sftp_session(session);
    path    = mrb_sftp_path(mrb, self, &len);

    if (!sftp) {
        mrb_raise(mrb, E_SFTP_NOT_CONNECTED_ERROR, "SFTP session not connected.");
    }

    do {
        handle = libssh2_sftp_open_ex(sftp, path, len, flags, mode, type);

        if (handle) break;

        err = libssh2_session_last_errno(ssh->session);

        if (err == LIBSSH2SFTP_EAGAIN)
        {
            mrb_ssh_wait_sock(ssh);
        }
        else if (err == LIBSSH2_ERROR_SFTP_PROTOCOL)
        {
            mrb_sftp_raise_last_error(mrb, sftp, "Unable to open the remote path.");
        }
        else if (err != LIBSSH2_ERROR_NONE)
        {
            mrb_ssh_raise_last_error(mrb, ssh);
        }
    } while (!handle);

    data          = mrb_malloc(mrb, sizeof(mrb_sftp_handle_t));
    data->session = mrb_ptr(session);
    data->handle  = handle;

    mrb_data_init(self, data, &mrb_sftp_handle_type);

    mrb_iv_set(mrb, self, SYM("type", 4), mrb_fixnum_value(type));
}

static mrb_value
mrb_sftp_f_gets_dir (mrb_state *mrb, mrb_value self)
{
    mrb_value session           = mrb_attr_get(mrb, self, SYM("@session", 8));
    mrb_ssh_t *ssh              = mrb_sftp_ssh_session(session);
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);
    struct RClass *cls          = mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "Entry");
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    char entry[256], longentry[512];
    mrb_value args[3];
    int rc;

    while ((rc = libssh2_sftp_readdir_ex(handle, entry, 256, longentry, 512, &attrs)) == LIBSSH2SFTP_EAGAIN) {
        mrb_ssh_wait_sock(ssh);
    }

    if (rc <= 0)
        return mrb_nil_value();

    args[0] = mrb_str_new(mrb, entry, rc);
    args[1] = mrb_str_new_cstr(mrb, longentry);
    args[2] = mrb_sftp_stat_obj(mrb, &attrs);

    return mrb_obj_new(mrb, cls, 3, args);
}

static mrb_value
mrb_sftp_f_gets_file (mrb_state *mrb, mrb_value self)
{
    mrb_value session           = mrb_attr_get(mrb, self, SYM("@session", 8));
    mrb_ssh_t *ssh              = mrb_sftp_ssh_session(session);
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);
    mrb_value buf               = mrb_attr_get(mrb, self, SYM("buf", 3));
    mrb_bool arg_given          = FALSE;
    mrb_bool opts_given         = FALSE;
    mrb_bool mem_size_given     = FALSE;
    const char *sep             = NULL;
    char *mem                   = NULL;
    unsigned int mem_size       = 256;
    unsigned int sep_len        = 0;
    int chomp                   = FALSE;
    mrb_value arg, opts, res;
    int rc, pos;

    mrb_get_args(mrb, "|o?H!?", &arg, &arg_given, &opts, &opts_given);

    if (opts_given && mrb_hash_p(opts)) {
        chomp = mrb_type(mrb_hash_get(mrb, opts, mrb_symbol_value(SYM("chomp", 5)))) == MRB_TT_TRUE;
    }

    if (arg_given && mrb_string_p(arg)) {
        sep     = RSTRING_PTR(arg);
        sep_len = RSTRING_LEN(arg);
    } else
    if (arg_given && mrb_hash_p(arg)) {
        sep     = "\n";
        sep_len = 1;
        chomp   = mrb_type(mrb_hash_get(mrb, arg, mrb_symbol_value(SYM("chomp", 5)))) == MRB_TT_TRUE;
    } else
    if (arg_given && mrb_fixnum_p(arg)) {
        mem_size       = mrb_fixnum(arg);
        mem_size_given = TRUE;
    } else
    if (arg_given && mrb_nil_p(arg)) {
        mem_size  = 3200000;
    } else
    if (!arg_given) {
        sep     = "\n";
        sep_len = 1;
    } else {
        mrb_raise(mrb, E_TYPE_ERROR, "String or Fixnum expected.");
    }

    if (sep && mrb_test(buf) && ((pos = mrb_str_index(mrb, buf, sep, sep_len, 0)) != -1))
        goto hit;

    if (mem_size_given && mrb_test(buf)) {
        if (RSTRING_LEN(buf) >= mem_size) {
            pos = mem_size;
            goto hit;
        } else {
            mem_size -= RSTRING_LEN(buf);
        }
    }

    mem = mrb_malloc(mrb, mem_size * sizeof(char));

  read:

    while ((rc = libssh2_sftp_read(handle, mem, mem_size)) == LIBSSH2SFTP_EAGAIN) {
        mrb_ssh_wait_sock(ssh);
    };

    if (rc <= 0) {
        mrb_iv_set(mrb, self, SYM("eof", 3), mrb_true_value());
        mrb_iv_remove(mrb, self, SYM("buf", 3));
        res = buf;
        goto chomp;
    }

    if (mrb_test(buf)) {
        buf = mrb_str_cat(mrb, buf, mem, rc);
    } else {
        buf = mrb_str_new(mrb, mem, rc);
    }

    if (!sep && rc > 0 && (!mem_size_given || RSTRING_LEN(buf) < mem_size))
        goto read;

    if (!sep) {
        mrb_iv_remove(mrb, self, SYM("buf", 3));
        res = buf;
        goto chomp;
    }

    if ((pos = mrb_str_index(mrb, buf, sep, sep_len, 0)) == -1)
        goto read;

  hit:

    pos += sep_len;
    res = mrb_str_new(mrb, RSTRING_PTR(buf), pos);
    buf = mrb_str_substr(mrb, buf, pos, RSTRING_LEN(buf) - pos);

    mrb_iv_set(mrb, self, SYM("buf", 3), buf);

  chomp:

    if (mem) {
        mrb_free(mrb, mem);
    }

    if (mrb_string_p(res) && RSTRING_LEN(res) == 0) {
        return mrb_nil_value();
    }

    if (chomp && mrb_string_p(res)) {
        mrb_funcall(mrb, res, "chomp!", 0);
    }

    return res;
}

static mrb_value
mrb_sftp_f_open_dir (mrb_state *mrb, mrb_value self) {
    mrb_sftp_open(mrb, self, 0, 0, LIBSSH2_SFTP_OPENDIR);
    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_open_file (mrb_state *mrb, mrb_value self) {
    mrb_int flag_len = 0, mode = 0;
    int flags        = LIBSSH2_FXF_READ;
    const char *flag;

    mrb_get_args(mrb, "|s!i", &flag, &flag_len, &mode);

    if (flag_len == 0) {
        flags = LIBSSH2_FXF_READ;
    } else
    if (strncmp(flag, "r", flag_len) == 0) {
        flags = LIBSSH2_FXF_READ;
    } else
    if (strncmp(flag, "r+", flag_len) == 0) {
        flags = LIBSSH2_FXF_READ | LIBSSH2_FXF_WRITE;
    } else
    if (strncmp(flag, "w", flag_len) == 0) {
        flags = LIBSSH2_FXF_WRITE | LIBSSH2_FXF_TRUNC | LIBSSH2_FXF_CREAT;
    } else
    if (strncmp(flag, "w+", flag_len) == 0) {
        flags = LIBSSH2_FXF_READ | LIBSSH2_FXF_WRITE | LIBSSH2_FXF_TRUNC | LIBSSH2_FXF_CREAT;
    } else
    if (strncmp(flag, "a", flag_len) == 0) {
        flags = LIBSSH2_FXF_APPEND | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_WRITE;
    } else
    if (strncmp(flag, "a+", flag_len) == 0) {
        flags = LIBSSH2_FXF_APPEND | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_READ | LIBSSH2_FXF_WRITE;
    } else {
        mrb_raise(mrb, E_SFTP_ERROR, "Unsupported flags.");
    }

    mrb_sftp_open(mrb, self, flags, mode, LIBSSH2_SFTP_OPENFILE);

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_pos (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);
    mrb_value buf               = mrb_attr_get(mrb, self, SYM("buf", 3));
    libssh2_uint64_t pos;

    pos = libssh2_sftp_tell64(handle);

    if (mrb_test(buf)) {
        pos -= RSTRING_LEN(buf);
    }

    return mrb_fixnum_value(pos);
}

static mrb_value
mrb_sftp_f_seek (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);
    mrb_int offset              = 0;
    mrb_sym whence              = SYM("SET", 3);
    LIBSSH2_SFTP_ATTRIBUTES attrs;

    mrb_get_args(mrb, "i|n", &offset, &whence);

    if (whence == SYM("CUR", 3)) {
        offset += libssh2_sftp_tell64(handle);
    } else
    if (whence == SYM("END", 3)) {
        while (libssh2_sftp_fstat(handle, &attrs) == LIBSSH2SFTP_EAGAIN);
        offset += attrs.filesize;
    } else
    if (whence != SYM("SET", 3)) {
        mrb_raise(mrb, E_SFTP_ERROR, "Unknown seek option for SFTP handle.");
    }

    if (offset < 0) {
        offset = 0;
    }

    libssh2_sftp_seek64(handle, offset);

    mrb_iv_remove(mrb, self, SYM("eof", 3));
    mrb_iv_remove(mrb, self, SYM("buf", 3));

    return mrb_fixnum_value(libssh2_sftp_tell64(handle));
}

static mrb_value
mrb_sftp_f_gets (mrb_state *mrb, mrb_value self)
{
    switch (mrb_sftp_type(mrb, self)) {
        case LIBSSH2_SFTP_OPENDIR:
            return mrb_sftp_f_gets_dir(mrb, self);
        default:
            return mrb_sftp_f_gets_file(mrb, self);
    }
}

static mrb_value
mrb_sftp_f_eof (mrb_state *mrb, mrb_value self)
{
    mrb_value eof = mrb_attr_get(mrb, self, SYM("eof", 3));

    mrb_sftp_handle_bang(mrb, self);

    return mrb_bool_value(mrb_test(eof) ? TRUE : FALSE);
}

static mrb_value
mrb_sftp_f_sync (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);
    LIBSSH2_SFTP *sftp;
    mrb_value session;
    int ret;

    while ((ret = libssh2_sftp_fsync(handle)) == LIBSSH2SFTP_EAGAIN);

    if (ret != 0) {
        session = mrb_attr_get(mrb, self, SYM("@session", 8));
        sftp    = mrb_sftp_session(session);

        mrb_sftp_raise_last_error(mrb, sftp, "Cannot sync the SFTP handle.");
    }

    mrb_iv_remove(mrb, self, SYM("eof", 3));
    mrb_iv_remove(mrb, self, SYM("buf", 3));

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_close (mrb_state *mrb, mrb_value self)
{
    mrb_sftp_handle_free(mrb, DATA_PTR(self));

    DATA_PTR(self)  = NULL;
    DATA_TYPE(self) = NULL;

    mrb_iv_remove(mrb, self, SYM("eof", 3));
    mrb_iv_remove(mrb, self, SYM("buf", 3));

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_closed (mrb_state *mrb, mrb_value self)
{
    mrb_sftp_handle_t *data = DATA_PTR(self);

    if (!(data && mrb_ssh_initialized()))
        return mrb_true_value();

    return mrb_bool_value(data->session->data ? FALSE : TRUE);
}

void
mrb_mruby_sftp_handle_init (mrb_state *mrb)
{
    struct RClass *ftp, *cls;

    ftp = mrb_module_get(mrb, "SFTP");
    cls = mrb_define_class_under(mrb, ftp, "Handle", mrb->object_class);

    MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);

    mrb_define_method(mrb, cls, "open_dir", mrb_sftp_f_open_dir, MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "open_file",mrb_sftp_f_open_file, MRB_ARGS_OPT(2));
    mrb_define_method(mrb, cls, "pos",      mrb_sftp_f_pos,    MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "seek",     mrb_sftp_f_seek,   MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, cls, "gets",     mrb_sftp_f_gets,   MRB_ARGS_OPT(1));
    mrb_define_method(mrb, cls, "eof?",     mrb_sftp_f_eof,    MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "sync",     mrb_sftp_f_sync,   MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "close",    mrb_sftp_f_close,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "closed?",  mrb_sftp_f_closed, MRB_ARGS_NONE());
}
