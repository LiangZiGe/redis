Redis 3.0 源码研究
============================
本项目参考：[redisbook](http://origin.redisbook.com)、[dumbee](http://dumbee.net/archives/114')、[redis-3.0-annotated](https://github.com/huangz1990/redis-3.0-annotated)，debug请使用[CLion](https://www.jetbrains.com/clion/)。

#命令处理

##流程
###客户端
- 对输入命令的处理
    - redis-cli.c/repl
        - redis-cli.c/cliSendCommand
            - hiredis.c/redisAppendCommandArgv
                - hiredis.c/redisFormatCommandArgv
                ```markdown
                命令格式化
                ```
                - hiredis.c/__redisAppendCommand    
                ```markdown
                写入全局的 redisContext 的 write 缓冲区 obuf 中
                ```
                                
- 向服务器发送命令
    - redis-cli.c/cliSendCommand
        - redis-cli.c/cliReadReply
            - hiredis.c/redisGetReply
                - hiredis.c/redisBufferWrite

- 获取服务器回复
    - redis-cli.c/cliSendCommand
        - redis-cli.c/cliReadReply
            - hiredis.c/redisGetReply
                - hiredis.c/redisBufferRead
                - hiredis.c/redisGetReplyFromReader
            - redis-cli.c/cliFormatReplyRaw
            - fwrite
            
 ###服务端
 - redis.c/initServer
    - redis.c/listenToPort
        - anet.c/anetTcpServer
            - anet.c/_anetTcpServer
            ```C
            // 建立socket套接字
            s = socket(p->ai_family,p->ai_socktype,p->ai_protocol))
            ```
    - redis.c/aeCreateFileEvent(,acceptTcpHandler,)
        - networking.c/acceptTcpHandler
            - anet.c/anetTcpAccept
            - networking.c/acceptCommonHandler
                - networking.c/createClient
                    - networking.c/readQueryFromClient
                    - ae.c/aeCreateFileEvent(server.el,fd,AE_READABLE,readQueryFromClient, c)
                    ```markdown
                    绑定readQueryFromClient到事件loop,redis的监听socket收到数据时将调用该函数进行处理
                    ```
                    
 ###命令同步
 - redis.c/call
    - redis.c/propagate
        - replication.c/replicationFeedSlaves
            - networking.c/addReply
                - networking.c/prepareClientToWrite
                    - networking.c/sendReplyToClient
                    ```C
                    void call(redisClient *c, int flags) {
                        // 同步监控客户端
                        // 执行函数    c->cmd->proc(c); redisCommandProc *proc 指针函数映射各命令实现
                        // 慢日志记录
                        // 命令传播
                        // 传播 alsoPropagate() 产生的命令，3.0版本为什么没有调用alsoPropagate这个方法呢？
                    }
                    
                    void replicationFeedSlaves(list *slaves, int dictid, robj **argv, int argc) {
                        /** backlog作用：backlog是一个slave在一段时间内断开连接时记录salve数据的缓冲，所以一个slave在重新连接时，不必要全量的同步，而是一个增量同步就足够了，将在断开连接的这段时间内slave丢失的部分数据传送给它。同步的backlog越大，slave能够进行增量同步并且允许断开连接的时间就越长。backlog只分配一次并且至少需要一个slave连接repl-backlog-size 1mb 当master在一段时间内不再与任何slave连接，backlog将会释放。以下选项配置了从最后一个 slave断开开始计时多少秒后，backlog缓冲将会释放。  0表示永不释放backlog  repl-backlog-ttl 3600*/

                        // 没有backlog ，则说明没有从服务器直接返回。
                        // 构建协议内容  
                        // 将协议内容备份到 backlog
                        // 将内容发送给各个从服务器
                        
                        /**此方法难点如下：
                            for (j = 0; j < argc; j++) {
                                        long objlen = stringObjectLen(argv[j]);
                            
                                        /* We need to feed the buffer with the object as a bulk reply
                                         * not just as a plain string, so create the $..CRLF payload len 
                                         * ad add the final CRLF */
                                        aux[0] = '$';
                                        len = ll2string(aux+1,sizeof(aux)-1,objlen);
                                        aux[len+1] = '\r';
                                        aux[len+2] = '\n';
                                        // 追加格式符号 $\r\n 到backlog
                                        feedReplicationBacklog(aux,len+3);
                                        // 追加命令或者参数到backlog，eg set key value中的三个变量之一
                                        feedReplicationBacklogWithObject(argv[j]);
                                        // 取aux[len+1] = '\r' 追加达到换行的目的
                                        feedReplicationBacklog(aux+len+1,2);
                                    }
                        */
                    }
                    
                    void addReply(redisClient *c, robj *obj) {
                        // 为客户端安装写处理器到事件循环
                        if (prepareClientToWrite(c) != REDIS_OK) return;
                        if(sds类型)
                            // 添加到回复缓存c->buf，如果回复缓存c->buf空间不足添加到回复链表c->reply
                        else
                            // 处理integer编码数据
                    }
                    
                    int prepareClientToWrite(redisClient *c) {
                        // 核心处理，sendReplyToClient写处理器到事件循环
                        if (c->bufpos == 0 && listLength(c->reply) == 0 &&
                            (c->replstate == REDIS_REPL_NONE ||
                             c->replstate == REDIS_REPL_ONLINE) &&
                            aeCreateFileEvent(server.el, c->fd, AE_WRITABLE,
                            sendReplyToClient, c) == AE_ERR) return REDIS_ERR;
                    }
                    
                    void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask){
                        // 回复缓存或者回复列表有值 写出去
                        while(c->bufpos > 0 || listLength(c->reply)) {
                            // 因为在 addReply方法中对 c->buf 写入时空间不够，就复制到 c->reply 链表中了，所以有如下处理。
                            if (c->bufpos > 0) {
                                // 处理回复缓存
                            }else{
                                // 处理回复列表
                            }
                        }
                        // 写检查，系统中断重置nwritten，其他原因，打印错误日志，释放从连接
                        // 释放写fd
                    }
                     ```
#