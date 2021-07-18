/*
 * string_util.h
 *
 * Functions that implement string operations.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "string_util.h"

/**
 * Copy characters of the source to the
 * destination, applying a transformation
 * function (e.g. toupper()) to each one.
 * Source and destination can be the same.
 *
 * @param dest destination string
 * @param src source string
 * @param xfmfn transformation function
 */
char *strapply(char *dest, const char *src, int (*xfmfn)(int))
{
    int i;
    for (i = 0; src[i] != '\0'; i++) {
        dest[i] = (char)xfmfn(src[i]);
    }
    dest[i] = '\0';
    return dest;
}

/**
 * Returns true if src string ends with endswith string.
 *
 * @param src source string
 * @param endswith ends with string
 * @return true if http_src ends with endswith
 */
bool strendswith(const char *src, const char *endswith) {
	size_t srclen = strlen(src);
	size_t endslen = strlen(endswith);
	return (srclen >= endslen) && (strcmp(src+srclen-endslen, endswith) == 0);
}

/**
 * Trims newline sequences '"\r\n", "\r", or "\n".
 *
 * @param src source string
 * @return true if string was trimmed
 */
bool trim_newline(char *src) {
    size_t len = strlen(src);
    bool trimmed = false;  // not trimmed

    // trim '\n' at end of string
    if ((len > 0) && (src[len-1] == '\n')) {
        src[--len] = '\0';  // shorten string
        trimmed = true;
    }

    // trim '\r' at end of string
    if ((len > 0) && (src[len-1] == '\r')) {
        src[--len] = '\0';
        trimmed = true;
    }

    return trimmed;
}
