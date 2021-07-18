/*
 * http_server.c
 *
 * The HTTP server main function sets up the listener socket
 * and dispatches client requests to request sockets.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "file_util.h"
#include "time_util.h"
#include "http_request.h"
#include "network_util.h"
#include "properties.h"
#include "http_server.h"
#include "media_util.h"
#include "thpool.h"


#define DEFAULT_HTTP_PORT 8080

/** http server configuration */
struct http_server_conf server;

/**
 * Process the server configuration file
 * @param configFileName name of the configuration file
 * @return true if successful, false if error occurred
 */
static bool process_config(const char* configFileName) {
	bool status = true;
	Properties* httpConfig = newProperties();

	do {
		// load properties from config file
		if (loadProperties(configFileName, httpConfig) == 0) {
			fprintf(stderr, "Missing configuration file '%s'\n", configFileName);
			status = false;
			break;
		}

		// initialize debug flag
		char debugProp[MAX_PROP_VAL];
		if (findProperty(httpConfig, 0, "Debug", debugProp) != SIZE_MAX) {
			server.debug =  (strcasecmp(debugProp, "true") == 0);
		}

		// server root directory relative to config file path
		char configFilePath[PATH_MAX];
		if (getPath(configFileName, configFilePath) != NULL) {
			if (chdir(configFilePath) != 0) {  // should not happen
				perror("configFilePath");
				status = false;
				break;
			}
		}

		// set server root directory
		char rootDirProp[MAX_PROP_VAL];
		if (findProperty(httpConfig, 0, "ServerRoot", rootDirProp) != SIZE_MAX) {
			if (chdir(rootDirProp) != 0) {
				perror("setServerRoot");
				status = false;
				break;
			}
		}

        // read media type
        char contentTypes[MAXBUF] = "mime.types";
		if (findProperty(httpConfig, 0, "ContentTypes", contentTypes) != SIZE_MAX) {
		    if (readMediaType(contentTypes) == 0) { // empty
                perror("setMediaType");
                status = false;
                break;
		    }  // readMediaType("mime.types");
		}

		// initialize the listener port
		server.server_port = DEFAULT_HTTP_PORT;
		char listenProp[MAX_PROP_VAL];
		if (findProperty(httpConfig, 0, "Port", listenProp) != SIZE_MAX) {
			if (   (sscanf(listenProp, "%d", &server.server_port) != 1)
				|| !isRegisteredPort(server.server_port)) {
				fprintf(stderr, "Invalid port %s\n", listenProp);
				status = false;
				break;
			}
		}

		// set content base property if specified or use default "content"
		static char contentBaseProp[MAX_PROP_VAL] = "content";
		server.content_base = contentBaseProp;
		findProperty(httpConfig, 0, "ContentBase", contentBaseProp);

		// set server host property or use default "localhost"
		static char serverHostProp[MAX_PROP_VAL] = "localhost";
		server.server_host = serverHostProp;
		findProperty(httpConfig, 0, "ServerHost", serverHostProp);

		// set default server name property if not set
		static char serverNameProp[MAX_PROP_VAL];
		server.server_name = serverNameProp;
		if (findProperty(httpConfig, 0, "ServerName", serverNameProp) == SIZE_MAX) {
			// set default server name as serverhost:port
			sprintf(serverNameProp,"%s:%d", server.server_host, server.server_port);
		}

		// set server response protocol property or use default "HTTP/1.1"
		static char serverProtocolProp[MAX_PROP_VAL] = "HTTP/1.1";
		server.server_protocol = serverProtocolProp;
		findProperty(httpConfig, 0, "ServerProtocol", serverProtocolProp);

	} while(false);

	deleteProperties(httpConfig);
	return status;
}

/**
 * Main program starts the server and processes requests
 * @param argc argument count
 * @param argv array of args; argv[1] may be port no.
 */

int main(int argc, char* argv[argc]) {
	// initialize config file name
	const char* configFileName = "httpd.conf";
    if (argc == 2) {
    	configFileName = argv[1];
    }

	// load property file with server configuration
	if (!process_config(configFileName)) {
		return EXIT_FAILURE;
	}

    // create listener socket for server with specified port
    int listen_sock_fd = get_listener_socket(server.server_port);
	if (listen_sock_fd == -1) {
		perror("listen_sock_fd");
		return EXIT_FAILURE;
	}

	if (server.debug) {
		fprintf(stderr, "HttpServer running on port %d\n", server.server_port);
	}

    // init the thread pool
    threadpool thpool = thpool_init(4);

	while (true) {
        // accept client connection
        int peer_socket_fd = accept_peer_connection(listen_sock_fd);
        if (peer_socket_fd == -1) {
            if (errno == EINTR) {  // interrupted by signal
                continue;
            }
            // report error and quit
            if (server.debug) {
                perror("accept_peer_connection");
            }
            close(listen_sock_fd);
            return EXIT_FAILURE;
        }

        if (server.debug) {
            int port;
            char host[HOST_NAME_MAX];
            if (get_peer_host_and_port(peer_socket_fd, host, &port) == -1) {
                perror("get_peer_host_and_port");
            } else {
                fprintf(stderr, "New connection accepted  %s:%u\n", host, port);
            }
        }

		// handle request
        thpool_add_work(thpool, process_request_helper, (void *)(intptr_t)peer_socket_fd);
    }
	 thpool_destroy(thpool);
    // close listener socket
    close(listen_sock_fd);
    return EXIT_SUCCESS;

}
