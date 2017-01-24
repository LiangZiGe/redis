//
// Created by zhangziliang on 2017/1/24.
//

int serverCron(long long id, void *clientData,int i) {
    printf("time %lld\n",id);
}

typedef int aeTimeProc(long long id, void *clientData);

typedef struct aeTimeEvent {
    // 事件处理函数
    aeTimeProc *timeProc;

} aeTimeEvent;

#include <stdio.h>
#include <malloc.h>

int main(int argc,char * argv[])
{

    aeTimeEvent *te;

    te = malloc(sizeof(*te));

    te->timeProc = serverCron;

    te->timeProc(NULL,10000000L);

    return 0;
}