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
- debug完估计还是一头雾水，哈哈。那么来从[dict.h](/dict.h)来分析，首先认真看完[字典](http://origin.redisbook.com/internal-datastruct/dict.html)，然后把数据结构关联关系自己再画一遍。

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
    - [dictGenCaseHashFunction](/dict.c#dictGenCaseHashFunction)函数，详见[hash.html](http://www.cse.yorku.ca/~oz/hash.html)。
    
     
     
     
    