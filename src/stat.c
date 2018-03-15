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

#include "mruby.h"
#include "mruby/hash.h"
#include "mruby/variable.h"

#include <libssh2_sftp.h>

static mrb_sym SYM_ATIME;
static mrb_sym SYM_MTIME;
static mrb_sym SYM_SIZE;
static mrb_sym SYM_MODE;
static mrb_sym SYM_UID;
static mrb_sym SYM_GID;

mrb_value
mrb_sftp_stat_obj (mrb_state *mrb, LIBSSH2_SFTP_ATTRIBUTES attrs)
{
    struct RClass *ftp, *cls;
    mrb_value obj;

    ftp = mrb_module_get(mrb, "SFTP");
    cls = mrb_class_get_under(mrb, ftp, "Stat");
    obj = mrb_obj_new(mrb, cls, 0, NULL);

    if (attrs.flags & LIBSSH2_SFTP_ATTR_ACMODTIME) {
        mrb_iv_set(mrb, obj, SYM_ATIME, mrb_fixnum_value(attrs.atime));
        mrb_iv_set(mrb, obj, SYM_MTIME, mrb_fixnum_value(attrs.mtime));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE) {
        mrb_iv_set(mrb, obj, SYM_SIZE, mrb_fixnum_value(attrs.filesize));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
        mrb_iv_set(mrb, obj, SYM_MODE, mrb_fixnum_value(attrs.permissions));
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_UIDGID) {
        mrb_iv_set(mrb, obj, SYM_UID, mrb_fixnum_value(attrs.uid));
        mrb_iv_set(mrb, obj, SYM_GID, mrb_fixnum_value(attrs.gid));
    }

    return obj;
}

void
mrb_sftp_hash_to_stat (mrb_state *mrb, mrb_value hsh, LIBSSH2_SFTP_ATTRIBUTES *attrs)
{
    mrb_value atime = mrb_hash_get(mrb, hsh, mrb_symbol_value(SYM_ATIME));
    mrb_value mtime = mrb_hash_get(mrb, hsh, mrb_symbol_value(SYM_MTIME));
    mrb_value gid   = mrb_hash_get(mrb, hsh, mrb_symbol_value(SYM_GID));
    mrb_value uid   = mrb_hash_get(mrb, hsh, mrb_symbol_value(SYM_UID));
    mrb_value size  = mrb_hash_get(mrb, hsh, mrb_symbol_value(SYM_SIZE));
    mrb_value mode  = mrb_hash_get(mrb, hsh, mrb_symbol_value(SYM_MODE));

    if (mrb_test(atime) || mrb_test(mtime)) {
        attrs->flags |= LIBSSH2_SFTP_ATTR_ACMODTIME;
        attrs->atime = mrb_fixnum(atime);
        attrs->mtime = mrb_fixnum(mtime);
    }

    if (mrb_test(uid) || mrb_test(gid)) {
        attrs->flags |= LIBSSH2_SFTP_ATTR_UIDGID;
        attrs->uid = mrb_fixnum(uid);
        attrs->gid = mrb_fixnum(gid);
    }

    if (mrb_test(size)) {
        attrs->flags |= LIBSSH2_SFTP_ATTR_SIZE;
        attrs->filesize = mrb_fixnum(size);
    }

    if (mrb_test(mode)) {
        attrs->flags |= LIBSSH2_SFTP_ATTR_PERMISSIONS;
        attrs->permissions = mrb_fixnum(mode);
    }
}

static mrb_value
mrb_sftp_f_type (mrb_state *mrb, mrb_value self)
{
    mrb_value mode = mrb_attr_get(mrb, self, SYM_MODE);
    unsigned int m = mrb_fixnum(mode);

    if (mrb_nil_p(mode) || m == 0)
        mrb_fixnum_value(LIBSSH2_SFTP_TYPE_UNKNOWN);

    if (LIBSSH2_SFTP_S_ISLNK(m))
        return mrb_fixnum_value(LIBSSH2_SFTP_TYPE_SYMLINK);

    if (LIBSSH2_SFTP_S_ISREG(m))
        return mrb_fixnum_value(LIBSSH2_SFTP_TYPE_REGULAR);

    if (LIBSSH2_SFTP_S_ISDIR(m))
        return mrb_fixnum_value(LIBSSH2_SFTP_TYPE_DIRECTORY);

    if (LIBSSH2_SFTP_S_ISCHR(m))
        return mrb_fixnum_value(LIBSSH2_SFTP_TYPE_CHAR_DEVICE);

    if (LIBSSH2_SFTP_S_ISBLK(m))
        return mrb_fixnum_value(LIBSSH2_SFTP_TYPE_BLOCK_DEVICE);

    if (LIBSSH2_SFTP_S_ISFIFO(m))
        return mrb_fixnum_value(LIBSSH2_SFTP_TYPE_FIFO);

    if (LIBSSH2_SFTP_S_ISSOCK(m))
        return mrb_fixnum_value(LIBSSH2_SFTP_TYPE_SOCKET);

    return mrb_fixnum_value(LIBSSH2_SFTP_TYPE_UNKNOWN);
}

