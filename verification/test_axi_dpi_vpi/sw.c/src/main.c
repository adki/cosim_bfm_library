//------------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights are reserved by Ando Ki.
//------------------------------------------------------------------------------
// main.c
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <getopt.h>
#include <signal.h>
#include "cosim_bfm_api.h"

const char program[]="test";
const unsigned int version=0x20210810;

int verbose = 0;
int cid = 0;

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    extern void sig_handle(int);
    extern int  arg_parser(int, char **);
    extern int  mem_test();

    if ((signal(SIGINT, sig_handle)==SIG_ERR)
            #ifndef WIN32
            ||(signal(SIGQUIT, sig_handle)==SIG_ERR)
            #endif
            ) {
          fprintf(stderr, "Error: signal error\n");
          exit(1);
    }
    
    if (arg_parser(argc, argv)) return 1;

    bfm_set_verbose(verbose); // optional
    bfm_open(cid); // mandatory
    bfm_barrier(cid); // mandatory

    mem_test();

    bfm_close(cid); // mandatory

    return 0;
}

//------------------------------------------------------------------------------
int mem_test()
{
    extern int mem_test_core( uint32_t addr
                            , uint32_t depth // num of bytes for all
                            , int      bsize // num of bytes for each beat, 1, 2, 4.
                            , int      leng);// burst length


    uint32_t  addr=0xA0000000;
    uint32_t  depth=4*16*3+3;

    for (int leng=1; leng<=16; leng*=2) {
        for (int bsize=1; bsize<=4; bsize*=2) {
            mem_test_core(addr, depth, bsize, leng);
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
// It writes known data to the location from addr to (addr+depth) in bsize-bytes wise.
// It reads data from the location from addr to (addr+depth) in bsize-bytes wise.
// It compares read data to the known data.
int mem_test_core( uint32_t addr
                 , uint32_t depth // num of bytes for all
                 , int      bsize // num of bytes for each beat, 1, 2, 4.
                 , int      leng) // burst length
{
    //--------------------------------------------------------------------------
    #define LENGTH 4*1024
    uint8_t dataW[LENGTH];
    uint8_t dataR[LENGTH];

    for (int idx=0; idx<LENGTH; idx++) {
         // note that dataW carries byte-stream in little-endian style.
         dataW[idx] = (idx+1)&0xFF;
         dataR[idx] = 0;
    }

    //--------------------------------------------------------------------------
    uint32_t offset;
    uint8_t *dW=dataW;
    for (offset=0x0; (offset+leng)<depth; offset+=(bsize*leng)) {
        bfm_write(addr+offset, dW, bsize, leng);
        dW += (bsize*leng);
    }
    if (offset<depth) {
        int ln = (depth-offset)/bsize;
        if (ln>0) {
            bfm_write(addr+offset, dW, bsize, ln);
            offset += (bsize*ln);
            dW += (bsize*ln);
        }
    }
    if (offset<depth) {
        bfm_write(addr+offset, dW, 1, depth-offset);
        dW += (depth-offset);
        offset += depth-offset;
    }
    uint8_t *dR=dataR;
    for (offset=0x0; (offset+leng)<depth; offset+=(bsize*leng)) {
        bfm_read(addr+offset, dR, bsize, leng);
        dR += (bsize*leng);
    }
    if (offset<depth) {
        int ln = (depth-offset)/bsize;
        if (ln>0) {
            bfm_read(addr+offset, dR, bsize, ln);
            offset += (bsize*ln);
            dR += (bsize*ln);
        }
    }
    if (offset<depth) {
        bfm_read(addr+offset, dR, 1, (depth-offset));
        dR += (depth-offset);
        offset += depth-offset;
    }

    int error=0;
    for (int idx=0; idx<depth; idx+=1) {
         if (dataW[idx]!=dataR[idx]) {
             printf("[%d] 0x%02X:%02X\n", idx, dataW[idx], dataR[idx]);
             error++;
         }
    }
    if (error==0) printf("Memory test %d-byte size %d-leng OK %d.\n", bsize, leng, depth);
    else          printf("Memory test %d-byte size %d-leng mis-match %d out of %d.\n", bsize, leng, error, depth);

    return 0;
}

//------------------------------------------------------------------------------
int arg_parser(int argc, char **argv)
{
    int opt;
    int longidx=0;
    extern void help(int, char **);
    extern void print_license(void);
    extern void print_version(void);

    struct option longopts[] = {
          {"cid"    , required_argument, 0, 'C'}
        , {"verbose", required_argument, 0, 'g'}
        , {"version", no_argument      , 0, 'v'}
        , {"license", no_argument      , 0, 'l'}
        , {"help"   , no_argument      , 0, 'h'}
        , { 0       , 0                , 0,  0 }
    };

    while ((opt=getopt_long(argc, argv, "C:g:vlh?", longopts, &longidx))!=-1) {
       switch (opt) {
       case 'C': cid = atoi(optarg); break;
       case 'g': verbose = atoi(optarg); break;
       case 'v': print_version(); exit(0); break;
       case 'l': print_license(); exit(0); break;
       case 'h':
       case '?': help(argc, argv); exit(0); break;
       case  0 : return -1;
                 break;
       default: 
          fprintf(stderr, "undefined option: %c\n", optopt);
          help(argc, argv);
          exit(1);
       }
    }

    return 0;
}

//------------------------------------------------------------------------------
void help(int argc, char **argv)
{
  fprintf(stderr, "[Usage] %s [options]\n", argv[0]);
  fprintf(stderr, "\t-C,--cid=num      channel id (default: %d)\n", cid);
  fprintf(stderr, "\t-g,--verbose=num  verbose level  (default: %d)\n", verbose);
  fprintf(stderr, "\t-v,--version      print version\n");
  fprintf(stderr, "\t-l,--license      print license message\n");
  fprintf(stderr, "\t-h                print help message\n");
  fprintf(stderr, "\n");
}

//------------------------------------------------------------------------------
const char license[] =
"Copyright (c) 2021 by Ando Ki (andoki@gmail.com).\n\n\
This contents and its associated materials are licensed with the 2-clause BSD license to make the program and library useful in open and closed source products independent of their licensing scheme. Each contributor holds copyright over their respective contribution.\n\
All contents are provided as it is WITHOUT ANY WARRANTY and NO TECHNICAL SUPPORT will be provided for problems that might arise.\n";
void print_license(void)
{
     printf("%s %X\n\n", program, version);
     printf("%s", license);
}

//------------------------------------------------------------------------------
void print_version(void)
{
     printf("%X\n", version);
}

//------------------------------------------------------------------------------
void sig_handle(int sig)
{
   extern void cleanup();
   switch (sig) {
   case SIGINT:
   #ifndef WIN32
   case SIGQUIT:
   #endif
        exit(0);
        break;
   }
}

//------------------------------------------------------------------------------
// Revision history
//
// 2021.08.01: Started by Ando Ki.
//------------------------------------------------------------------------------
