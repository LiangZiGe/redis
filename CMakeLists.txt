cmake_minimum_required(VERSION 3.3)
project(redis_code)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES ziplist.c "zmalloc.c" "zmalloc.h" "ziplist.h" "ziplist.c" "util.c" "util.h" "sds.c" "sds.h" "redisassert.h" "endianconv.h" "endianconv.c" "fmacros.h"
        "debug.c" "adlist.c" "adlist.h")
add_executable(redis_code ${SOURCE_FILES})