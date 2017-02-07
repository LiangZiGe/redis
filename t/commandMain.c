//
// Created by zhangziliang on 2017/2/4.
//

// 命令标志
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include "../dict.h"
#include "../sds.h"

#define REDIS_CMD_WRITE 1                   /* "w" flag */
#define REDIS_CMD_READONLY 2                /* "r" flag */
#define REDIS_CMD_DENYOOM 4                 /* "m" flag */
#define REDIS_CMD_NOT_USED_1 8              /* no longer used flag */
#define REDIS_CMD_ADMIN 16                  /* "a" flag */
#define REDIS_CMD_PUBSUB 32                 /* "p" flag */
#define REDIS_CMD_NOSCRIPT  64              /* "s" flag */
#define REDIS_CMD_RANDOM 128                /* "R" flag */
#define REDIS_CMD_SORT_FOR_SCRIPT 256       /* "S" flag */
#define REDIS_CMD_LOADING 512               /* "l" flag */
#define REDIS_CMD_STALE 1024                /* "t" flag */
#define REDIS_CMD_SKIP_MONITOR 2048         /* "M" flag */
#define REDIS_CMD_ASKING 4096               /* "k" flag */

#define NULL ((void *)0)
#define REDIS_LRU_BITS 24

typedef struct redisClient {

    // 记录被客户端执行的命令
    struct redisCommand *cmd, *lastcmd;

} redisClient;

typedef struct redisObject {

    // 类型
    unsigned type:4;

    // 编码
    unsigned encoding:4;

    // 对象最后一次被访问的时间
    unsigned lru:REDIS_LRU_BITS; /* lru time (relative to server.lruclock) */

    // 引用计数
    int refcount;

    // 指向实际值的指针
    void *ptr;

} robj;
typedef void redisCommandProc(redisClient *c);
typedef int *redisGetKeysProc(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);

/*
 * Redis 命令
 */
struct redisCommand {

    // 命令名字
    char *name;

    // 实现函数
    redisCommandProc *proc;

    // 参数个数
    int arity;

    // 字符串表示的 FLAG
    char *sflags; /* Flags as string representation, one char per flag. */

    // 实际 FLAG
    int flags;    /* The actual flags, obtained from the 'sflags' field. */

    /* Use a function to determine keys arguments in a command line.
     * Used for Redis Cluster redirect. */
    // 从命令中判断命令的键参数。在 Redis 集群转向时使用。
    redisGetKeysProc *getkeys_proc;

    /* What keys should be loaded in background when calling this command? */
    // 指定哪些参数是 key
    int firstkey; /* The first argument that's a key (0 = no keys) */
    int lastkey;  /* The last argument that's a key */
    int keystep;  /* The step between first and last key */

    // 统计信息
    // microseconds 记录了命令执行耗费的总毫微秒数
    // calls 是命令被执行的总次数
    long long microseconds, calls;
};


struct redisServer {

    // 命令表（受到 rename 配置选项的作用）
    dict *commands;             /* Command table */
    // 命令表（无 rename 配置选项的作用）
    dict *orig_commands;        /* Command table before command renaming. */

} server;

extern struct redisServer server;

void getCommand(redisClient *c);
void setCommand(redisClient *c);
void setnxCommand(redisClient *c);
void setexCommand(redisClient *c);
void psetexCommand(redisClient *c);
void appendCommand(redisClient *c);
void strlenCommand(redisClient *c);
void delCommand(redisClient *c);

struct redisCommand redisCommandTable[] = {
        {"get",getCommand,2,"r",0,NULL,1,1,1,0,0},
        {"set",setCommand,-3,"wm",0,NULL,1,1,1,0,0},
        {"setnx",setnxCommand,3,"wm",0,NULL,1,1,1,0,0},
        {"setex",setexCommand,4,"wm",0,NULL,1,1,1,0,0},
        {"psetex",psetexCommand,4,"wm",0,NULL,1,1,1,0,0},
        {"append",appendCommand,3,"wm",0,NULL,1,1,1,0,0},
        {"strlen",strlenCommand,2,"r",0,NULL,1,1,1,0,0},
        {"del",delCommand,-2,"w",0,NULL,1,-1,1,0,0}
};

