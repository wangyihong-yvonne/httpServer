/*
 * http_do_get.c
 *
 * Implements HTTP GET and HEAD methods.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>

#include "media_util.h"
#include "properties.h"
#include "string_util.h"
#include "file_util.h"
#include "time_util.h"
#include "http_server.h"
#include "http_util.h"
#include "http_codes.h"
#include "http_do_get.h"


/**
 * Handle GET or HEAD request.
 *
 * @param stream the socket stream
 * @param uri the request URI
 * @param requestHeaders the request headers
 * @param responseHeaders the response headers
 * @param sendContent send content (GET)
 */
static void do_get_or_head(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders, bool sendContent) {
	// get path to URI in file system
	char filePath[MAXPATHLEN];
	resolveUri(uri, filePath);
	FILE *contentStream = NULL;

	// ensure file exists
	struct stat sb;
	if (stat(filePath, &sb) != 0) { // works for dir and file
	    // If the directory does not exist, return a "404 "Not Found" response
		sendStatusResponse(stream, Http_NotFound, NULL, responseHeaders);
		return;
	}
	// directory path ends with '/'
	if (S_ISDIR(sb.st_mode) && strendswith(filePath, "/")) {
		// generates the directory listing and returns it as the body of the response
        contentStream = tmpStringFile(generateList(filePath));
        // get property
        struct stat sbList;
        fileStat(contentStream, &sbList);
        // record the last-modified date/time
        char time[MAXBUF];
        time_t timer = sb.st_mtime;
        putProperty(responseHeaders,"Last-Modified",
                    milliTimeToRFC_1123_Date_Time(timer, time));

        // get mime type of file
        char mediaType[MAX_PROP_VAL];
        getMediaType(filePath, mediaType);
        if (strcmp(mediaType, "text/directory") == 0) {
            // some browsers interpret text/directory as a VCF file
            strcpy(mediaType,"text/html");
        }
        putProperty(responseHeaders, "Content-type", mediaType);

        // Send response headers
        sendResponseStatus(stream, Http_OK, NULL);
        sendResponseHeaders(stream, responseHeaders);
        // copy the html file as the steam
        copyFileStreamBytes(contentStream, stream, (size_t)sbList.st_size);
        fclose(contentStream);
		return;
	} else if (!S_ISREG(sb.st_mode)) { // error if not regular file
		sendStatusResponse(stream, Http_NotFound, NULL, responseHeaders);
		return;
	}

	// record the last-modified date/time
    char buf[MAXBUF];
	time_t timer = sb.st_mtime;
	putProperty(responseHeaders,"Last-Modified",
				milliTimeToRFC_1123_Date_Time(timer, buf));

	// get mime type of file
    char mediaType[MAX_PROP_VAL];
	getMediaType(filePath, mediaType);
	if (strcmp(mediaType, "text/directory") == 0) {
		// some browsers interpret text/directory as a VCF file
		strcpy(mediaType,"text/html");
	}
	putProperty(responseHeaders, "Content-type", mediaType);

    // get file length
    size_t contentLen = (size_t)sb.st_size;

    // set content length or chunked transfer encoding if requested
    // char buf[MAXBUF];
    bool chunked = false;
    if ( (findProperty(requestHeaders, 0, "Transfer-Encoding", buf) != SIZE_MAX)
           && (strcmp(buf, "chunked") == 0)
           && sendContent) { // only chunk if sending content
        // record transfer encoding
        chunked = true;  // for curl, use -H "Transfer-Encoding:chunked"
        putProperty(responseHeaders, "Transfer-Encoding", "chunked");
    } else {
        // record file length
        sprintf(buf, "%lu", contentLen);
        putProperty(responseHeaders, "Content-Length", buf);
    }

    // send response
	sendResponseStatus(stream, Http_OK, NULL);

	// Send response headers
	sendResponseHeaders(stream, responseHeaders);

	// send content for GET
	if (sendContent) {
		contentStream = fopen(filePath, "r");
        if (chunked) {
            copyToChunkedFileStreamBytes(contentStream, stream, contentLen);
        } else {
            copyFileStreamBytes(contentStream, stream, contentLen);
        }
		fclose(contentStream);
	}
}

/**
 * Handle GET request.
 *
 * @param stream the socket stream
 * @param uri the request URI
 * @param requestHeaders the request headers
 * @param responseHeaders the response headers
 * @param headOnly only perform head operation
 */
void do_get(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders) {
	do_get_or_head(stream, uri, requestHeaders, responseHeaders, true);
}

/**
 * Handle HEAD request.
 *
 * @param the socket stream
 * @param uri the request URI
 * @param requestHeaders the request headers
 * @param responseHeaders the response headers
 */
void do_head(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders) {
	do_get_or_head(stream, uri, requestHeaders, responseHeaders, false);
}
