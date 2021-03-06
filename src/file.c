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
#include "mruby/ext/sftp.h"

#include <stdio.h>
#include <sys/stat.h>
#include <libssh2_sftp.h>

#define SYM(name, len) mrb_intern_static(mrb, name, len)

static mrb_value
mrb_sftp_f_download (mrb_state *mrb, mrb_value self)
{
    size_t mem_size = 3200000;
    const char* path;
    mrb_int len;
    FILE *file;
    char *mem;
    int rc;

    mrb_value session           = mrb_attr_get(mrb, self, SYM("@session", 8));
    mrb_ssh_t *ssh              = mrb_sftp_ssh_session(session);
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);

    libssh2_sftp_rewind(handle);
    mrb_iv_remove(mrb, self, SYM("buf", 3));

    mrb_get_args(mrb, "s", &path, &len);

    if (!(file = fopen(path, "wb"))) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot open the path specified.");
    }

    mem = mrb_malloc(mrb, mem_size * sizeof(char));

  read:

    while ((rc = libssh2_sftp_read(handle, mem, mem_size)) == LIBSSH2SFTP_EAGAIN) {
        mrb_ssh_wait_sock(ssh);
    }

    if (rc < 0) goto done;

    fwrite(mem, sizeof(char), rc, file);

    if (rc > 0) goto read;

  done:

    mrb_free(mrb, mem);
    fclose(file);
    mrb_iv_set(mrb, self, SYM("eof", 3), mrb_true_value());

    return mrb_fixnum_value(libssh2_sftp_tell64(handle));
}

static mrb_value
mrb_sftp_f_upload (mrb_state *mrb, mrb_value self)
{
    struct stat st;
    size_t mem_size;
    const char* path;
    mrb_int len;
    FILE *file;
    char*mem;

    mrb_value session           = mrb_attr_get(mrb, self, SYM("@session", 8));
    mrb_ssh_t *ssh              = mrb_sftp_ssh_session(session);
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);

    libssh2_sftp_rewind(handle);
    mrb_iv_remove(mrb, self, SYM("buf", 3));

    mrb_get_args(mrb, "s", &path, &len);

    if (!(file = fopen(path, "rb"))) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot open the path specified.");
    }

    if (fstat(fileno(file), &st) != 0) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot determine file size.");
    }

    mem_size = st.st_size;

    if (mem_size <= 0) goto done;

    mem = (char*) mrb_malloc(mrb, mem_size * sizeof(char));

    fread(mem, sizeof(char), mem_size, file);
    fclose(file);

    while (libssh2_sftp_write(handle, mem, mem_size) == LIBSSH2SFTP_EAGAIN) {
        mrb_ssh_wait_sock(ssh);
    }

    mrb_free(mrb, mem);

  done:

    mrb_iv_set(mrb, self, SYM("eof", 3), mrb_true_value());

    return mrb_fixnum_value(mem_size);
}

static mrb_value
mrb_sftp_f_write (mrb_state *mrb, mrb_value self)
{
    const char* buf;
    mrb_int len;
    libssh2_uint64_t pos_before, pos_after;

    mrb_value session           = mrb_attr_get(mrb, self, SYM("@session", 8));
    mrb_ssh_t *ssh              = mrb_sftp_ssh_session(session);
    LIBSSH2_SFTP_HANDLE *handle = mrb_sftp_handle_bang(mrb, self);
    pos_before                  = libssh2_sftp_tell64(handle);

    mrb_get_args(mrb, "s", &buf, &len);

    while (libssh2_sftp_write(handle, buf, len) == LIBSSH2SFTP_EAGAIN) {
        mrb_ssh_wait_sock(ssh);
    }

    mrb_iv_remove(mrb, self, SYM("buf", 3));
    pos_after = libssh2_sftp_tell64(handle);

    return mrb_fixnum_value(pos_after - pos_before);
}

void
mrb_mruby_sftp_file_init (mrb_state *mrb)
{
    struct RClass *ftp, *cls, *sup;

    ftp = mrb_module_get(mrb, "SFTP");
    sup = mrb_class_get_under(mrb, ftp, "Handle");
    cls = mrb_define_class_under(mrb, ftp, "File", sup);

    mrb_define_method(mrb, cls, "download", mrb_sftp_f_download, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "upload",   mrb_sftp_f_upload, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, cls, "write",    mrb_sftp_f_write,  MRB_ARGS_REQ(1));
}
