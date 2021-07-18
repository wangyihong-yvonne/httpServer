cmake_minimum_required(VERSION 3.17)
project(assignment_5 C)

set(CMAKE_C_STANDARD 11)

# project include directories
include_directories(http_src)
include_directories(thpool_src)

# project source directories
aux_source_directory(http_src http_src)
aux_source_directory(thpool_src thpool_src)

# build http server
add_executable(http_server ${http_src}  http_src/http_do_post.c http_src/http_do_post.h http_src/http_do_put.c http_src/http_do_put.h thpool_src/thpool.c)
#add_executable(http_server ${http_src}  http_src/http_do_post.c http_src/http_do_post.h http_src/http_do_put.c http_src/http_do_put.h)


# build thread pool example
add_executable(thpool_example ${thpool_src})