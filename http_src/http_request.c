/*
 * http_request.c
 *
 * Functions used to process requests from clients.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <dirent.h>
#include "http_do_get.h"
#include "http_do_post.h"
#include "http_do_delete.h"
#include "http_codes.h"
#include "http_server.h"
#include "http_util.h"
#include "time_util.h"
#include "media_util.h"
#include "properties.h"
#include "string_util.h"
#include "file_util.h"
#include "http_do_put.h"
/**
 *  Process an http request.
 *  @param sock_fd the socket descriptor
 */

void process_request(int sock_fd) {
	char buf[MAXBUF];
	char request[MAXBUF];
	char method[MAXBUF];
	char uri[MAXBUF], encUri[MAXBUF];
	char version[MAXBUF];

	// open socket aFILE *stream
	FILE *stream = fdopen(sock_fd, "r+");
	if (stream == NULL) {
		perror("fdopen");
		return;
	}
	// turn off buffering to also allow direct use of socket
	setvbuf(stream, NULL, _IONBF, 0);

	// get header line
	if (fgets(request, MAXBUF, stream) == NULL) {
		return;
	}
	// eliminate newline from request
	trim_newline(request);

	// initialize request headers
	Properties *responseHeaders = newProperties();
	// name of server
	putProperty(responseHeaders, "Server", server.server_name);

	// date and time of this response
	time_t timer;
	time(&timer); // need to get local file time?
	putProperty(responseHeaders,"Date",
				milliTimeToRFC_1123_Date_Time(timer, buf));


	// parse header
	if (sscanf(request, "%s %s %s", method, encUri, version) != 3) {
		if (server.debug) {
			fprintf(stderr, "request header incomplete: %s\n", request);
		}
		sendStatusResponse(stream, Http_BadRequest, NULL, responseHeaders);
		return;
	}

	// initialize request headers
	Properties *requestHeaders = newProperties();
	readRequestHeaders(stream, requestHeaders);
	if (server.debug) {
		debugRequest(request, requestHeaders);
	}

	// save query parameters as request header key "?"
	char *p = strpbrk(encUri,"?&");  // query separators
	if (p != NULL) {
		putProperty(requestHeaders, "?", p+1);
		*p = '\0';
	}

	// unescape URI
	if (unescapeUri(encUri, uri) == NULL) {
		if (server.debug) {
			fprintf(stderr, "request header invalid URI encoding %s\n", request);
		}
		sendStatusResponse(stream, Http_BadRequest, NULL, responseHeaders);
		return;
	}

    // dispatch based on method
    // what method should be for list directory
    if (strcasecmp(method, "GET") == 0) {
        do_get(stream, uri, requestHeaders, responseHeaders);
    } else 	if (strcasecmp(method, "HEAD") == 0) {
        do_head(stream, uri, requestHeaders, responseHeaders);
    } else if (strcasecmp(method, "DELETE") == 0) {
        do_delete(stream, uri, requestHeaders, responseHeaders);
    } else if (strcasecmp(method, "PUT") == 0) {
        do_put(stream, uri, requestHeaders, responseHeaders);
    }else if (strcasecmp(method, "POST") == 0) {
        do_post(stream, uri, requestHeaders, responseHeaders);
    } else {
        sendStatusResponse(stream, Http_NotImplemented, NULL, responseHeaders);
    }

	// delete headers
	deleteProperties(requestHeaders);
	deleteProperties(responseHeaders);

	// close socket stream
	fflush(stream);
	fclose(stream);
	close(sock_fd);
}

void process_request_helper(void* sock_fd) {
    int a = (int)sock_fd;
    process_request(sock_fd);
}

