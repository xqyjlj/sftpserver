#include "sftpcommon.h"
#include "utils.h"
#include "charset.h"
#include "alloc.h"
#include "debug.h"
#include <string.h>
#include <assert.h>

wchar_t *convertm2w(const char *s) {
  wchar_t *ws;
  size_t len;
  mbstate_t ps;
  
  memset(&ps, 0, sizeof ps);
  len = mbsrtowcs(0, &s, 0, &ps);
  if(len == (size_t)-1)
    return 0;
  ws = xcalloc(len + 1, sizeof *ws);
  memset(&ps, 0, sizeof ps);
  mbsrtowcs(ws, &s, len, &ps);
  return ws;
}

int iconv_wrapper(struct allocator *a, iconv_t cd, char **sp) {
  const char *const input = *sp;
  const size_t inputsize = strlen(input);
  size_t outputsize = 2 * strlen(input) + 1;
  size_t rc;
  const char *inbuf;
  char *outbuf, *output;
  size_t inbytesleft, outbytesleft;
  
  assert(cd != 0);
  do {
    output = alloc(a, outputsize);
    iconv(cd, 0, 0, 0, 0);
    inbuf = input;
    inbytesleft = inputsize;
    outbuf = output;
    outbytesleft = outputsize;
    rc = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    outputsize *= 2;
  } while(rc == (size_t)-1 && errno == E2BIG);
  if(rc == (size_t)-1)
    return -1;
  *sp = output;
  return 0;
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
