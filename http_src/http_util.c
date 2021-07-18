/*
 * http_util.c
 *
 * Functions used to implement http operations.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#include <stdio.h>
#include <string.h>
#include "properties.h"
#include "file_util.h"
#include "string_util.h"
#include "http_codes.h"
#include "http_server.h"


/**
 * Reads request headers from request stream until empty line.
 *
 * @param istream the socket input stream
 * @param request headers
 */
void readRequestHeaders(FILE *istream, Properties *requestHeaders) {
	char buf[MAXBUF];

	while (fgets(buf, MAXBUF, istream) != NULL) {
		// trim newline characters
		trim_newline(buf);

		// empty line marks send of headers
		if (*buf == '\0') {
			break;
		}

		char *p = strchr(buf, ':'); // find name/value delimiter
		if (p != NULL) {
			for (*p++ = '\0'; *p == ' '; p++) {} // skip over leading value whitespace
			if (!putProperty(requestHeaders, buf, p)) {  // save request property
				fprintf(stderr, "readRequestHeaders requestHeaders full\n");
				break;
			}
		}
	}
}

/**
 * Send bytes for status to response output stream.
 *
 * @param ostream the output socket stream
 * @param status the response status
 * @param statusMsg the response message
 *   (NULL for default message)
 */
void sendResponseStatus(FILE *ostream, int status, const char *statusMsg) {
    if (statusMsg == NULL) {
        statusMsg = httpCodeStr(status);
    }
	fprintf(ostream, "%s %d %s %s", server.server_protocol, status, statusMsg, CRLF);
	if (server.debug) {
		fprintf(stderr, "%s %d %s\n", server.server_protocol, status, statusMsg);
	}
}


/**
 * Send bytes for headers to response output stream
 * with terminating blank line.
 *
 * @param responseHeaders the header name value pairs
 * @param responseCharset the response charset
 */
void sendResponseHeaders(FILE *ostream, Properties *responseHeaders) {
	// output headers
	char name[MAX_PROP_NAME], val[MAX_PROP_VAL];
	for (int i = 0; getProperty(responseHeaders, i, name, val); i++) {
		fprintf(ostream, "%s: %s%s", name, val, CRLF);
    	if (server.debug) {
    		fprintf(stderr, "%s: %s\n", name, val);
    	}
	}

	// Send a blank line to indicate the end of the header lines.
	fprintf(ostream, "%s", CRLF);
	if (server.debug) {
		fprintf(stderr, "\n");
	}
}

/**
 * Set status response and status page to the response output stream.
 *
 * @param ostream the output socket stream
 * @param status the response status
 * @param statusMsg the response message
 *   (NULL for default response message)
 * @param responseHeaders the response headers
 */
void sendStatusResponse(FILE* ostream, int status, const char *statusMsg, Properties *responseHeaders) {
    if (statusMsg == NULL) {  // use default message
        statusMsg = httpCodeStr(status);
    }
	sendResponseStatus(ostream, status, statusMsg);

	char errorBody[2*MAXBUF];  // because of data substitution.
	const char *errorPage =
		"<html>"
	    "<head><title>%d %s</title></head>"
	    "<body>%d %s</body></html>";
	sprintf(errorBody, errorPage, status, statusMsg, status, statusMsg);
	FILE *tmpStream = tmpStringFile(errorBody);

	char buf[MAXBUF];
	size_t contentLen = strlen(errorBody);
	sprintf(buf, "%lu", contentLen);
	putProperty(responseHeaders,"Content-Length", buf);
	putProperty(responseHeaders,"Content-type", "text/html");

	// Send the headers
	sendResponseHeaders(ostream, responseHeaders);

	// Send the error page body.
	copyFileStreamBytes(tmpStream, ostream, contentLen);
	fclose(tmpStream);
}

/**
 * Decode a URI string by replacing %xx with the
 * corresponding character code and '+' with " ".
z * @param escUrl the esc URI
 * @param uri the decoded URI
 * @return the URL if successful, NULL if error
 */