void
mrb_mruby_sftp_stat_init (mrb_state *mrb)
{
    struct RClass *ftp, *cls;

    ftp = mrb_module_get(mrb, "SFTP");
    cls = mrb_define_class_under(mrb, ftp, "Stat", mrb->object_class);

    mrb_define_method(mrb, cls, "type", mrb_sftp_f_type, MRB_ARGS_NONE());

    SYM_ATIME = mrb_intern_static(mrb, "@atime", 6);
    SYM_MTIME = mrb_intern_static(mrb, "@mtime", 6);
    SYM_SIZE  = mrb_intern_static(mrb, "@size", 5);
    SYM_MODE  = mrb_intern_static(mrb, "@mode", 5);
    SYM_UID   = mrb_intern_static(mrb, "@uid", 4);
    SYM_GID   = mrb_intern_static(mrb, "@gid", 4);

    mrb_define_const(mrb, cls, "T_REGULAR", mrb_fixnum_value(LIBSSH2_SFTP_TYPE_REGULAR));
    mrb_define_const(mrb, cls, "T_DIRECTORY", mrb_fixnum_value(LIBSSH2_SFTP_TYPE_DIRECTORY));
    mrb_define_const(mrb, cls, "T_SYMLINK", mrb_fixnum_value(LIBSSH2_SFTP_TYPE_SYMLINK));
    mrb_define_const(mrb, cls, "T_UNKNOWN", mrb_fixnum_value(LIBSSH2_SFTP_TYPE_UNKNOWN));
    mrb_define_const(mrb, cls, "T_SOCKET",  mrb_fixnum_value(LIBSSH2_SFTP_TYPE_SOCKET));
    mrb_define_const(mrb, cls, "T_CHAR_DEVICE", mrb_fixnum_value(LIBSSH2_SFTP_TYPE_CHAR_DEVICE));
    mrb_define_const(mrb, cls, "T_BLOCK_DEVICE", mrb_fixnum_value(LIBSSH2_SFTP_TYPE_BLOCK_DEVICE));
    mrb_define_const(mrb, cls, "T_FIFO",    mrb_fixnum_value(LIBSSH2_SFTP_TYPE_FIFO));

    mrb_define_const(mrb, cls, "P_USR_RWX", mrb_fixnum_value(LIBSSH2_SFTP_S_IRWXU));
    mrb_define_const(mrb, cls, "P_USR_R",   mrb_fixnum_value(LIBSSH2_SFTP_S_IRUSR));
    mrb_define_const(mrb, cls, "P_USR_W",   mrb_fixnum_value(LIBSSH2_SFTP_S_IWUSR));
    mrb_define_const(mrb, cls, "P_USR_X",   mrb_fixnum_value(LIBSSH2_SFTP_S_IXUSR));

    mrb_define_const(mrb, cls, "P_GRP_RWX", mrb_fixnum_value(LIBSSH2_SFTP_S_IRWXG));
    mrb_define_const(mrb, cls, "P_GRP_R",   mrb_fixnum_value(LIBSSH2_SFTP_S_IRGRP));
    mrb_define_const(mrb, cls, "P_GRP_W",   mrb_fixnum_value(LIBSSH2_SFTP_S_IWGRP));
    mrb_define_const(mrb, cls, "P_GRP_X",   mrb_fixnum_value(LIBSSH2_SFTP_S_IXGRP));

    mrb_define_const(mrb, cls, "P_OTH_RWX", mrb_fixnum_value(LIBSSH2_SFTP_S_IRWXO));
    mrb_define_const(mrb, cls, "P_OTH_R",   mrb_fixnum_value(LIBSSH2_SFTP_S_IROTH));
    mrb_define_const(mrb, cls, "P_OTH_W",   mrb_fixnum_value(LIBSSH2_SFTP_S_IWOTH));
    mrb_define_const(mrb, cls, "P_OTH_X",   mrb_fixnum_value(LIBSSH2_SFTP_S_IXOTH));
}
