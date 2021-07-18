/*
 * string_util.h
 *
 * Functions that implement string operations.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#ifndef STRING_UTIL_H_
#define STRING_UTIL_H_

#include <stdbool.h>

#ifndef strlcpy
#include <stdio.h>

/** For systems without strlcpy() */
#define strlcpy(dst,src,size) snprintf((dst),(size),"%s",(src))
#endif

/**
 * Copy characters of the source to the
 * destination, applying a transformation
 * function (e.g. toupper()) to each one.
 * Source and destination can be the same.
 *
 * @param dest destination string
 * @param src source string
 * @param xfmfn a transformation function
 */
char *strapply(char *dest, const char *src, int (*xfmfn)(int));

/**
 * Returns true if src string ends with endswith string.
 *
 * @param src source string
 * @param endswith ends with string
 * @return true if src ends with endswith
 */
bool strendswith(const char *src, const char *endswith);

/**
 * Trims newline sequences "\r\n", "\r", or "\n".
 *
 * @param src source string
 * @return true if string was trimmed
 */
bool trim_newline(char *src);

#endif /* STRING_UTIL_H_ */
