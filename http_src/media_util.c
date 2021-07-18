/*
 * media_util.c
 *
 * Functions for processing media types.
 *
 *  @since 2019-04-10
 *  @author: Philip Gust
 */

#include "media_util.h"

#include <string.h>
#include <ctype.h>
#include "string_util.h"
#include "http_server.h"
#include "file_util.h"

/** default media type */
static const char *DEFAULT_MEDIA_TYPE = "application/octet-stream";
static Properties *mediaTypeProperties = NULL;  //// need initialization or not?
// readMediaTypes()
// external property


/**
 * Return a media type for a given filename.
 *
 * @param filename the name of the file
 * @param mediaType output buffer for mime type
 * @return pointer to media type string
 */
char *getMediaType(const char *filename, char *mediaType)  //// TODO: need to be reimplement
{

	// special-case directory based on trailing '/'
	if (filename[strlen(filename)-1] == '/') {
		strcpy(mediaType, "text/directory");
		return mediaType;
	}

	// get file extension
    char ext[MAXBUF];
    if (getExtension(filename, ext) == NULL) {
    	// default if no extension
    	strcpy(mediaType, DEFAULT_MEDIA_TYPE);
    	return mediaType;
    }

    // lower-case extension
    strapply(ext, ext, tolower);

    // find mediaType
    if (mediaTypeProperties == NULL || findProperty(mediaTypeProperties, 0, ext, mediaType) == SIZE_MAX) {
        strcpy(mediaType, DEFAULT_MEDIA_TYPE);
    }
    return mediaType;
}

/**
 * Return the number of entries in Properties.
 *
 * @param filename the name of the media file
 * @return the number of entries in Properties.
 */

int readMediaType(char* fileName) {
     // If there is no Properties instance, this method creates one; else, replace it
     if (mediaTypeProperties != NULL) {
         deleteProperties(mediaTypeProperties);
     }
    mediaTypeProperties = newProperties();
     // open the file
    FILE *buffStream = fopen(fileName, "r");
    if (buffStream == NULL) {
        return 0;
    }

    int prop_ct = 0;  // the number of entries in Properties
    char buf[MAXBUF];

    while (fgets(buf, MAXBUF, buffStream) != NULL) {
        if (buf[0] == '#') {
            continue;
        }

        char* rest = buf;
        //get content type for value
        char *value = strtok_r(rest, " \t\n", &rest);

        if (value != NULL) {
            char type[MAX_PROP_VAL];
            strcpy(type, value);
            // get file extensions for key
            char *key;
            while ((key = strtok_r(rest, " \t\n", &rest)) != NULL) {  //// strtok_r-thread safe
                if (key[0] != '\0') {
                    char ext[MAXBUF];
                    strcpy(ext, key);
                    putProperty(mediaTypeProperties, key, type);
                    prop_ct++;
                }
            }
        } // else, continue
    }

    fclose(buffStream);
    return prop_ct;
}
