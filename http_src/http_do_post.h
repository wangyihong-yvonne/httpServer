/*
* http_do_post.c
        *
        * Implement HTTP POST methods.
*
*  @since 2021-04-20
*/

#ifndef HTTP_DO_POST_H
#define HTTP_DO_POST_H

#include <stdio.h>
#include "properties.h"

void do_post(FILE *stream, const char *uri, Properties *requestHeaders, Properties *responseHeaders);


#endif /* HTTP_DO_POST_H_ */
