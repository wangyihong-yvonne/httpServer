/*
 * http_do_delete.c
 *
 * Implement HTTP DELETE methods.
 *
 *  @since 2021-04-19
 *  @author: Xiaoli Ou
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>

#include "media_util.h"
#include "properties.h"
#include "string_util.h"
#include "file_util.h"
#include "time_util.h"
#include "http_server.h"
#include "http_util.h"
#include "http_codes.h"
#include "http_do_delete.h"

/**
 * Handle DELETE request.
 *
 * @param stream the socket stream
 * @param uri the request URI
 * @param requestHeaders the request headers
 * @param responseHeaders the response headers
 */
void do_delete(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders) {
    // get path to URI in file system
    char filePath[MAXPATHLEN];
    resolveUri(uri, filePath);
    FILE *contentStream = NULL;

    // ensure file exists
    struct stat sb;
    if (stat(filePath, &sb) != 0) {
        sendStatusResponse(stream, Http_NotFound, NULL, responseHeaders);
        return;
    }
    // directory path ends with '/'
    if (S_ISDIR(sb.st_mode) && strendswith(filePath, "/")) {
        if (rmdir(filePath) == 0) { // dir is empty and has been deleted successfully
            sendResponseStatus(stream, Http_OK, NULL);  // send response
            sendResponseHeaders(stream, responseHeaders);  // Send response headers
        } else {
            sendStatusResponse(stream, Http_MethodNotAllowed, NULL, responseHeaders);
        }
    } else if (!S_ISREG(sb.st_mode)) { // error if not regular file
        sendStatusResponse(stream, Http_NotFound, NULL, responseHeaders);
    } else { // delete file in server
        if (unlink(filePath) == 0) {  // delete successfully
            sendResponseStatus(stream, Http_OK, NULL);  // send response
            sendResponseHeaders(stream, responseHeaders);  // Send response headers
        } else {
            sendStatusResponse(stream, Http_MethodNotAllowed, NULL, responseHeaders);
        }
    }
}