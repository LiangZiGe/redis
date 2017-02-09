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
        {"del",delCommand,-2,"w",0,NULL,1,-1,1,0,0},
        {"exists",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"setbit",NULL,4,"wm",0,NULL,1,1,1,0,0},
        {"getbit",NULL,3,"r",0,NULL,1,1,1,0,0},
        {"setrange",NULL,4,"wm",0,NULL,1,1,1,0,0},
        {"getrange",NULL,4,"r",0,NULL,1,1,1,0,0},
        {"substr",NULL,4,"r",0,NULL,1,1,1,0,0},
        {"incr",NULL,2,"wm",0,NULL,1,1,1,0,0},
        {"decr",NULL,2,"wm",0,NULL,1,1,1,0,0},
        {"mget",NULL,-2,"r",0,NULL,1,-1,1,0,0},
        {"rpush",NULL,-3,"wm",0,NULL,1,1,1,0,0},
        {"lpush",NULL,-3,"wm",0,NULL,1,1,1,0,0},
        {"rpushx",NULL,3,"wm",0,NULL,1,1,1,0,0},
        {"lpushx",NULL,3,"wm",0,NULL,1,1,1,0,0},
        {"linsert",NULL,5,"wm",0,NULL,1,1,1,0,0},
        {"rpop",NULL,2,"w",0,NULL,1,1,1,0,0},
        {"lpop",NULL,2,"w",0,NULL,1,1,1,0,0},
        {"brpop",NULL,-3,"ws",0,NULL,1,1,1,0,0},
        {"brpoplpush",NULL,4,"wms",0,NULL,1,2,1,0,0},
        {"blpop",NULL,-3,"ws",0,NULL,1,-2,1,0,0},
        {"llen",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"lindex",NULL,3,"r",0,NULL,1,1,1,0,0},
        {"lset",NULL,4,"wm",0,NULL,1,1,1,0,0},
        {"lrange",NULL,4,"r",0,NULL,1,1,1,0,0},
        {"ltrim",NULL,4,"w",0,NULL,1,1,1,0,0},
        {"lrem",NULL,4,"w",0,NULL,1,1,1,0,0},
        {"rpoplpush",NULL,3,"wm",0,NULL,1,2,1,0,0},
        {"sadd",NULL,-3,"wm",0,NULL,1,1,1,0,0},
        {"srem",NULL,-3,"w",0,NULL,1,1,1,0,0},
        {"smove",NULL,4,"w",0,NULL,1,2,1,0,0},
        {"sismember",NULL,3,"r",0,NULL,1,1,1,0,0},
        {"scard",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"spop",NULL,2,"wRs",0,NULL,1,1,1,0,0},
        {"srandmember",NULL,-2,"rR",0,NULL,1,1,1,0,0},
        {"sinter",NULL,-2,"rS",0,NULL,1,-1,1,0,0},
        {"sinterstore",NULL,-3,"wm",0,NULL,1,-1,1,0,0},
        {"sunion",NULL,-2,"rS",0,NULL,1,-1,1,0,0},
        {"sunionstore",NULL,-3,"wm",0,NULL,1,-1,1,0,0},
        {"sdiff",NULL,-2,"rS",0,NULL,1,-1,1,0,0},
        {"sdiffstore",NULL,-3,"wm",0,NULL,1,-1,1,0,0},
        {"smembers",NULL,2,"rS",0,NULL,1,1,1,0,0},
        {"sscan",NULL,-3,"rR",0,NULL,1,1,1,0,0},
        {"zadd",NULL,-4,"wm",0,NULL,1,1,1,0,0},
        {"zincrby",NULL,4,"wm",0,NULL,1,1,1,0,0},
        {"zrem",NULL,-3,"w",0,NULL,1,1,1,0,0},
        {"zremrangebyscore",NULL,4,"w",0,NULL,1,1,1,0,0},
        {"zremrangebyrank",NULL,4,"w",0,NULL,1,1,1,0,0},
        {"zremrangebylex",NULL,4,"w",0,NULL,1,1,1,0,0},
        {"zunionstore",NULL,-4,"wm",0,NULL,0,0,0,0,0},
        {"zinterstore",NULL,-4,"wm",0,NULL,0,0,0,0,0},
        {"zrange",NULL,-4,"r",0,NULL,1,1,1,0,0},
        {"zrangebyscore",NULL,-4,"r",0,NULL,1,1,1,0,0},
        {"zrevrangebyscore",NULL,-4,"r",0,NULL,1,1,1,0,0},
        {"zrangebylex",NULL,-4,"r",0,NULL,1,1,1,0,0},
        {"zrevrangebylex",NULL,-4,"r",0,NULL,1,1,1,0,0},
        {"zcount",NULL,4,"r",0,NULL,1,1,1,0,0},
        {"zlexcount",NULL,4,"r",0,NULL,1,1,1,0,0},
        {"zrevrange",NULL,-4,"r",0,NULL,1,1,1,0,0},
        {"zcard",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"zscore",NULL,3,"r",0,NULL,1,1,1,0,0},
        {"zrank",NULL,3,"r",0,NULL,1,1,1,0,0},
        {"zrevrank",NULL,3,"r",0,NULL,1,1,1,0,0},
        {"zscan",NULL,-3,"rR",0,NULL,1,1,1,0,0},
        {"hset",NULL,4,"wm",0,NULL,1,1,1,0,0},
        {"hsetnx",NULL,4,"wm",0,NULL,1,1,1,0,0},
        {"hget",NULL,3,"r",0,NULL,1,1,1,0,0},
        {"hmset",NULL,-4,"wm",0,NULL,1,1,1,0,0},
        {"hmget",NULL,-3,"r",0,NULL,1,1,1,0,0},
        {"hincrby",NULL,4,"wm",0,NULL,1,1,1,0,0},
        {"hincrbyfloat",NULL,4,"wm",0,NULL,1,1,1,0,0},
        {"hdel",NULL,-3,"w",0,NULL,1,1,1,0,0},
        {"hlen",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"hkeys",NULL,2,"rS",0,NULL,1,1,1,0,0},
        {"hvals",NULL,2,"rS",0,NULL,1,1,1,0,0},
        {"hgetall",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"hexists",NULL,3,"r",0,NULL,1,1,1,0,0},
        {"hscan",NULL,-3,"rR",0,NULL,1,1,1,0,0},
        {"incrby",NULL,3,"wm",0,NULL,1,1,1,0,0},
        {"decrby",NULL,3,"wm",0,NULL,1,1,1,0,0},
        {"incrbyfloat",NULL,3,"wm",0,NULL,1,1,1,0,0},
        {"getset",NULL,3,"wm",0,NULL,1,1,1,0,0},
        {"mset",NULL,-3,"wm",0,NULL,1,-1,2,0,0},
        {"msetnx",NULL,-3,"wm",0,NULL,1,-1,2,0,0},
        {"randomkey",NULL,1,"rR",0,NULL,0,0,0,0,0},
        {"select",NULL,2,"rl",0,NULL,0,0,0,0,0},
        {"move",NULL,3,"w",0,NULL,1,1,1,0,0},
        {"rename",NULL,3,"w",0,NULL,1,2,1,0,0},
        {"renamenx",NULL,3,"w",0,NULL,1,2,1,0,0},
        {"expire",NULL,3,"w",0,NULL,1,1,1,0,0},
        {"expireat",NULL,3,"w",0,NULL,1,1,1,0,0},
        {"pexpire",NULL,3,"w",0,NULL,1,1,1,0,0},
        {"pexpireat",NULL,3,"w",0,NULL,1,1,1,0,0},
        {"keys",NULL,2,"rS",0,NULL,0,0,0,0,0},
        {"scan",NULL,-2,"rR",0,NULL,0,0,0,0,0},
        {"dbsize",NULL,1,"r",0,NULL,0,0,0,0,0},
        {"auth",NULL,2,"rslt",0,NULL,0,0,0,0,0},
        {"ping",NULL,1,"rt",0,NULL,0,0,0,0,0},
        {"echo",NULL,2,"r",0,NULL,0,0,0,0,0},
        {"save",NULL,1,"ars",0,NULL,0,0,0,0,0},
        {"bgsave",NULL,1,"ar",0,NULL,0,0,0,0,0},
        {"bgrewriteaof",NULL,1,"ar",0,NULL,0,0,0,0,0},
        {"shutdown",NULL,-1,"arlt",0,NULL,0,0,0,0,0},
        {"lastsave",NULL,1,"rR",0,NULL,0,0,0,0,0},
        {"type",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"multi",NULL,1,"rs",0,NULL,0,0,0,0,0},
        {"exec",NULL,1,"sM",0,NULL,0,0,0,0,0},
        {"discard",NULL,1,"rs",0,NULL,0,0,0,0,0},
        {"sync",NULL,1,"ars",0,NULL,0,0,0,0,0},
        {"psync",NULL,3,"ars",0,NULL,0,0,0,0,0},
        {"replconf",NULL,-1,"arslt",0,NULL,0,0,0,0,0},
        {"flushdb",NULL,1,"w",0,NULL,0,0,0,0,0},
        {"flushall",NULL,1,"w",0,NULL,0,0,0,0,0},
        {"sort",NULL,-2,"wm",0,NULL,1,1,1,0,0},
        {"info",NULL,-1,"rlt",0,NULL,0,0,0,0,0},
        {"monitor",NULL,1,"ars",0,NULL,0,0,0,0,0},
        {"ttl",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"pttl",NULL,2,"r",0,NULL,1,1,1,0,0},
        {"persist",NULL,2,"w",0,NULL,1,1,1,0,0},
        {"slaveof",NULL,3,"ast",0,NULL,0,0,0,0,0},
        {"debug",NULL,-2,"as",0,NULL,0,0,0,0,0},
        {"config",NULL,-2,"art",0,NULL,0,0,0,0,0},
        {"subscribe",NULL,-2,"rpslt",0,NULL,0,0,0,0,0},
        {"unsubscribe",NULL,-1,"rpslt",0,NULL,0,0,0,0,0},
        {"psubscribe",NULL,-2,"rpslt",0,NULL,0,0,0,0,0},
        {"punsubscribe",NULL,-1,"rpslt",0,NULL,0,0,0,0,0},
        {"publish",NULL,3,"pltr",0,NULL,0,0,0,0,0},
        {"pubsub",NULL,-2,"pltrR",0,NULL,0,0,0,0,0},
        {"watch",NULL,-2,"rs",0,NULL,1,-1,1,0,0},
        {"unwatch",NULL,1,"rs",0,NULL,0,0,0,0,0},
        {"cluster",NULL,-2,"ar",0,NULL,0,0,0,0,0},
        {"restore",NULL,-4,"awm",0,NULL,1,1,1,0,0},
        {"restore-asking",NULL,-4,"awmk",0,NULL,1,1,1,0,0},
        {"migrate",NULL,-6,"aw",0,NULL,0,0,0,0,0},
        {"asking",NULL,1,"r",0,NULL,0,0,0,0,0},
        {"readonly",NULL,1,"r",0,NULL,0,0,0,0,0},
        {"readwrite",NULL,1,"r",0,NULL,0,0,0,0,0},
        {"dump",NULL,2,"ar",0,NULL,1,1,1,0,0},
        {"object",NULL,-2,"r",0,NULL,2,2,2,0,0},
        {"client",NULL,-2,"ar",0,NULL,0,0,0,0,0},
        {"eval",NULL,-3,"s",0,NULL,0,0,0,0,0},
        {"evalsha",NULL,-3,"s",0,NULL,0,0,0,0,0},
        {"slowlog",NULL,-2,"r",0,NULL,0,0,0,0,0},
        {"script",NULL,-2,"ras",0,NULL,0,0,0,0,0},
        {"time",NULL,1,"rR",0,NULL,0,0,0,0,0},
        {"bitop",NULL,-4,"wm",0,NULL,2,-1,1,0,0},
        {"bitcount",NULL,-2,"r",0,NULL,1,1,1,0,0},
        {"bitpos",NULL,-3,"r",0,NULL,1,1,1,0,0},
        {"wait",NULL,3,"rs",0,NULL,0,0,0,0,0},
        {"pfselftest",NULL,1,"r",0,NULL,0,0,0,0,0},
        {"pfadd",NULL,-2,"wm",0,NULL,1,1,1,0,0},
        {"pfcount",NULL,-2,"w",0,NULL,1,1,1,0,0},
        {"pfmerge",NULL,-2,"wm",0,NULL,1,-1,1,0,0},
        {"pfdebug",NULL,-3,"w",0,NULL,0,0,0,0,0}
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

void testDictSdsCaseHash(){
    char *key = "get";
    sds s = sdsnew(key);
    int hash = dictSdsCaseHash(s);
    printf("%d",hash);
}

void testTolower(){
    char *key = "GET";
     int i = tolower(*key++);
    printf("%c ascii code is %d",i,i);
}

int main(int argc,char * argv[])
{
    // 测试命令初始化
    server.commands = dictCreate(&commandTableDictType,NULL);
    server.orig_commands = dictCreate(&commandTableDictType,NULL);
    populateCommandTable();
    dictPrintStats(server.commands);

    // testDictSdsCaseHash
//    testDictSdsCaseHash();

    // testTolower
//    testTolower();
    return 0;
}
