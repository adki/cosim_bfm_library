//----------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights are reserved by Ando Ki.
//----------------------------------------------------------------------------
// main.c
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include "cosim_ipc.h"

//----------------------------------------------------------------------------
int tx;
int rx;
int   cid ;
void *chan;
#define MLENG 64
char buf[128];

int host(int cid);
int target(int cid);

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    int a;
    int level=0;

    tx = 0;
    rx = 0;
    cid  = 0;
    chan = 0;
    for (a=1; a<argc; a++)
    {
         if ((!strcmp(argv[a],"-h"))||(!strcmp(argv[a],"-?"))) {
            printf("%s [options]\n", argv[0]);
            printf("   -tx       : host\n");
            printf("   -rx       : target\n");
            printf("   -v  level : set verbos level\n");
            printf("   -h        : print help\n");
            printf("   -?        : print help\n");
            return 0;
         } else if (!(strcmp(argv[a],"-cid"))) {
            cid = atoi(argv[++a]);
         } else if (!(strcmp(argv[a],"-tx"))) {
            tx = 1; rx = 0;
         } else if (!(strcmp(argv[a],"-rx"))) {
            if (tx==1) {
                printf("only one of \"-tx\" or \"-rx\" is allowed\n"); 
                return 1;
            }
            tx = 0; rx = 1;
         } else if (!(strcmp(argv[a],"-v"))) {
            level = atoi(argv[++a]);
         } else {
            printf("un-known option %s\n", argv[a]);
            return 1;
         }
    }
    if ((tx==0)&&(rx==0)) {
        printf("one of \"-tx\" or \"-rx\" should be given\n"); 
        return 1;
    }
    chn_init();
    chn_set_verbose(level);
    if (tx==1) {
        host(0);
    } else {
        target(0);
    }
    return 0;
}

//----------------------------------------------------------------------------
#define DELAY(D) { volatile int d; for (d=0; d<(D); d++); }
//----------------------------------------------------------------------------
int host(int cid)
{
    int i;

    if (chn_create_connect(cid, CHAN_HOST)<0) return 1;
    printf("host established channel\n");
    chn_barrier(cid);

    for (i=0; i<5; i++) {
         sprintf(buf, "It is sender at HOST: %d.", i);
         int len = strlen(buf) + 1;
         if (chn_send(cid, len, buf)!=len) {
             printf("Unsuccessful sending: %s\n", chn_error_msg(cid,0));
         } else {
           DELAY(10000);
           if (chn_recv(cid, MLENG, buf)<0) {
               printf("Unsuccessful receiving: %s\n", chn_error_msg(cid,0));
           } else {
             // mind that buf[] should have null-terminated string.
             printf("received \"%s\"\n", buf);
           }
           DELAY(11000);
         }
    }

printf("Enter to close:"); getchar();

    if (chn_close(cid)<0) {
        printf("fail to close channel\n");
        return 1;
    }
    return 0;
}

//----------------------------------------------------------------------------
int target(int cid)
{
    int i;

    char buf[128];
    if (chn_create_connect(cid, CHAN_TARGET)<0) return 1;
    printf("target established channel\n");
    chn_barrier(cid);

    for (i=0; i<5; i++) {
       if (chn_recv(cid, MLENG, buf)<0) {
           printf("Unsuccessful receiving: %s\n", chn_error_msg(cid,0));
       } else {
         DELAY(12000);
         // mind that buf[] should have null-terminated string.
         printf("received \"%s\"\n", buf);
         sprintf(buf, "It is target at TARGET: %d", i);
         int len = strlen(buf)+1;
         if (chn_send(cid, len, buf)!=len) {
             printf("Unsuccessful sending: %s\n", chn_error_msg(cid,0));
         }
         DELAY(2000);
       }
    }

printf("Enter to close:"); getchar();

    if (chn_close(cid)<0) {
        printf("fail to close channel\n");
        return 1;
    }
    return 0;
}

//----------------------------------------------------------------------------
void interrupt_handler(int sig) {
    switch (sig) {
    case SIGNAL_INTERRUPT: // ^C
    case SIGNAL_TERMINATION: // ^\
        fflush(stdout); fflush(stderr);
        chn_close(-1); // close all
        exit(0);
        break;
    default :
        assert((sig==SIGNAL_INTERRUPT) || (sig==SIGNAL_TERMINATION));
        break;
    }
}

//----------------------------------------------------------------------------
int register_interrupt()
{
    if ((signal(SIGNAL_INTERRUPT, interrupt_handler)==SIG_ERR)||
        (signal(SIGNAL_TERMINATION, interrupt_handler)==SIG_ERR)) {
        printf("failed to register interrupt handler.\n");
        assert(0);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Revision history
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//----------------------------------------------------------------------------
