/*
 * file_util.c
 *
 * Utilities functions for working with C FILE streams.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include "http_server.h"
#include "file_util.h"
#include "time_util.h"

/**
 * This function creates a temporary stream for this string.
 * When the FILE is closed, it will be automatically removed.
 *
 * @param contentStr the content of the file
 * @return the FILE for this string
 */
FILE *tmpStringFile(const char *contentStr) {
    FILE *tmpstream = tmpfile();
	fwrite(contentStr, 1, strlen(contentStr), tmpstream);
	rewind(tmpstream);
	return tmpstream;
}

char* generateList(const char *filePath) {
    // get root path
     char rootPath[MAXBUF];
     strcpy(rootPath, server.content_base);
     strcat(rootPath, "/");

    char contentStr[200000];
    // process the header of the html
    char header[5000];
    sprintf(header, "<html>\n"
                    "<head>\n"
                    "  <title>%s</title>\n"
                    "</head>\n"
                    "<body>\n"
                    "  <h1>%s</h1>\n"
                    "  <table>\n"
                    "  <tr>\n"
                    "    <th valign=\"top\"></th>\n"
                    "    <th>Name</th>\n"
                    "    <th>Last modified</th>\n"
                    "    <th>Size</th>\n"
                    "    <th>Description</th>\n"
                    "  </tr>\n"
                    "  <tr>\n"
                    "    <td colspan=\"5\"><hr></td>\n"
                    "  </tr>", filePath, filePath);
    strcpy(contentStr, header);


    struct dirent *de; // Pointer for directory entry
    DIR *dr = opendir(filePath);

    while ((de = readdir(dr)) != NULL) {  // get entry
        char childRow[20000];
        char *name = de->d_name;

        // exclude ".";
        if (strcmp(name, ".") == 0) {
            continue;
        }

        // exclude ".." of rootPath;
        if (strcmp(filePath, rootPath) == 0 && strcmp(name, "..") == 0) {
            continue; // root dir
        }

        // get filePath of entry
        char path[MAXBUF];
        strcpy(path, filePath);
        strcat(path, name); // "dir filepath + name -> child path
        char locate[MAXBUF];
        strcpy(locate, name);
        if (de->d_type == DT_DIR) {
            strcat(path, "/");
            strcat(locate, "/");
        }

        // get properties
        struct stat sb;
        stat(path, &sb);
        int size = sb.st_size;
        time_t timer = sb.st_mtime;
        char time[MAXBUF];
        milliTimeToRFC_1123_Date_Time(timer, time);

        // process the table row of the html
        if (strcmp(name, "..") == 0) {
            sprintf(childRow, "<tr>\n"
                              "    <td>&#x23ce</td>\n"
                              "    <td><a href=\"%s\">Parent Directory</a></td>\n"
                              "    <td align=\"right\">%s</td>\n"
                              "    <td align=\"right\">%d</td>\n"
                              "    <td></td>\n"
                              "  </tr>", name, time, size);
        } else {
            sprintf(childRow, "<tr>\n<td></td>\n"
                              "    <td><a href=\"%s\">%s</a></td>\n"
                              "    <td align=\"right\">%s</td>\n"
                              "    <td align=\"right\">%d</td>\n"
                              "    <td></td>\n"
                              "  </tr>", locate, name, time, size);  ////name, name, time, size
        }

        // copy it to the contentStr
        strcat(contentStr, childRow);
    }
    closedir(dr);
    // process the footer
    char footer[2048];
    sprintf(footer, "  <tr><td colspan=\"5\"><hr></td></tr>\n"
                    "</body>\n"
                    "</html>");
    strcat(contentStr, footer);
    return contentStr;
}

/**
 * This function calls fstat() on the file descriptor of the
 * specified stream.
 *
 * @param stream the stream
 * @param buf the stat struct
 * @return 0 if successful, -1 with errno set if error.
 */
int fileStat(FILE *stream, struct stat *buf) {
	int fd = fileno(stream);
	return fstat(fd, buf);
}


/**
 * Copy bytes from input stream to output stream
 * @param istream the input stream
 * @param ostream the output stream
 * @param nbytes the number of bytes to send
 * @param return 0 if successful or -1 if error
 */
int copyFileStreamBytes(FILE *istream, FILE *ostream, int nbytes) {
	char *buf[MAXBUF];
    while ((nbytes > 0) && !feof(istream)) {
    	int ntoread = (nbytes < MAXBUF) ? nbytes : MAXBUF;
        size_t nread = fread(buf, sizeof(char), ntoread, istream);
        if (nread > 0) {
			if (fwrite(buf, sizeof(char), nread, ostream) < nread) {
				return -1;
			}
			nbytes -= nread;
		}
    }
    return 0;
}

/**
 * Returns path component of the file path without trailing
 * path separator. If no path component, returns NULL.
 *
 * @param filePath the path and file
 * @param pathOfFile return buffer (must be large enough)
 * @return pointer to pathOfFile or NULL if no path
 */
char *getPath(const char *filePath, char *pathOfFile) {
	char *p = strrchr(filePath, '/');
	if (p == NULL) {
		return NULL;
	}
	strncpy(pathOfFile, filePath, p-filePath);
	pathOfFile[p-filePath] = '\0';  // must terminate;
	return pathOfFile;
}

/**
 * Returns name component of the file.
 *
 * @param filePath the path and file
 * @param name return buffer (must be large enough)
 * @return pointer to name or NULL if no path
 */
char *getName(const char *filePath, char *name) {
	char *p = strrchr(filePath, '/');
	strcpy(name, (p == NULL) ? filePath : p+1);
	return name;
}

/**
 * Returns extension of the file path without the '.'.
 * If no extension, returns NULL.
 *
 * @param filePath the path and file
 * @param extension return buffer (must be large enough)
 * @return pointer to extension or NULL if no path
 */
char *getExtension(const char *filePath, char *extension) {
	char *p = strrchr(filePath, '.');
	if (p == NULL) {
		return NULL;
	}
	strcpy(extension, p+1);
	return extension;
}

/**
 * Make a file path by combining a path and a file name.
 * If the file name begins with '/', return the name as an
 * absolute. Otherwise, concatenate the path and name,
 * ensuring there is a '/' separator between them.
 *
 * @param path the path component
 * @param name the file name component
 * @return the file path
 */
char *makeFilePath(const char *path, const char *name, char *filepath) {
	if (name[0] == '/') {
		strcpy(filepath, name);
	} else {
		strcpy(filepath, path);
		if (path[strlen(path)-1] != '/') {
			strcat(filepath, "/");
		}
		strcat(filepath, name);
	}
	return filepath;
}

/**
 * Make directories specified by path.
 *
 * @param path the path to create
 * @param mode mode if a directory is created
 * @return 0 if successful, -1 if error
 */
int mkdirs(const char *path, mode_t mode) {
	char buf[strlen(path)+1];
	const char *p = (*path == '/') ? path+1 : path;
	while (*p != '\0') {
		p = strchr(p, '/');  // find next path separator
		if (p == NULL) {     // last path element
			p = path + strlen(path);
		}
		strncpy(buf, path, p-path);
		buf[p-path] = '\0';  // must terminate
		if (mkdir(buf, mode) != 0) {
		    if (errno != EEXIST) {
                return -1;
            }
		}

		if (*p == '/') {  // advance past path separator
			p++;
		}
	}
	return 0;
}
