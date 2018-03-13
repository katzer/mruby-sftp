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
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/ext/sftp.h"

#include <stdlib.h>
#include <string.h>
#include <libssh2_sftp.h>

static mrb_sym CUR;
static mrb_sym SET;
static mrb_sym TYPE;
static mrb_sym PATH;
static mrb_sym SESSION;

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
    return mrb_fixnum(mrb_attr_get(mrb, self, TYPE));
}

static const char*
mrb_sftp_path (mrb_state *mrb, mrb_value self, int *len)
{
    mrb_value path = mrb_iv_get(mrb, self, PATH);

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

    session = mrb_iv_get(mrb, self, SESSION);
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

    mrb_iv_set(mrb, self, TYPE, mrb_fixnum_value(type));
}

static mrb_value
mrb_sftp_readdir (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    char mem[512];
    int mem_len;

    mem_len = libssh2_sftp_readdir(handle, mem, sizeof(mem), NULL);

    if (mem_len <= 0) {
        return mrb_nil_value();
    }

    return mrb_str_new(mrb, mem, mem_len);
}

static mrb_value
mrb_sftp_readfile (mrb_state *mrb, mrb_value self)
{
    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_open_dir (mrb_state *mrb, mrb_value self) {
    mrb_sftp_open(mrb, self, 0, 0, LIBSSH2_SFTP_OPENDIR);

    return mrb_nil_value();
}

static mrb_value
mrb_sftp_f_open_file (mrb_state *mrb, mrb_value self) {
    mrb_int flag_len = 0, mode = 0;
    int flags = LIBSSH2_FXF_READ;
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
mrb_sftp_f_tell (mrb_state *mrb, mrb_value self)
{
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    mrb_sftp_raise_unless_opened(mrb, handle);

    return mrb_fixnum_value(libssh2_sftp_tell64(handle));
}

static mrb_value
mrb_sftp_f_seek (mrb_state *mrb, mrb_value self)
{
    mrb_int offset = 0;
    mrb_sym whence = SET;
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle(mrb, self);
    mrb_sftp_raise_unless_opened(mrb, handle);

    mrb_get_args(mrb, "i|n", &offset, &whence);

    if (CUR == whence) {
        offset += libssh2_sftp_tell64(handle);
    } else
    if (SET != whence) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "Unknown seek option for SFTP handle.");
    }

    libssh2_sftp_seek64(handle, offset);

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
mrb_sftp_f_close (mrb_state *mrb, mrb_value self)
{
    mrb_sftp_handle_free(mrb, DATA_PTR(self));

    DATA_PTR(self)  = NULL;
    DATA_TYPE(self) = NULL;

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

    CUR     = mrb_intern_static(mrb, "CUR", 3);
    SET     = mrb_intern_static(mrb, "SET", 3);
    TYPE    = mrb_intern_static(mrb, "type", 4);
    PATH    = mrb_intern_static(mrb, "@path", 5);
    SESSION = mrb_intern_static(mrb, "@session", 8);

    mrb_define_method(mrb, cls, "open_dir", mrb_sftp_f_open_dir,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "open_file",mrb_sftp_f_open_file, MRB_ARGS_OPT(2));
    mrb_define_method(mrb, cls, "tell",     mrb_sftp_f_tell,   MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "seek",     mrb_sftp_f_seek,   MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, cls, "gets",     mrb_sftp_f_gets,   MRB_ARGS_OPT(1));
    mrb_define_method(mrb, cls, "close",    mrb_sftp_f_close,  MRB_ARGS_NONE());
    mrb_define_method(mrb, cls, "closed?",  mrb_sftp_f_closed, MRB_ARGS_NONE());
}
