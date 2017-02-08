dict
===

先debug再深入理论知识，否则又是一头雾水。

- debug 修改CMakeLists.txt，运行[main方法](/t/commandMain.c)。

        cmake_minimum_required(VERSION 3.3)
        project(redis_code)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        set(SOURCE_FILES t/commandMain.c "dict.c" dict.h sds.h redisassert.h "sds.c" zmalloc.h "zmalloc.c")
        add_executable(redis_code ${SOURCE_FILES})
        
- 先debug跑多次，熟悉每个方法的作用。
- debug完估计还是一头雾水，哈哈。那么来从[dict.h](/dict.h)来分析，首先认真看完[字典](http://origin.redisbook.com/internal-datastruct/dict.html)，写的特别好，然后把数据结构关联关系自己再画一遍。

    ![structrue](/pics/dict-struct.png)

    `typedef struct dictType`这个数据结构设计的太棒了，字典在很多场景下都有使用，这里给出了声明，在使用时指定具体实现函数，否则代码耦合就是一坨。比如在初始化命令数组时使用了`commandTableDictType`
    ```C
            dictType commandTableDictType = {
                dictSdsCaseHash,           /* hash function */
                NULL,                      /* key dup */
                NULL,                      /* val dup */
                dictSdsKeyCaseCompare,     /* key compare */
                dictSdsDestructor,         /* key destructor */
                NULL                       /* val destructor */
            };
            
            ///////////////////////////////
            
            server.commands = dictCreate(&commandTableDictType,NULL);
            server.orig_commands = dictCreate(&commandTableDictType,NULL);
            
            //////////////////////////////////////////////
            // 定义基础宏，在操作字典的函数中使用，触发具体的实现函数，比如commandTableDictType
            #define dictFreeVal(d, entry) \
                if ((d)->type->valDestructor) \
                    (d)->type->valDestructor((d)->privdata, (entry)->v.val)
            
            // 设置给定字典节点的值
            #define dictSetVal(d, entry, _val_) do { \
                if ((d)->type->valDup) \
                    entry->v.val = (d)->type->valDup((d)->privdata, _val_); \
                else \
                    entry->v.val = (_val_); \
            } while(0)
            
            // ...
    ```
    
    其他场景如下图：
    
    ![dict-find](/pics/dict-find.jpg)
- 再继续debug，这个时候头脑里有了些许清晰的认识，接着分析细节。
    - [dictGenCaseHashFunction](/dict.c#dictGenCaseHashFunction)函数：遍历整个buf，从左到右依次取对应小写字母的ascii值，然后与 ((hash << 5) + hash) = hash * 2^5 + hash = hash * 33 相加。详见[hash.html](http://www.cse.yorku.ca/~oz/hash.html)。
    - [_dictKeyIndex](/dict.c#_dictKeyIndex)函数：
        - `idx = h & d->ht[table].sizemask;`，`sizemask=ht[table].size - 1`，然后`h & d->ht[table].sizemask`总是得到一个小于等于`ht[table].sizemask`的值，比较有意思也很基础。一般不加思索的写代码又会怎么写？`if (h < ht[table].sizemask) ...`，这是一个有意思的话题。
        - 验证`ids`，请参看文档[添加新键值对时发生碰撞处理](http://origin.redisbook.com/internal-datastruct/dict.html#id11   )
            ```C
            he = d->ht[table].table[idx];
             while(he) {
                  if (dictCompareKeys(d, key, he->key))
                      return -1;
                  he = he->next;
             }
             // 根据计算得到的idx后，再对应的`dictEntry`链表是否已经包含he，
             // 验证通过后就得到了有效的hash值。这种hash值碰撞的几率有多大呢？直接决定着循环次数，一个有意思的话题，那接下来研究吧。    
             ```
   - dictht上bucket的hash碰撞分析
      >todo
         