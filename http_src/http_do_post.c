/*
* http_do_post.c
*
* Implement HTTP POST methods.
*
*  @since 2021-04-20
*
*/

#include <sys/param.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
//#include <AppleTextureEncoder.h>
//#include <AppleEXR.h>
#include "http_codes.h"
#include "http_do_post.h"
#include "properties.h"
#include "string_util.h"
#include "file_util.h"
#include "http_server.h"
#include "http_util.h"


/**
 *
 * @param stream HTTP socket stream
 * @param uri  request URI
 * @param requestHeaders
 * @param responseHeaders
 */
void do_post(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders) {

    char filePath[MAXPATHLEN];
    resolveUri(uri, filePath);
    FILE *putStream = NULL;
    char buf[MAXBUF];

    if (findProperty(requestHeaders, 0, "Content-Length", buf) == SIZE_MAX
        && findProperty(requestHeaders, 0, "Transfer-Encoding", buf) == SIZE_MAX)
    {
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

    if (strendswith(filePath, "/"))
    {
        sendStatusResponse(stream, Http_MethodNotAllowed, NULL, responseHeaders);
        return;
    }
    putProperty(responseHeaders, "Location", filePath);
    if (mkdirs(filePath, 0755) < 0)
    {
        sendStatusResponse(stream, Http_MethodNotAllowed, NULL, responseHeaders);
        return;
    }


    char fileName[MAXPATHLEN];
    strcpy(fileName, filePath);
    //rename the file
    strcat(fileName, "/rdm_file_XXX");
    int tempFile;
    findProperty(requestHeaders, 0, "Content-Type", buf);

    // transfer file types
    if (strcmp(buf, "multipart/form-data") == 0)
    {
        strcat(fileName, ".mime");
        tempFile = mkstemps(fileName, 6);
    }
    else if (strcmp(buf, "text/plain") == 0)
    {
        strcat(fileName, ".txt");
        tempFile = mkstemps(fileName, 5);
    }
    else if (strcmp(buf, "application/x-www-form-urlencoded") == 0)
    {
        strcat(fileName, ".urlencoded");
        tempFile = mkstemps(fileName, 13);
    }
    else {
        strcat(fileName, ".bin");
        tempFile = mkstemps(fileName, 5);
    }

    putStream = fdopen(tempFile, "w");
    if (len==-1) {
        copyFromChunkedFileStreamBytes(stream, putStream);
    } else {
        copyFileStreamBytes(stream, putStream,len);
    }
    fclose(putStream);
    sendStatusResponse(stream, Http_Created, NULL, responseHeaders);
}

