epoll����
===

###����
�򵥵�epollԭ��Ͳ�ճ���ˣ�debug�밴�����²��������


- �޸� CMakeLists.txt
```markdown
    cmake_minimum_required(VERSION 3.3)
    project(redis_code)
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    
    set(SOURCE_FILES t/TestServer.c)
    add_executable(redis_code ${SOURCE_FILES})
```
- ����[TestServer.c](https://github.com/LiangZiGe/redis/tree/master/epoll/TestServer.c)
- �ٴ��޸�CMakeLists.txt����[TestClient.c](https://github.com/LiangZiGe/redis/tree/master/epoll/TestClient.c)

###redis��������