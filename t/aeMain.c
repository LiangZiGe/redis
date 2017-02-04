//
// Created by zhangziliang on 2017/2/4.
//
typedef struct aeFileEvent {

    // �����¼��������룬
    // ֵ������ AE_READABLE �� AE_WRITABLE ��
    // ���� AE_READABLE | AE_WRITABLE
    int mask; /* one of AE_(READABLE|WRITABLE) */

    // ��·���ÿ��˽������
    void *clientData;

} aeFileEvent;

typedef struct aeEventLoop {

    // ��ע����ļ��¼�
    aeFileEvent *events; /* Registered events */

} aeEventLoop;

#include <stdio.h>
#include <malloc.h>
#define AE_NONE 0

int main(int argc,char * argv[])
{
    aeEventLoop *eventLoop = malloc(sizeof(aeEventLoop));
    int setsize = 10;
    eventLoop->events = malloc(sizeof(aeFileEvent)*setsize);
    int i;
    for (i = 0; i < setsize; i++)
        eventLoop->events[i].mask = AE_NONE;

    int fd = 9;
    aeFileEvent *fe = &eventLoop->events[fd];

    if(fe->mask!=AE_NONE)
        printf("error fd ... ");
    else
        printf(" fd is %d",fd);
    return 0;
}