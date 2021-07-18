/*
 * http_do_delete.h
 *
 * Implement HTTP DELETE methods.
 *
 *  @since 2021-04-19
 *  @author: Xiaoli Ou
 */

#ifndef HTTP_DO_DELETE_H_
#define HTTP_DO_DELETE_H_

#include <stdio.h>
#include "properties.h"

/**
 * Handle DELETE request.
 *
 * @param the socket stream
 * @param uri the request URI
 * @param requestHeaders the request headers
 * @param responseHeaders the response headers
 */
void do_delete(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders);



#endif /* HTTP_DO_DELETE_H_ */