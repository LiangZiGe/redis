cmake_minimum_required(VERSION 3.3)
project(redis_code)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES t/commandMain.c dict.h sds.h redisassert.h "sds.c" "dict.c")
add_executable(redis_code ${SOURCE_FILES})