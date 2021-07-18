/*
 * http_util.h
 *
 * Functions used to implement http operations.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#ifndef HTTP_UTIL_H_
#define HTTP_UTIL_H_

#include "properties.h"

/**
 * Reads request headers from request stream until empty line.
 *
 * @param istream the socket input stream
 * @param request headers
 * @throws IOException if error occurs
 */
void readRequestHeaders(FILE *istream, Properties *requestHeader);

/**
 * Send bytes for status to response output stream.
 *
 * @param status the response status
 * @param statusMsg the response message
 *   (NULL for default message)
 */
void sendResponseStatus(FILE *ostream, int status, const char *statusMsg);

/**
 * Send bytes for headers to response output stream.
 *
 * @param responseHeaders the header name value pairs
 * @param responseCharset the response charset
 * @throws IOException if errors occur
 */
void sendResponseHeaders(FILE *ostream, Properties *responseHeaders);

/**
 * Set error response and error page to the response output stream.
 *
 * @param ostream the output socket stream
 * @param status the response status
 * @param statusMsg the response message
 *   (NULL for default response message)
 * @param responseHeaders the response headers
 */
void sendStatusResponse(FILE* ostream, int status, const char *statusMsg, Properties *responseHeaders);

/**
 * Decode a URI string by replacing %xx with the
 * corresponding character code and '+' with " ".
 * @param escUrl the esc URI
 * @param uri the decoded URI
 * @return the URL if successful, NULL if error
 */
char *unescapeUri(const char *escUri, char *uri);

/**
 * Resolves server URI to file system path.
 * @param uri the request URI
 * @param fspath the file system path
 * @return the file system path
 */
char *resolveUri(const char *uri, char *fspath);

/**
 * Decode query string.
 *
 * @parma query the query string (e.g. a=b&c=d&e=f);
 * @param queryProps the query parameters
 */
void decodeQuery(const char *query, Properties *queryProps);

/**
 * Debug request by printing request and request headers
 *
 * @param request the request line
 * @param requestHeaders the request headers
 */
void debugRequest(const char *request, Properties *requestHeaders);

/**
 * Copy bytes from HTTP 1.1 chunked transfer-encoded
 * input stream to output stream.
 * See:
 * https://gist.github.com/CMCDragonkai/6bfade6431e9ffb7fe88
 * https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
 * https://en.wikipedia.org/wiki/Chunked_transfer_encoding
 *
 * @param istream the chunk transfer-encoded input stream
 * @param ostream the output stream
 * @param return number of bytes copied or -1 if error
 */
int copyFromChunkedFileStreamBytes(FILE *istream, FILE *ostream);

/**
 * Copy bytes from input stream to HTTP 1.1 chunked transfer-encoded
 * output stream.
 * See:
 * https://gist.github.com/CMCDragonkai/6bfade6431e9ffb7fe88
 * https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
 * https://en.wikipedia.org/wiki/Chunked_transfer_encoding
 *
 * @param istream input stream
 * @param ostream the the chunk transfer-encoded output stream
 * @param nbytes the number of bytes to send
 * @param return 0 if successful or -1 if error
 */
int copyToChunkedFileStreamBytes(FILE *istream, FILE *ostream, int nbytes);

#endif /* HTTP_UTIL_H_ */
