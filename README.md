# An HTTP Server
The HTTP server provided with this exercise is sometimes known as a "tiny http server" because it supports the most basic web server functionaity. In this asignment, you will have an opportunity to make several significant enhancements to this HTTP server and to learn more about networking, threads, synchronization, and this kind of stateless client-server architecture.

The repository contains
a "http_src" directory with the source code for the tiny HTTP server,
a "content" directory that contains HTML files and other content comprising the http server's content root directory,
a "thpool_src" directory with the source code and related files for a tread pool implementation,
several configuration files used by the HTTP server,
a "CMakeLists.txt" file for building the project under CLion.
Create a project with these files. Configure the run properties of both targets with the "Working directory" set to the project directory: $ProjectFileDir$.

## Adding DELETE, PUT, POST HTTP methods
This initial implemenetation supports two of the five primary methods that an HTTP server can perform. One of them is GET. This method requests the server to return a document from its hierarchically-organized file and directory system. The other method is HEAD. This command returns the same information about the document as GET, but HEAD does not return the response body.

You will implement three other HTTP methods, which are used by web applications to delete and store documents in the web server's file system. These operations are DELETE, PUT, and POST.

The DELETE HTTP method has the same format as GET. The named document is deleted from the server if it exists and the server returns a 200 "OK" response. If the file is not found, it returns a "404 "Not Found" response. For the purposes of this exercise, allow deleting a directory, but only if it is empty. The path must have a trailing path seperator to indicate that is is a directory. If the directory is not empty, return 405 "Method not Allowed." Use CURL to issue a DELETE request:
curl -i -X "DELETE" http://localhost:8080/myfile.html
Implement it in the C function do_delete(FILE *stream, const char *uri, Properties *requestProperties, Properties *responseHeaders) in new files "http_do_delete.c" and "http_do_delete.h" that is called by process_request().
The PUT HTTP method creates or replaces a document on the HTTP server at the location specified by the URL. If the file is created in the server, the server returns a 201 "Created" response and sets the "Location" response prop;erty to the server-relative URL path of created file. If the named file is overwritten, the server returns a 200 "OK" response. The content is sent as the request body, after the empty line that ends the request parameters. If the file cannot be created or opened, the server, returns 405: "Method Not Allowed". If the content length is not specified in the request header, the server returns 411: "Length Required" unless block transfer encoding is specified. For the purposes of this exercise, create any intermediate directories specified by the request path. To test this operation use CURL to issue a PUT request to upload a file with length specified in the request header:
curl -i --upload-file localfile.html -X PUT http://localhost:8080/serverfile.html
or using block transfer encoding:
curl -i -H 'Transfer-Encoding:chunked' --upload-file localfile.html -X PUT http://localhost:8080/serverfile.html
Implement it in a new C method do_put(FILE *stream, const char* uri, Properties *requestHeaders, Properties *responseHeaders) in new files "http_do_put.c" and "http_do_post.h" that is called by process_request().
The POST HTTP method is used to request the server to accept the content as a new subordinate of the resource identified by the request URL. Essentially, the URL is a collection URL. For example it is used to submit form data to a back-end form processor like an registration system. POST sends just the form data name/value pairs as the request body. The server creates a new document and returns "201 "Created" in response and sets the "Location" response property to the server-relative URL path of created file. If the server cannot store the content, it returns 405: "Method Not Allowed". If the content length is not specified in the request header, the server sends back 411: "Length Required" unless block transfer encoding is specified (see example under PUT).
For this exercise, the server creates a directory specified by the URL path if it does not already exist, and creates a new document with the content of the request body in this directory. Create any intermediate directories specified by the request path. The name of the file is determined by the server. One way is to use mkstemps() to create a file with a unique name based on a template string. Give the file a suffix based on the HTTP content type: ".mime" for "multipart/form-data", ".urlencoded" for "application/xwww-form-urlencoded", and ".txt" for "text/plain". Use ".bin" for any other content type.
The HTML form encoding type application/x-www-form-urlencoded sends the field names and values in the request body as a single line, with the name separated from the value with an equal sign ("="), and name/value pairs separated by an ampersand (&) or semicolon (;). Values are URL encoded (spaces are converted to "+" symbols, and special characters are converted to ASCII HEX values) in case they contiain any special characters like equal sign or amersand. The request "Content-Type" is application/x-www-form-urlencoded. For example, the fields values
Name: Gareth Wylie
Age: 24
Formula: a + b
are encoded as
Name=Gareth+Wylie&Age=24&Formula=a+%2B+b
Use CURL to issue a POST request that URL-encodes its data values like an HTML form (on one line):
curl -i --data-urlencode 'Name=Gareth Wylie' --data-urlencode 'Age=24' --data-urlencode 'Formula=a + b' -X POST http://localhost:8080/registration
Add "-H 'Transfer-Encoding:chunked'" for chunked transfer. The "form-post.html" file in the "forms" subdirectory of the "content" directory that uses the default application/x-www-form-urlencoded encoding type.
The HTML form encoding type multipart/form-data sends the field names and values in the request body as a multi- part form with each field name and value in its own part, separated from other parts by special delimiter lines. This type of form is used to send non-text valued field values, such as documents, images, and other non-text data. The parts include information that enable the fields and their values to be extracted and processed by a form-processing back end. This is how an image of a user would be sent as part of a registration form for a website.

