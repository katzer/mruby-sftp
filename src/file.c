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
#include "mruby/variable.h"

#include <stdlib.h>
#include <stdio.h>
#include <libssh2_sftp.h>

static mrb_sym SYM_EOF;
static mrb_sym SYM_BUF;

static mrb_value
mrb_sftp_f_download (mrb_state *mrb, mrb_value self)
{
    size_t mem_size = 3200000;
    const char* path;
    long size = 0;
    mrb_int len;
    FILE *file;
    char *mem;
    int rc;

    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);

    libssh2_sftp_rewind(handle);
    mrb_iv_remove(mrb, self, SYM_BUF);

    mrb_get_args(mrb, "s", &path, &len);

    if (!(file = fopen(path, "wb"))) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot open the path specified.");
    }

    mem = malloc(mem_size * sizeof(char));

  read:

    while ((rc = libssh2_sftp_read(handle, mem, mem_size)) == LIBSSH2_ERROR_EAGAIN);

    if (rc < 0) goto done;

    fwrite(mem, sizeof(char), rc, file);
    size += rc;

    if (rc > 0) goto read;

  done:

    free(mem);
    fclose(file);
    mrb_iv_set(mrb, self, SYM_EOF, mrb_true_value());

    return mrb_fixnum_value(size);
}

static mrb_value
mrb_sftp_f_upload (mrb_state *mrb, mrb_value self)
{
    size_t mem_size = 3200000;
    const char* path;
    long size = 0;
    mrb_int len;
    FILE *file;
    char *mem;
    size_t rc;

    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);

    libssh2_sftp_rewind(handle);
    mrb_iv_remove(mrb, self, SYM_BUF);

    mrb_get_args(mrb, "s", &path, &len);

    if (!(file = fopen(path, "rb"))) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot open the path specified.");
    }

    mem = malloc(mem_size * sizeof(char));

  read:

    rc = fread(mem, sizeof(char), mem_size, file);

    if (rc <= 0) goto done;

    while ((rc = libssh2_sftp_write(handle, mem, rc)) == LIBSSH2_ERROR_EAGAIN);

    size += rc;

    goto read;

  done:

    free(mem);
    fclose(file);
    mrb_iv_set(mrb, self, SYM_EOF, mrb_true_value());

    return mrb_fixnum_value(size);
}

static mrb_value
mrb_sftp_f_write (mrb_state *mrb, mrb_value self)
{
    const char* buf;
    mrb_int len;
    ssize_t ret;

    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);

    mrb_get_args(mrb, "s", &buf, &len);

    while ((ret = libssh2_sftp_write(handle, buf, len) == LIBSSH2_ERROR_EAGAIN));

    if (ret > 0) {
        mrb_iv_remove(mrb, self, SYM_BUF);
    }

    return mrb_fixnum_value(ret >= 0 ? ret : 0);
}

void
mrb_mruby_sftp_file_init (mrb_state *mrb)
{
    struct RClass *ftp, *cls, *sup;

    ftp = mrb_module_get(mrb, "SFTP");
    sup = mrb_class_get_under(mrb, ftp, "Handle");
    cls = mrb_define_class_under(mrb, ftp, "File", sup);

    // MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);

    SYM_EOF     = mrb_intern_static(mrb, "eof", 3);
    SYM_BUF     = mrb_intern_static(mrb, "buf", 3);

    mrb_define_method(mrb, cls, "download", mrb_sftp_f_download, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "upload",   mrb_sftp_f_upload, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "write",    mrb_sftp_f_write,  MRB_ARGS_REQ(1));
}