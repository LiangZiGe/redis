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
- 在redis.c/initServer()初始化事件槽
    ```C
    server.el = aeCreateEventLoop(server.maxclients+REDIS_EVENTLOOP_FDSET_INCR);
    
    aeEventLoop *aeCreateEventLoop(int setsize) {
        ...
            // 初始化了全部的事件槽，使用指针数组
            for (i = 0; i < setsize; i++)
                eventLoop->events[i].mask = AE_NONE;
        ...
    }
    ```
- 绑定事件，比如创建一个新的客户端 networking.c/*createClient()，其他的请参看`redis-3.0-annotated`源码
    ```C
    if (aeCreateFileEvent(server.el,fd,AE_READABLE,
                readQueryFromClient, c) == AE_ERR)
                
     // ae.c/aeCreateFileEvent()
    int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
            aeFileProc *proc, void *clientData)
    {
        if (fd >= eventLoop->setsize) { www
            errno = ERANGE;
            return AE_ERR;
        }
    
        if (fd >= eventLoop->setsize) return AE_ERR;
    
        // 取出文件事件结构，直接从数组中取多么的巧妙
        aeFileEvent *fe = &eventLoop->events[fd];
    
        // 监听指定 fd 的指定事件
        if (aeApiAddEvent(eventLoop, fd, mask) == -1)
            return AE_ERR;
    
        // 设置文件事件类型，以及事件的处理器
        fe->mask |= mask;
        if (mask & AE_READABLE) fe->rfileProc = proc;
        if (mask & AE_WRITABLE) fe->wfileProc = proc;
    
        // 私有数据
        fe->clientData = clientData;
    
        // 如果有需要，更新事件处理器的最大 fd
        if (fd > eventLoop->maxfd)
            eventLoop->maxfd = fd;
    
        return AE_OK;
    }
    ```
- 事件处理，服务启动时redis.c/main()触发
    ```C
    aeMain(server.el);
    
    /*
     * ae.c/ 事件处理器的主循环
     */
    void aeMain(aeEventLoop *eventLoop) {
    
        eventLoop->stop = 0;
    
        while (!eventLoop->stop) {
    
            // 如果有需要在事件处理前执行的函数，那么运行它
            if (eventLoop->beforesleep != NULL)
                eventLoop->beforesleep(eventLoop);
    
            // 开始处理事件
            aeProcessEvents(eventLoop, AE_ALL_EVENTS);
        }
    }
    
    int aeProcessEvents(aeEventLoop *eventLoop, int flags)
    {
  
         // 执行读事件，比如执行新建客户端的 rfileProc 映射到 readQueryFromClient 方法
         if (fe->mask & mask & AE_READABLE) {
            rfired = 1;
            fe->rfileProc(eventLoop,fd,fe->clientData,mask);
         }
          
    }
    ```