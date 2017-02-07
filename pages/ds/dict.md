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
- debug����ƻ���һͷ��ˮ����������ô����[dict.h](/dict.h)���������������濴��[�ֵ�](http://origin.redisbook.com/internal-datastruct/dict.html)��Ȼ������ݽṹ������ϵ�Լ��ٻ�һ�顣

    ![structrue](/pics/dict-struct.png)

    `typedef struct dictType`������ݽṹ��Ƶ�̫���ˣ��ֵ��ںܶೡ���¶���ʹ�ã������������������ʹ��ʱָ������ʵ�ֺ��������������Ͼ���һ�硣�����ڳ�ʼ����������ʱʹ����`commandTableDictType`
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
            // ��������꣬�ڲ����ֵ�ĺ�����ʹ�ã����������ʵ�ֺ���������commandTableDictType
            #define dictFreeVal(d, entry) \
                if ((d)->type->valDestructor) \
                    (d)->type->valDestructor((d)->privdata, (entry)->v.val)
            
            // ���ø����ֵ�ڵ��ֵ
            #define dictSetVal(d, entry, _val_) do { \
                if ((d)->type->valDup) \
                    entry->v.val = (d)->type->valDup((d)->privdata, _val_); \
                else \
                    entry->v.val = (_val_); \
            } while(0)
            
            // ...
    ```
    
    ������������ͼ��
    
    ![dict-find](/pics/dict-find.jpg)
- �ټ���debug�����ʱ��ͷ��������Щ����������ʶ�����ŷ���ϸ�ڡ�
    - [dictGenCaseHashFunction](/dict.c#dictGenCaseHashFunction)���������[hash.html](http://www.cse.yorku.ca/~oz/hash.html)��
    
     
     
     
    