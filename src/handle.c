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

#include "handle.h"

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/version.h"
#include "mruby/variable.h"
#include "mruby/ext/sftp.h"

#include <stdlib.h>
#include <string.h>
#include <libssh2_sftp.h>

#if MRUBY_RELEASE_NO < 10400
static mrb_int
mrb_str_index(mrb_state *mrb, mrb_value str, const char *lit, mrb_int len, mrb_int off)
{
    mrb_value pos = mrb_funcall(mrb, str, "index", 2, mrb_str_new_static(mrb, lit, len), mrb_fixnum_value(off));
    return mrb_nil_p(pos) ? -1 : mrb_fixnum(pos);
}
#endif

static mrb_sym SYM_EOF;
static mrb_sym SYM_BUF;
static mrb_sym SYM_CUR;
static mrb_sym SYM_SET;
static mrb_sym SYM_TYPE;
static mrb_sym SYM_PATH;
static mrb_value KEY_CHOMP;
static mrb_sym SYM_SESSION;

static void
mrb_sftp_handle_free (mrb_state *mrb, void *p)
{
    mrb_sftp_handle_t *data;

    if (!p) return;

    data = (mrb_sftp_handle_t *)p;

    if (data->handle && data->session->data && mrb_ssh_initialized()) {
        libssh2_sftp_close_handle(data->handle);
    }

    free(data);
}

static mrb_data_type const mrb_sftp_handle_type = { "SFTP::Handle", mrb_sftp_handle_free };

static LIBSSH2_SFTP_HANDLE *
mrb_sftp_handle (mrb_state *mrb, mrb_value self)
{
    mrb_sftp_handle_t *data = DATA_PTR(self);
    return data ? data->handle : NULL;
}

static void
mrb_sftp_raise_unless_opened (mrb_state *mrb, LIBSSH2_SFTP_HANDLE *handle)
{
    if (handle && mrb_ssh_initialized()) return;
    mrb_raise(mrb, E_RUNTIME_ERROR, "SFTP handle not opened.");
}

static int
mrb_sftp_type (mrb_state *mrb, mrb_value self)
{
    return mrb_fixnum(mrb_attr_get(mrb, self, SYM_TYPE));
}

static const char*
mrb_sftp_path (mrb_state *mrb, mrb_value self, int *len)
{
    mrb_value path = mrb_iv_get(mrb, self, SYM_PATH);

    if (len) {
        *len = mrb_string_value_len(mrb, path);
    }

    return mrb_string_value_ptr(mrb, path);
}

static void
mrb_sftp_open (mrb_state *mrb, mrb_value self, long flags, long mode, int type)
{
    const char *path;
    int len;

    LIBSSH2_SFTP *sftp;
    LIBSSH2_SFTP_HANDLE *handle;
    mrb_sftp_handle_t *data;
    mrb_value session;

    if (DATA_PTR(self)) return;

    session = mrb_attr_get(mrb, self, SYM_SESSION);
    sftp    = mrb_sftp_session(session);
    path    = mrb_sftp_path(mrb, self, &len);

    if (!sftp) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "SFTP session not connected.");
    }

    handle = libssh2_sftp_open_ex(sftp, path, len, flags, mode, type);

    if (!handle) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "The system cannot find the dir specified.");
    }

    data          = malloc(sizeof(mrb_sftp_handle_t));
    data->session = mrb_ptr(session);
    data->handle  = handle;

    mrb_data_init(self, data, &mrb_sftp_handle_type);

    mrb_iv_set(mrb, self, SYM_TYPE, mrb_fixnum_value(type));
}

static mrb_value
mrb_sftp_readdir (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    int mem_size = 256;
    char mem[mem_size];
    int rc;

    rc = libssh2_sftp_readdir(handle, mem, mem_size, NULL);

    if (rc <= 0) {
        return mrb_nil_value();
    }

    return mrb_str_new(mrb, mem, rc);
}