### Use CURL to issue a POST request that includes a "Profile-pic" field whose value is a PNG file of the user:
curl -i --form "Name=Gareth Wylie" --form "Age=24" --form "Profile-pic=@/Users/gareth/gareth.png" -X POST http://localhost:8080/registration
Add "-H 'Transfer-Encoding:chunked'" for chunked transfer. The "form-post-multipart.html" file in the "forms" subdirectory of the "content" directory that uses the multipart/form-data encoding type and allows a profile image to be uploaded.
The HTML form encoding type text/plain sends the field names and values in the request body, and is similar to application/x-www-form-urlencoded, except that the fields values are not encoded. For example, the fields values
Name: Gareth Wylie
Age: 24
Formula: a + b
appear as
Name=Gareth Wylie&Age=24&Formula=a + b
Use CURL to issue a POST request that formats its data values like an HTML form (on one line):
curl -i --data 'Name=Gareth Wylie' --data 'Age=24' --data 'Formula=a + b' -H 'Content-Type:text/plain' -X POST http://localhost:8080/registration
Add "-H 'Transfer-Encoding:chunked'" for chunked transfer. The "form-post-textplain.html" file in the "forms" subdirectory of the "content" directory uses the "text/plain" encoding type.
Implement the POST processing code in the C method do_post(FILE *stream, const char* uri, Properties *requestHeaders, Properties *responseHeaders) in new files "http_do_post.c" and "http_do_post.h" that is called by process_request().

### Listing directories
Add code to list the contents of a directory as a formatted HTML page. This method is an extension of GET and the server returns a 200 "OK" response. The content is sent as the body of the response, after the empty line that ends the request parameters. The path must have a trailing path seperator to indicate that is is directory. If the directory does not exist, return a "404 "Not Found" response. Use CURL to issue a GET request, in this case to list the server's root content directory:
curl -i http://localhost:8080/
Add code to do_get_or_head() that determines whether the specified file is a directory (path ends with '/' and file type is directory). If so, it generates the directory listing and returns it as the body of the response. A recommendation is to write a function that generates the directory listing as a temporary file and returns a FILE*, and then have do_get() return the contents of that file.

For each entry, the directory listing should include the name of the entry, the last modified date of the form "2019-04-13 19:03", the size of the file or directory, and optionally any descriptive information. The entries should include a link to the content or for a file, or to the subdirectory if a directory. Subdirectories should include a "Parent Directory" entry to return to the previous level.

Here are sample listings for a root directory and a subdirectory that correspond closely with the default output for the Apache web server.

## Improved media type processing
The code in "media_types.c" detects the type of a file and returns the media type. A media type (formerly known as MIME type) is a two-part identifier for file formats and format contents transmitted on the Internet. The current implementaiton uses a small set of hard-wired tests. Production HTTP servers use more flexible mechanisms. One of the earliest was to use a file named "mime.types" that has lines with media types and zero or more associated file extensions. The HTTP server reads this file at startup. A copy of a typical "mime.types" file is included with your assignment. Replace the hard-wired mechanism with one that uses the information in this file.

A good implementation strategy is to add a readMediaTypes() method to "media_util.c" and "media_util.h" that accepts the name of the "mime.types" file and builds a static global Properties instance with file extensions as keys and media types as values. If there is no Properties instance, this method creates one. If there is a Properties instance, this method replaces it with one created from this file. The method returns the number of entries in Properties. Reimplement the getMediaType() method to use this Properties instance to get the media type. If the file extension is not registered, return the default media type.

Add a new "ContentTypes" property to the HTTP configuration file that specifies the "mime.types" file. A relative path is relative to the server root directory. Add code to the process_config() method in http_server.c that reads the "ContentTypes" property and calls readMediaTypes() with the file name to initialize the media types. If the property is not defined in the configuration file, use "mime.types" as the default value. If no entries are read, process_config() should return false.

More advanced HTTP servers use a variety of techniques including the file extension, as well as examining or "sniffing" the format of the document. An example of a framework for detecting media type using a variety of techniques is Apache Tika library. You may want to explore this library to see how it works.

## Threading
This server only processes a single request at a time. A limited number of other requests are queued by the socket layer. Your next task will be to add threading to the HTTP server. For each client request, you will create a separate thread to handle that request. The main() function calls process_request() with the client socket to process the request. This is the function that you will need to thread.

Rather than creating raw threads, you will instead use a framework called a thread pool to queue tasks waiting for a thread from a fixed pool to become available. When one does, the thread pool selects a task waiting in the queue and assigns the free threed to run the task. When the task completes, the thread is reurned to the pool, ready to run another task.

For this exercise use a C thread pool implementation located in the "thpool_src" project directory. This is a modified version of the original written by Johan Hanssen Seferidis, which is available at https://github.com/Pithikos/C-Thread-Pool. This modified version has been updated to support CygWin. The repository contains a .c and.h file that implements the thread pool, and an example program to demonstrate its use. A "thpool_example" target in the CLion "CMakeLists.txt" file builds and runs a demo of the thread pool code. You will need to add "thpool_src/thpool.c" to your "http_server" target to use it in the http server. Choose a reasonable number of threads to test out your threaded code request processor.

## Testing
Use CURL to run basic tests and ensure the responses header properties and the response body match your expectations. Next use a brower to view and interact with the HTTP server. Some browsers provide ways to send HEAD, PUT, and DELETE requests. All browsers support sending GET and POST (form) requests. Also be sure to test listing directories. Ensure that navigation and opening files in the listing work correctly and that there is a parent directory target only for subdirectory listings.
