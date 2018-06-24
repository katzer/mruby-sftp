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

#ifndef MRUBY_SFTP_H
#define MRUBY_SFTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mruby.h"
#include "mruby/ext/ssh.h"
#include <libssh2_sftp.h>

struct mrb_sftp
{
    struct RData *session;
    LIBSSH2_SFTP *sftp;
};

typedef struct mrb_sftp mrb_sftp_t;

#define E_SFTP_ERROR                  (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "Exception"))
#define E_SFTP_PERM_ERROR             (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "PermissionError"))
#define E_SFTP_HANDLE_CLOSED_ERROR    (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "HandleNotOpened"))
#define E_SFTP_UNSUPPORTED_ERROR      (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "Unsupported"))
#define E_SFTP_NOT_CONNECTED_ERROR    (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "NotConnected"))
#define E_SFTP_CONNECTION_LOST_ERROR  (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "ConnectionLost"))
#define E_SFTP_FILE_ERROR             (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "FileError"))
#define E_SFTP_DIR_ERROR              (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "DirError"))
#define E_SFTP_PATH_ERROR             (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "PathError"))
#define E_SFTP_NAME_ERROR             (mrb_class_get_under(mrb, mrb_module_get(mrb, "SFTP"), "NameError"))

LIBSSH2_SFTP* mrb_sftp_session (mrb_value self);
mrb_ssh_t* mrb_sftp_ssh_session (mrb_value self);
void mrb_sftp_raise_last_error (mrb_state *mrb, LIBSSH2_SFTP *sftp, const char* msg);
void mrb_sftp_raise (mrb_state *mrb, int err, const char* msg);

#ifdef __cplusplus
}
#endif
#endif