static mrb_value
mrb_sftp_readfile (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    mrb_value arg, opts, res, buf = mrb_attr_get(mrb, self, SYM_BUF);
    mrb_bool arg_given = FALSE, mem_size_given = FALSE;
    int rc, pos, max_mem_size = 30000, chomp = FALSE;
    int mem_size = 256, sep_len = 0;
    const char *sep = NULL;
    char *mem;

    mrb_get_args(mrb, "|o?H!", &arg, &arg_given, &opts);

    if (mrb_hash_p(opts)) {
        chomp = mrb_type(mrb_hash_get(mrb, opts, KEY_CHOMP)) == MRB_TT_TRUE;
    }

    if (mrb_string_p(arg)) {
        sep       = RSTRING_PTR(arg);
        sep_len   = RSTRING_LEN(arg);
    } else
    if (mrb_fixnum_p(arg)) {
        mem_size       = mrb_fixnum(arg);
        mem_size_given = TRUE;
    } else
    if (mrb_hash_p(arg)) {
        sep     = "\n";
        sep_len = 1;
        chomp   = mrb_type(mrb_hash_get(mrb, arg, KEY_CHOMP)) == MRB_TT_TRUE;
    } else
    if (arg_given && mrb_nil_p(arg)) {
        mem_size  = max_mem_size;
    } else
    if (!arg_given) {
        sep     = "\n";
        sep_len = 1;
    } else {
        mrb_raise(mrb, E_TYPE_ERROR, "String or Fixnum expected.");
    }

    if (sep && mrb_test(buf) && ((pos = mrb_str_index(mrb, buf, sep, sep_len, 0)) != -1))
        goto hit;

    if (mem_size > max_mem_size) {
        mem_size = max_mem_size;
    }

    if (mem_size_given && mrb_test(buf)) {
        if (RSTRING_LEN(buf) >= mem_size) {
            pos = mem_size;
            goto hit;
        } else {
            mem_size -= RSTRING_LEN(buf);
        }
    }

  read:

    mem = malloc(mem_size * sizeof(char));
    rc  = libssh2_sftp_read(handle, mem, mem_size);

    if (rc <= 0) {
        free(mem);
        mrb_iv_set(mrb, self, SYM_EOF, mrb_true_value());
        mrb_iv_remove(mrb, self, SYM_BUF);
        res = buf;
        goto chomp;
    }

    if (mrb_test(buf)) {
        buf = mrb_str_cat(mrb, buf, mem, rc);
    } else {
        buf = mrb_str_new(mrb, mem, rc);
    }

    free(mem);

    if (!sep && !mem_size_given && rc > 0)
        goto read;

    if (!sep) {
        mrb_iv_remove(mrb, self, SYM_BUF);
        res = buf;
        goto chomp;
    }

    if ((pos = mrb_str_index(mrb, buf, sep, sep_len, 0)) == -1)
        goto read;

  hit:

    pos += sep_len;
    res = mrb_str_new(mrb, RSTRING_PTR(buf), pos);
    buf = mrb_str_substr(mrb, buf, pos, RSTRING_LEN(buf) - pos);

    mrb_iv_set(mrb, self, SYM_BUF, buf);

  chomp:

    if (mrb_string_p(res) && RSTRING_LEN(res) == 0) {
        return mrb_nil_value();
    }

    if (chomp && mrb_string_p(res)) {
        res = mrb_funcall(mrb, res, "chomp", 0);
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
        mrb_raise(mrb, E_RUNTIME_ERROR, "Unsupported flags.");
    }

    mrb_sftp_open(mrb, self, flags, mode, LIBSSH2_SFTP_OPENFILE);

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_pos (mrb_state *mrb, mrb_value self)
{
    mrb_value buf               = mrb_attr_get(mrb, self, SYM_BUF);
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    int pos;

    mrb_sftp_raise_unless_opened(mrb, handle);

    pos = libssh2_sftp_tell64(handle);

    if (mrb_test(buf)) {
        pos -= RSTRING_LEN(buf);
    }

    return mrb_fixnum_value(pos);
}

static mrb_value
mrb_sftp_f_seek (mrb_state *mrb, mrb_value self)
{
    mrb_int offset              = 0;
    mrb_sym whence              = SYM_SET;
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    mrb_sftp_raise_unless_opened(mrb, handle);

    mrb_get_args(mrb, "i|n", &offset, &whence);

    if (SYM_CUR == whence) {
        offset += libssh2_sftp_tell64(handle);
    } else
    if (SYM_SET != whence) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "Unknown seek option for SFTP handle.");
    }

    libssh2_sftp_seek64(handle, offset);

    mrb_iv_remove(mrb, self, SYM_EOF);
    mrb_iv_remove(mrb, self, SYM_BUF);

    return mrb_fixnum_value(libssh2_sftp_tell64(handle));
}

static mrb_value
mrb_sftp_f_gets (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    mrb_sftp_raise_unless_opened(mrb, handle);

    if (mrb_sftp_type(mrb, self) == LIBSSH2_SFTP_OPENDIR) {
        return mrb_sftp_readdir(mrb, self);
    }

    return mrb_sftp_readfile(mrb, self);
}

static mrb_value
mrb_sftp_f_eof (mrb_state *mrb, mrb_value self)
{
    mrb_value eof               = mrb_attr_get(mrb, self, SYM_EOF);
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    mrb_sftp_raise_unless_opened(mrb, handle);

    return mrb_bool_value(mrb_test(eof) ? TRUE : FALSE);
}

static mrb_value
mrb_sftp_f_close (mrb_state *mrb, mrb_value self)
{
    mrb_sftp_handle_free(mrb, DATA_PTR(self));

    DATA_PTR(self)  = NULL;
    DATA_TYPE(self) = NULL;

    mrb_iv_remove(mrb, self, SYM_EOF);
    mrb_iv_remove(mrb, self, SYM_BUF);

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

    SYM_CUR     = mrb_intern_static(mrb, "CUR", 3);
    SYM_SET     = mrb_intern_static(mrb, "SET", 3);
    SYM_EOF     = mrb_intern_static(mrb, "eof", 3);
    SYM_BUF     = mrb_intern_static(mrb, "buf", 3);
    SYM_TYPE    = mrb_intern_static(mrb, "type", 4);
    KEY_CHOMP   = mrb_symbol_value(mrb_intern_static(mrb, "chomp", 5));
    SYM_PATH    = mrb_intern_static(mrb, "@path", 5);
    SYM_SESSION = mrb_intern_static(mrb, "@session", 8);

    mrb_define_method(mrb, cls, "open_dir", mrb_sftp_f_open_dir,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "open_file",mrb_sftp_f_open_file, MRB_ARGS_OPT(2));
    mrb_define_method(mrb, cls, "pos",      mrb_sftp_f_pos,    MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "seek",     mrb_sftp_f_seek,   MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, cls, "gets",     mrb_sftp_f_gets,   MRB_ARGS_OPT(1));
    mrb_define_method(mrb, cls, "eof?",     mrb_sftp_f_eof,    MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "close",    mrb_sftp_f_close,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "closed?",  mrb_sftp_f_closed, MRB_ARGS_NONE());
}
