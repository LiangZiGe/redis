dict
===

��debug����������֪ʶ����������һͷ��ˮ��

- debug �޸�CMakeLists.txt������[main����](/t/commandMain.c)��

        cmake_minimum_required(VERSION 3.3)
        project(redis_code)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        set(SOURCE_FILES t/commandMain.c "dict.c" dict.h sds.h redisassert.h "sds.c" zmalloc.h "zmalloc.c")
        add_executable(redis_code ${SOURCE_FILES})
        
- ��debug�ܶ�Σ���Ϥÿ�����������á�

    
    