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
- debug����ƻ���һͷ��ˮ����������ô����[dict.h](/dict.h)���������������濴��[�ֵ�](http://origin.redisbook.com/internal-datastruct/dict.html)��д���ر�ã�Ȼ������ݽṹ������ϵ�Լ��ٻ�һ�顣

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
    - [dictGenCaseHashFunction](/dict.c#dictGenCaseHashFunction)��������������buf������������ȡ��ӦСд��ĸ��asciiֵ��Ȼ���� ((hash << 5) + hash) = hash * 2^5 + hash = hash * 33 ��ӡ����[hash.html](http://www.cse.yorku.ca/~oz/hash.html)��
    
    - [_dictKeyIndex](/dict.c#_dictKeyIndex)������
    
        - `idx = h & d->ht[table].sizemask;`��`sizemask=ht[table].size - 1`��Ȼ��`h & d->ht[table].sizemask`���ǵõ�һ��С�ڵ���`ht[table].sizemask`��ֵ���Ƚ�����˼Ҳ�ܻ�����һ�㲻��˼����д�����ֻ���ôд��`if (h < ht[table].sizemask) ...`������һ������˼�Ļ��⡣
        
        - ��֤`ids`����ο��ĵ�[����¼�ֵ��ʱ������ײ����](http://origin.redisbook.com/internal-datastruct/dict.html#id11   )
            
            ```C
            he = d->ht[table].table[idx];
             while(he) {
                  if (dictCompareKeys(d, key, he->key))
                      return -1;
                  he = he->next;
             }
             // ���ݼ���õ���idx���ڶ�Ӧbucket�е�`dictEntry`�����Ƿ��Ѿ�����he��
             // ��֤ͨ����͵õ�����Ч��hashֵ������hashֵ��ײ�ļ����ж���أ�ֱ�Ӿ�����ѭ��������һ������˼�Ļ��⣬�ǽ������о��ɡ�    
             ```
             
   - dictht��bucket��hash��ײ����
        
       ��linux������ʹ��clion����[main](/t/commandMain.c)������
       
            Hash table stats:
             table size: 128
             number of elements: 85
             different slots: 52
             max chain length: 5
             avg chain length (counted): 1.63
             avg chain length (computed): 1.63
             Chain length distribution:
               0: 76 (59.38%)
               1: 32 (25.00%)
               2: 12 (9.38%)
               3: 5 (3.91%)
               4: 1 (0.78%)
               5: 2 (1.56%)
            -- Rehashing into ht[1]:
            Hash table stats:
             table size: 256
             number of elements: 75
             different slots: 61
             max chain length: 3
             avg chain length (counted): 1.23
             avg chain length (computed): 1.23
             Chain length distribution:
               0: 195 (76.17%)
               1: 49 (19.14%)
               2: 10 (3.91%)
               3: 2 (0.78%)
                   
        ���������redis����˳�ʼ�����������ֵ�ʱmax chain length = 5����ײ�ļ��ʻ��ǱȽϴ�ġ���Ȼ��������������������������ܻ����ۣ�����rehash�����ˡ�
        `_dictExpandIfNeeded`�У�
        ```C
            // bucket��ʹ�� >= ��ʼ����С && (����rehash��ʾ || ʹ��/��ʼ����С > ǿ��rehash����)
            if (d->ht[0].used >= d->ht[0].size &&
                    (dict_can_resize ||
                     d->ht[0].used/d->ht[0].size > dict_force_resize_ratio))
                {
                    // �¹�ϣ��Ĵ�С������Ŀǰ��ʹ�ýڵ���������
                    // T = O(N)
                    return dictExpand(d, d->ht[0].used*2);
                }
        ```
   - rehash
        

      
      
      
      
        
         