char *unescapeUri(const char *escUri, char *uri) {
	char *p = uri;
	while (*escUri) {
		if ( *escUri == '%') {
            int c;
            if (sscanf(escUri, "%%%02x", &c) == 0) {
                return NULL;
            }
            *p++ = (unsigned char) c;
            escUri += 3;
        } else if (*escUri == '+') { // spaces encoded as "+"
		    *p++ = ' ';
            escUri++;
		} else {
			*p++ = *escUri++;
		}
	}
	*p++ = '\0';
	return uri;
}

/**
 * Resolves server URI to file system path.
 * @param uri the request URI
 * @param fspath the file system path
 * @return the file system path
 */
char *resolveUri(const char *uri, char *fspath) {
	strcpy(fspath, server.content_base);
	strcat(fspath, uri);
	return fspath;
}

/**
 * Debug request by printing request and request headers
 *
 * @param request the request line
 * @param requestHeaders the request headers
 */
void debugRequest(const char *request, Properties *requestHeaders) {
	char name[MAX_PROP_NAME], val[MAX_PROP_VAL];
	fprintf(stderr, "\n%s\n", request);
	for (int i = 0; getProperty(requestHeaders, i, name, val); i++) {
		fprintf(stderr, "%s: %s\n", name, val);
	}
	fprintf(stderr, "\n");
}

/**
 * Decode query string.
 *
 * @parma query the query string (e.g. a=b&c=d&e=f);
 * @param queryProps the query parameters
 */
void decodeQuery(const char *query, Properties *queryProps) {
    char querytoks[strlen(query)+1];
    strcpy(querytoks, query);  // strtok_r modifies string
    char *saveptr;  // for re-entrant strtok_r
    for (char* qt = querytoks; ; qt = NULL) {
        // get next name/value
        char* tok = strtok_r(qt, "&;", &saveptr); // either & or ;
        if (tok == NULL) {
            break;
        }
        // split into name and value
        char *valp = strchr(tok, '=');
        if (valp == NULL) {
            valp = tok+strlen(tok); // non-standard: no value
        } else {
            *valp++ = '\0';  // split into name/value
        }

        // decode name and value
        char name[strlen(tok) + 1];
        char value[strlen(valp) + 1];
        unescapeUri(tok, name);
        unescapeUri(valp, value);
        putProperty(queryProps, name, value);
    }
}

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
int copyFromChunkedFileStreamBytes(FILE *istream, FILE *ostream) {
    char buf[MAXBUF];  // buffer for reading lines
    int nbytes = 0; // number of bytes copied
    int chunk_len; // length of next chunk

    do { // copy next chunk
        // get chunk length from first line
        // note: ignores chunk extensions, where chunk length
        // is delimited by ';' and extension name=value pairs,
        // each delimited by ';' see
        // https://www.wikipedia.org/Chunked_transfer_encoding
        if (   (fgets(buf, MAXBUF, istream) == NULL)
               || (sscanf(buf, "%x", &chunk_len) != 1)
               || (chunk_len < 0)) {
            return -1; // missing or invalid negative chunk_len
        }

        // copy chunk_len bytes to output stream
        if (copyFileStreamBytes(istream, ostream, chunk_len) != 0) {
            return -1; // unexpected copy error
        }
        nbytes += chunk_len;

        // skip past newline after chunk
        if (fgets(buf, MAXBUF, istream) == NULL) {
            return -1; // unexpected end of input
        }
    } while (chunk_len > 0); // 0-length chunk ends chunk input stream

    return nbytes;
}

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
 * @param return 0 if successful, -1 if error
 */
int copyToChunkedFileStreamBytes(FILE *istream, FILE *ostream, int nbytes) {
    static const int chunk_size = 4096;

    while(nbytes > 0) {  // write next chunk
        int nwrite = (nbytes > chunk_size) ? chunk_size : nbytes;
        nbytes -= nwrite;
        if (   (fprintf(ostream, "%x\r\n", nwrite) < 0)
               || (copyFileStreamBytes(istream, ostream, nwrite) != 0)
               || (fprintf(ostream, "\r\n") < 0)){
            return -1;
        }
    }
    // write ending chunk
    return ((fprintf(ostream, "0\n\r\n\r") < 0) ? -1 : 0);
}