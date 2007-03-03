#include "sftpserver.h"
#include "sftp.h"
#include "send.h"
#include "types.h"
#include "globals.h"
#include <assert.h>
#include <errno.h>
#include <string.h>

const char *status_to_string(uint32_t status) {
  switch(status) {
  case SSH_FX_OK: return "OK";
  case SSH_FX_EOF: return "end of file";
  case SSH_FX_NO_SUCH_FILE: return "file does not exist";
  case SSH_FX_PERMISSION_DENIED: return "permission denied";
  case SSH_FX_FAILURE: return "operation failed";
  case SSH_FX_BAD_MESSAGE: return "badly encoded SFTP packet";
  case SSH_FX_NO_CONNECTION: return "no connection";
  case SSH_FX_CONNECTION_LOST: return "connection lost";
  case SSH_FX_OP_UNSUPPORTED: return "operation not supported";
  case SSH_FX_INVALID_HANDLE: return "invalid handle";
  case SSH_FX_NO_SUCH_PATH: return "path does not exist or is invalid";
  case SSH_FX_FILE_ALREADY_EXISTS: return "file already exists";
  case SSH_FX_WRITE_PROTECT: return "file is on read-only medium";
  case SSH_FX_NO_MEDIA: return "no medium in drive";
  case SSH_FX_NO_SPACE_ON_FILESYSTEM: return "no space on filesystem";
  case SSH_FX_QUOTA_EXCEEDED: return "quota exceeded";
  case SSH_FX_UNKNOWN_PRINCIPAL: return "unknown principal";
  case SSH_FX_LOCK_CONFLICT: return "file is locked";
  case SSH_FX_DIR_NOT_EMPTY: return "directory is not empty";
  case SSH_FX_NOT_A_DIRECTORY: return "file is not a directory";
  case SSH_FX_INVALID_FILENAME: return "invalid filename";
  case SSH_FX_LINK_LOOP: return "too many symbolic links";
  case SSH_FX_CANNOT_DELETE: return "file cannot be deleted";
  case SSH_FX_INVALID_PARAMETER: return "invalid parameter";
  case SSH_FX_FILE_IS_A_DIRECTORY: return "file is a directory";
  case SSH_FX_BYTE_RANGE_LOCK_CONFLICT: return "byte range is locked";
  case SSH_FX_BYTE_RANGE_LOCK_REFUSED: return "cannot lock byte range";
  case SSH_FX_DELETE_PENDING: return "file deletion pending";
  case SSH_FX_FILE_CORRUPT: return "file is corrupt";
  case SSH_FX_OWNER_INVALID: return "invalid owner";
  case SSH_FX_GROUP_INVALID: return "invalid group";
  case SSH_FX_NO_MATCHING_BYTE_RANGE_LOCK: return "no such lock";
  default: return "unknown status";
  }
}

void send_status(struct sftpjob *job, 
                 uint32_t status,
                 const char *msg) {
  if(status == (uint32_t)-1) {
    /* Bodge to allow us to treat -1 as a magical status meaning 'consult
     * errno'.  This goes back via the protocol-specific status callback, so
     * statuses out of range for the current protocol version get properly
     * laundered. */
    send_errno_status(job);
    return;
  }
  /* If there is no message, fill one in */
  if(!msg)
    msg = status_to_string(status);
  /* Limit to status values known to this version of the protocol */
  if(status > protocol->maxstatus)
    status = SSH_FX_FAILURE;
  send_begin(job);
  send_uint8(job, SSH_FXP_STATUS);
  send_uint32(job, job->id);
  send_uint32(job, status);
  send_string(job, msg);
  send_string(job, "en");               /* we are not I18N'd yet */
  send_end(job);
}

static const struct {
  int errno_value;
  uint32_t status_value;
} errnotab[] = {
  { 0, SSH_FX_OK },
  { EPERM, SSH_FX_PERMISSION_DENIED },
  { EACCES, SSH_FX_PERMISSION_DENIED },
  { ENOENT, SSH_FX_NO_SUCH_FILE },
  { EIO, SSH_FX_FILE_CORRUPT },
  { ENOSPC, SSH_FX_NO_SPACE_ON_FILESYSTEM },
  { ENOTDIR, SSH_FX_NOT_A_DIRECTORY },
  { EISDIR, SSH_FX_FILE_IS_A_DIRECTORY },
  { EEXIST, SSH_FX_FILE_ALREADY_EXISTS },
  { EROFS, SSH_FX_WRITE_PROTECT },
  { ELOOP, SSH_FX_LINK_LOOP },
  { ENAMETOOLONG, SSH_FX_INVALID_FILENAME },
  { ENOTEMPTY, SSH_FX_DIR_NOT_EMPTY },
  { EDQUOT, SSH_FX_QUOTA_EXCEEDED },
  { -1, SSH_FX_FAILURE },
};

void send_errno_status(struct sftpjob *job) {
  int n;
  const int errno_value = errno;

  for(n = 0; 
      errnotab[n].errno_value != errno_value && errnotab[n].errno_value != -1;
      ++n)
    ;
  send_status(job, errnotab[n].status_value, strerror(errno_value));
}

void send_ok(struct sftpjob *job) {
  send_status(job, SSH_FX_OK, 0);
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
