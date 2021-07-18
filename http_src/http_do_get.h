/*
 * http_do_get.h
 *
 * Implement HTTP GET and HEAD methods.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#ifndef HTTP_DO_GET_H_
#define HTTP_DO_GET_H_

#include <stdio.h>
#include "properties.h"

/**
 * Handle HEAD request.
 *
 * @param the socket stream
 * @param uri the request URI
 * @param requestHeaders the request headers
 * @param responseHeaders the response headers
 */
void do_get(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders);

/**
 * Handle HEAD request.
 *
 * @param the socket stream
 * @param uri the request URI
 * @param requestHeaders the request headers
 * @param responseHeaders the response headers
 */
void do_head(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders);


#endif /* HTTP_DO_GET_H_ */
