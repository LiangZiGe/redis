//
// Created by zhangziliang on 2017/2/4.
//

// �����־
#include <stdio.h>
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

    // ��¼���ͻ���ִ�е�����
    struct redisCommand *cmd, *lastcmd;

} redisClient;

typedef struct redisObject {

    // ����
    unsigned type:4;

    // ����
    unsigned encoding:4;

    // �������һ�α����ʵ�ʱ��
    unsigned lru:REDIS_LRU_BITS; /* lru time (relative to server.lruclock) */

    // ���ü���
    int refcount;

    // ָ��ʵ��ֵ��ָ��
    void *ptr;

} robj;
typedef void redisCommandProc(redisClient *c);
typedef int *redisGetKeysProc(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);

/*
 * Redis ����
 */
struct redisCommand {

    // ��������
    char *name;

    // ʵ�ֺ���
    redisCommandProc *proc;

    // ��������
    int arity;

    // �ַ�����ʾ�� FLAG
    char *sflags; /* Flags as string representation, one char per flag. */

    // ʵ�� FLAG
    int flags;    /* The actual flags, obtained from the 'sflags' field. */

    /* Use a function to determine keys arguments in a command line.
     * Used for Redis Cluster redirect. */
    // ���������ж�����ļ��������� Redis ��Ⱥת��ʱʹ�á�
    redisGetKeysProc *getkeys_proc;

    /* What keys should be loaded in background when calling this command? */
    // ָ����Щ������ key
    int firstkey; /* The first argument that's a key (0 = no keys) */
    int lastkey;  /* The last argument that's a key */
    int keystep;  /* The step between first and last key */

    // ͳ����Ϣ
    // microseconds ��¼������ִ�кķѵ��ܺ�΢����
    // calls �����ִ�е��ܴ���
    long long microseconds, calls;
};


struct redisServer {

    // ��������ܵ� rename ����ѡ������ã�
    dict *commands;             /* Command table */
    // ��������� rename ����ѡ������ã�
    dict *orig_commands;        /* Command table before command renaming. */

};


void getCommand(redisClient *c);
void setCommand(redisClient *c);

struct redisCommand redisCommandTable[] = {
        {"get",getCommand,2,"r",0,NULL,1,1,1,0,0},
        {"set",setCommand,-3,"wm",0,NULL,1,1,1,0,0}
};

void setCommand(redisClient *c) {
    printf(" func setCommand ");
}

void getCommand(redisClient *c) {
    printf(" func getCommand ");
}

extern struct redisServer server;

void populateCommandTable(void) {
    int j;

    // ���������
    int numcommands = sizeof(redisCommandTable)/sizeof(struct redisCommand);

    for (j = 0; j < numcommands; j++) {

        // ָ������
        struct redisCommand *c = redisCommandTable+j;

        // ȡ���ַ��� FLAG
        char *f = c->sflags;

        int retval1, retval2;

        // �����ַ��� FLAG ����ʵ�� FLAG
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

        // ����������������
        retval1 = dictAdd(server.commands, sdsnew(c->name), c);

        /* Populate an additional dictionary that will be unaffected
         * by rename-command statements in redis.conf.
         *
         * ������Ҳ������ԭʼ�����
         *
         * ԭʼ����������� redis.conf �����������Ӱ��
         */
        retval2 = dictAdd(server.orig_commands, sdsnew(c->name), c);

        printf("%d",retval1);
        printf("%d",retval2);
    }
}

int main(int argc,char * argv[])
{
    populateCommandTable();
    return 0;
}