void setnxCommand(redisClient *c) {
    printf(" func setnxCommand ");
}

void setexCommand(redisClient *c) {
    printf(" func setexCommand ");
}

void psetexCommand(redisClient *c) {
    printf(" func psetexCommand ");
}

void appendCommand(redisClient *c) {
    printf(" func appendCommand ");
}

void strlenCommand(redisClient *c) {
    printf(" func strlenCommand ");
}

void delCommand(redisClient *c) {
    printf(" func delCommand ");
}

void setCommand(redisClient *c) {
    printf(" func setCommand ");
}

void getCommand(redisClient *c) {
    printf(" func getCommand ");
}

void populateCommandTable(void) {
    int j;

    // 命令的数量
    int numcommands = sizeof(redisCommandTable)/sizeof(struct redisCommand);

    for (j = 0; j < numcommands; j++) {

        // 指定命令
        struct redisCommand *c = redisCommandTable+j;

        // 取出字符串 FLAG
        char *f = c->sflags;

        int retval1, retval2;

        // 根据字符串 FLAG 生成实际 FLAG
        while(*f != '\0') {
            switch(*f) {
                case 'w': c->flags |= REDIS_CMD_WRITE; break;
                case 'r': c->flags |= REDIS_CMD_READONLY; break;
                case 'm': c->flags |= REDIS_CMD_DENYOOM; break;
                case 'a': c->flags |= REDIS_CMD_ADMIN; break;
                case 'p': c->flags |= REDIS_CMD_PUBSUB; break;
                case 's': c->flags |= REDIS_CMD_NOSCRIPT; break;
                case 'R': c->flags |= REDIS_CMD_RANDOM; break;
                case 'S': c->flags |= REDIS_CMD_SORT_FOR_SCRIPT; break;
                case 'l': c->flags |= REDIS_CMD_LOADING; break;
                case 't': c->flags |= REDIS_CMD_STALE; break;
                case 'M': c->flags |= REDIS_CMD_SKIP_MONITOR; break;
                case 'k': c->flags |= REDIS_CMD_ASKING; break;
                default: printf("Unsupported command flag"); break;
            }
            f++;
        }

        // 将命令关联到命令表
        retval1 = dictAdd(server.commands, sdsnew(c->name), c);

        /* Populate an additional dictionary that will be unaffected
         * by rename-command statements in redis.conf.
         *
         * 将命令也关联到原始命令表
         *
         * 原始命令表不会受 redis.conf 中命令改名的影响
         */
        retval2 = dictAdd(server.orig_commands, sdsnew(c->name), c);

    }
}
unsigned int dictSdsCaseHash(const void *key) {
    return dictGenCaseHashFunction((unsigned char*)key, sdslen((char*)key));
}

static uint32_t dict_hash_function_seed = 5381;

int dictSdsKeyCaseCompare(void *privdata, const void *key1,
                          const void *key2)
{
    DICT_NOTUSED(privdata);

    return strcasecmp(key1, key2) == 0;
}

void dictSdsDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    sdsfree(val);
}

/* Command table. sds string -> command struct pointer. */
dictType commandTableDictType = {
        dictSdsCaseHash,           /* hash function */
        NULL,                      /* key dup */
        NULL,                      /* val dup */
        dictSdsKeyCaseCompare,     /* key compare */
        dictSdsDestructor,         /* key destructor */
        NULL                       /* val destructor */
};

unsigned int dictSdsCaseHash(const void *key) {
    return dictGenCaseHashFunction((unsigned char*)key, sdslen((char*)key));
}

void testDictSdsCaseHash(){
    char *key = "get";
    int hash = dictSdsCaseHash(key);
    printf("%d",hash);
}

int main(int argc,char * argv[])
{
    // 测试命令初始化
    /*server.commands = dictCreate(&commandTableDictType,NULL);
    server.orig_commands = dictCreate(&commandTableDictType,NULL);
    populateCommandTable();
    dictPrintStats(server.commands);*/

    // testDictSdsCaseHash
    testDictSdsCaseHash();
    return 0;
}
