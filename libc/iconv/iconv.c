#include <iconv.h>
#include <errno.h>
#include <stdlib.h>

struct _iconv_state {
	char *tocode;
	char *fromcode;
};

iconv_t iconv_open(const char *tocode, const char *fromcode) {
	errno = EINVAL;
	return (iconv_t)-1;
}

int iconv_close(iconv_t cd) {
	return 0;
}

size_t iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft) {
	return -1;
}
