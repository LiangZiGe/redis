epoll处理
===

###基础
简单的epoll原理就不粘贴了，debug请按照如下步骤操作：


- 修改 CMakeLists.txt
```markdown
    cmake_minimum_required(VERSION 3.3)
    project(redis_code)
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    
    set(SOURCE_FILES t/TestServer.c)
    add_executable(redis_code ${SOURCE_FILES})
```
- 运行[TestServer.c](https://github.com/LiangZiGe/redis/tree/master/epoll/TestServer.c)
- 再次修改CMakeLists.txt运行[TestClient.c](https://github.com/LiangZiGe/redis/tree/master/epoll/TestClient.c)

###redis处理流程