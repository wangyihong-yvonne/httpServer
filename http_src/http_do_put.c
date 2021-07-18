/*
* http_do_put.c
*
* Implement HTTP PUT methods.
*
*  @since 2021-04-20
*
*/


#include <stddef.h>
#include <string.h>
#include "http_server.h"
#include "http_do_put.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "http_util.h"
#include "http_codes.h"
#include "file_util.h"
#include "media_util.h"
#include "properties.h"
#include "string_util.h"



/**
 *
 * @param stream HTTP socket stream
 * @param uri  Request URI
 * @param requestHeaders
 * @param responseHeaders
 */
void do_put(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders) {

    char filePath[MAXPATHLEN];
    resolveUri(uri, filePath);
    FILE *putStream = NULL;
    enum HttpCode status;
    char buf[MAXBUF];

    if (findProperty(requestHeaders, 0, "Content-Length", buf) == SIZE_MAX
    && findProperty(requestHeaders, 0, "Transfer-Encoding", buf) == SIZE_MAX) {
        sendStatusResponse(stream, Http_LengthRequired, NULL, responseHeaders);
        return;
    }
    int len = atoi(buf);
    if (findProperty(requestHeaders, 0, "Transfer-Encoding", buf) != SIZE_MAX)
    {
        if(strcmp(buf,"chunked")==0){
            len=-1;
            putProperty(responseHeaders, "Transfer-Encoding", "chunked");
        }
        else{
            sendStatusResponse(stream, Http_MethodNotAllowed, NULL, responseHeaders);
            return;
        }
    }
    getMediaType(filePath, buf);

    if (strcmp(buf, "text/directory") == 0) {
        strcpy(buf,"text/html");
    }
    putProperty(responseHeaders, "Content-type", buf);
    if (strendswith(filePath, "/")) {
        sendStatusResponse(stream, Http_MethodNotAllowed, NULL, responseHeaders);
        return;
    }
    putProperty(responseHeaders, "Location", filePath);


    char path[MAXPATHLEN];
    getPath(filePath, path);
    if (mkdirs(path, 0755) < 0) {
        sendStatusResponse(stream, Http_MethodNotAllowed, NULL, responseHeaders);
        return;
    }


    struct stat sb;
    if (stat(filePath, &sb) != 0) {
        status=Http_Created;
    } else {
        status=Http_OK;
    }
    sendResponseStatus(stream,status,NULL);

    putStream = fopen(filePath, "w");
    if (len==-1) {
        copyFromChunkedFileStreamBytes(stream, putStream);
    }
    else {
        copyFileStreamBytes(stream, putStream,len);
    }

    fclose(putStream);
    sendResponseHeaders(stream, responseHeaders);
}


