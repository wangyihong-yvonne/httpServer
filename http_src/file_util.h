/*
 * file_util.h
 *
 * Utilities functions for working with C FILE streams.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#ifndef FILE_UTIL_H_
#define FILE_UTIL_H_

#include <stdio.h>
#include <sys/stat.h>

/**
 * This function creates a temporary stream for this string.
 * When the FILE is closed, it will be automatically removed.
 *
 * @param contentStr the content of the file
 * @return the FILE for this string
 */
FILE *tmpStringFile(const char *contentStr);

/**
 * This function calls fstat() on the file descriptor of the
 * specified stream.
 *
 * @param stream the stream
 * @param buf the stat struct
 * @return 0 if successful, -1 with errno set if error.
 */
int fileStat(FILE *stream, struct stat *buf);

/**
 * Copy bytes from input stream to output stream
 * @param istream the input stream
 * @param ostream the output stream
 * @param nbytes the number of bytes to send
 * @param return 0 if successful
 */
int copyFileStreamBytes(FILE *istream, FILE *ostream, int nbytes);

/**
 * Returns path component of the file path without trailing
 * path separator. If no path component, returns NULL.
 *
 * @param filePath the path and file
 * @param pathOfFile return buffer (must be large enough)
 * @return pointer to pathOfFile or NULL if no path
 */
char *getPath(const char *filePath, char *pathOfFile);

/**
 * Returns name component of the file.
 *
 * @param filePath the path and file
 * @param name return buffer (must be large enough)
 * @return pointer to name or NULL if no path
 */
char *getName(const char *filePath, char *name);

/**
 * Returns extension of the file path without the '.'.
 * If no extension, returns NULL.
 *
 * @param filePath the path and file
 * @param extension return buffer (must be large enough)
 * @return pointer to extension or NULL if no path
 */
char *getExtension(const char *filePath, char *extension);

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
char *makeFilePath(const char *path, const char *name, char *filepath);

/**
 * Make directories specified by path.
 *
 * @param path the path to create
 * @param mode mode if a directory is created
 * @return 0 if successful, -1 if error
 */
int mkdirs(const char *path, mode_t mode);

char *generateList(const char *filePath);

#endif /* FILE_UTIL_H_ */
