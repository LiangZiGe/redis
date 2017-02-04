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
- ��redis.c/initServer()��ʼ���¼���
    ```C
    server.el = aeCreateEventLoop(server.maxclients+REDIS_EVENTLOOP_FDSET_INCR);
    
    aeEventLoop *aeCreateEventLoop(int setsize) {
        ...
            // ��ʼ����ȫ�����¼��ۣ�ʹ��ָ������
            for (i = 0; i < setsize; i++)
                eventLoop->events[i].mask = AE_NONE;
        ...
    }
    ```
- ���¼������紴��һ���µĿͻ��� networking.c/*createClient()����������ο�`redis-3.0-annotated`Դ��
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
    
        // ȡ���ļ��¼��ṹ��ֱ�Ӵ�������ȡ��ô������
        aeFileEvent *fe = &eventLoop->events[fd];
    
        // ����ָ�� fd ��ָ���¼�
        if (aeApiAddEvent(eventLoop, fd, mask) == -1)
            return AE_ERR;
    
        // �����ļ��¼����ͣ��Լ��¼��Ĵ�����
        fe->mask |= mask;
        if (mask & AE_READABLE) fe->rfileProc = proc;
        if (mask & AE_WRITABLE) fe->wfileProc = proc;
    
        // ˽������
        fe->clientData = clientData;
    
        // �������Ҫ�������¼������������ fd
        if (fd > eventLoop->maxfd)
            eventLoop->maxfd = fd;
    
        return AE_OK;
    }
    ```
- �¼�������������ʱredis.c/main()����
    ```C
    aeMain(server.el);
    
    /*
     * ae.c/ �¼�����������ѭ��
     */
    void aeMain(aeEventLoop *eventLoop) {
    
        eventLoop->stop = 0;
    
        while (!eventLoop->stop) {
    
            // �������Ҫ���¼�����ǰִ�еĺ�������ô������
            if (eventLoop->beforesleep != NULL)
                eventLoop->beforesleep(eventLoop);
    
            // ��ʼ�����¼�
            aeProcessEvents(eventLoop, AE_ALL_EVENTS);
        }
    }
    
    int aeProcessEvents(aeEventLoop *eventLoop, int flags)
    {
  
         // ִ�ж��¼�������ִ���½��ͻ��˵� rfileProc ӳ�䵽 readQueryFromClient ����
         if (fe->mask & mask & AE_READABLE) {
            rfired = 1;
            fe->rfileProc(eventLoop,fd,fe->clientData,mask);
         }
          
    }
    ```