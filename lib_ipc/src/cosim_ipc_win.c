#ifndef COSIM_IPC_WIN_C
#define COSIM_IPC_WIN_C
//----------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights are reserved by Ando Ki.
//----------------------------------------------------------------------------
// cosim_ipc_win.c
//----------------------------------------------------------------------------
#define    PIPE_NAME              "\\\\.\\pipe\\ipc" 
#define    PIPE_TIMEOUT           20000
#define    CONNECT_TRY_TIMEOUT    20

//----------------------------------------------------------------------------
char *chn_error_msg_string[] = {
/* 0 */ "nothing wrong",
/* 1 */ "failed to allocate memory for pipe name",
/* 2 */ "invalid side reference",
/* 3 */ "failed to connect to existing pipe",
/* 4 */ "failed to create pipe",
/* 5 */ "failed to connect to existing pipe due to the pipe is busy",
/* 6 */ "failed to disconnect pipe",
/* 7 */ "failed to close pipe",
/* 8 */ "length exceeds maximum buffer size in sending",
/* 9 */ "send error with error message",
/*10 */ "length exceeds maximum buffer size in receiving",
/*11 */ "receive error with error message",
/*12 */ "channel is not exist yet for barrier",
/*13 */ "unexpected first message from target for barrier",
/*14 */ "unexpected second message from target for barrier",
/*15 */ "host pid mis-match from target for barrier",
/*16 */ "unexpected first message from host for barrier",
/*17 */ "unexpected second message from host for barrier",
/*18 */ "target pid mis-match from host for barrier"
/*19 */ "reserved"
/*20 */ "error message not valid"
};
static int max_error_num = 21;

//----------------------------------------------------------------------------
static HANDLE file_lock=INVALID_HANDLE_VALUE;
static char file_lock_name[64]="\0";

//----------------------------------------------------------------------------
// use 'LockFile()/LockFileEx()/UnlockFile()' for exclusive execution of thie function.
static int
cc_printf (const int level, const char *format, ...)
{
    int ret = 0;
    BOOL fSuccess = FALSE;
    va_list ap;
    va_start (ap, format);
    if (level <= m_verbose) {
        #if defined(MULTI_THREADING_SAFE)
        // Actually lock the file.  Specify exclusive access, and fail 
        // immediately if the lock cannot be obtained.
        fSuccess = LockFileEx(file_lock         // exclusive access, 
                             ,LOCKFILE_EXCLUSIVE_LOCK
                             ,0  // reserved, must be zero
                             ,4  // number of bytes to lock
                             ,0
                             ,0); // contains the file offset
        if (!fSuccess) { // Handle the error.
           fprintf(stderr, "LockFileEx failed (%d)\n", GetLastError());
           return -1;
        } //else fprintf(stderr, "LockFileEx succeeded\n");
        #endif

        fprintf (stderr, "-ipc- ");
        ret = vfprintf (stderr, format, ap);

        #if defined(MULTI_THREADING_SAFE)
        // Unlock the file.
        fSuccess = UnlockFileEx(file_lock
                               ,0   // reserved, must be zero
                               ,4   // num. of bytes to unlock
                               ,0
                               ,0); // contains the file offset
        if (!fSuccess) { // Handle the error.
           fprintf (stderr, "UnlockFileEx failed (%d)\n", GetLastError());
           return -1;
        } //else fprintf(stderr, "UnlockFileEx succeeded\n");
        #endif
    }
    va_end (ap);
    return ret;
}

//----------------------------------------------------------------------------
int
_chn_init(void)
{
    #if defined(MULTI_THREADING_SAFE)
    // Create the file, open for both read and write.
    // Refer: hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD pid = GetCurrentProcessId();
    char fname[64];
    sprintf(file_lock_name, ".lock_file%d.txt", pid);
    file_lock = CreateFile(TEXT(file_lock_name)
                          ,GENERIC_READ | GENERIC_WRITE
                          ,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE // open with exclusive access
                          ,NULL       // no security attributes
                          ,OPEN_ALWAYS // creating a new temp file
                          ,0          // not overlapped index/O
                          ,NULL);
    
    if (file_lock==INVALID_HANDLE_VALUE) { // Handle the error.
        fprintf(stderr, "CreateFile failed (%d)\n", GetLastError());
        return -1;
    }
    #endif

    return 0;
}

//----------------------------------------------------------------------------
// Connect to Channel
//
// [input]
// chan_id:   channel ID
// side:  which side of message (host or target)
// [return]
// !=NULL: OK
// NULL: something wrong, and 'error_msg_num' will have error number
//----------------------------------------------------------------------------
static void *
_chn_connect (int chan_id, int side) 
{
    HANDLE hPipe;
    LPTSTR lpszPipename;
    char *pipename;

    //------------------------------------------------------------------------
    if((side!=CHAN_HOST)&&(side!=CHAN_TARGET)) {
        error_msg_num[chan_id] = 1;
        return NULL;
    }
    if(side==CHAN_HOST) {
        cc_printf(1,"Creating/connecting channel %d for host.\n", chan_id);
    } else {
        cc_printf(1,"Creating/connecting channel %d for target.\n", chan_id);
    }
    side_ht[chan_id] = side;

    //------------------------------------------------------------------------
    pipename = (char *)calloc(strlen(PIPE_NAME)+2, sizeof(char));
    if(pipename == NULL) {
        error_msg_num[chan_id] = 1;
        return NULL;
    }
    sprintf(pipename, "%s%02d", PIPE_NAME, chan_id);
    lpszPipename = pipename;

    //------------------------------------------------------------------------
    // HANDLE CreateFile(
    //             LPCTSTR lpFileName,
    //             DWORD dwDesiredAccess,
    //             DWORD dwShareMode,
    //             LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    //             DWORD dwCreationDisposition,
    //             DWORD dwFlagsAndAttributes,
    //             HANDLE hTemplateFile
    // );
    hPipe = CreateFile (lpszPipename,    // pipe name 
                         GENERIC_READ | GENERIC_WRITE,    // read and write access 
                         0,    // no sharing 
                         NULL,    // no security attributes
                         OPEN_EXISTING,    // opens existing pipe 
                         0,    // default attributes 
                         NULL);    // no template file 

    if (hPipe == INVALID_HANDLE_VALUE) {
        if (GetLastError()==ERROR_PIPE_BUSY) {
            error_msg_num[chan_id] = 5;
            return NULL;
        } else {
            error_msg_num[chan_id] = 3;
            return NULL;
        }
    }
        
    //if (!WaitNamedPipe (lpszPipename, PIPE_TIMEOUT)) {
    //    printf ("Could not open pipe");
    //    return NULL;
    //}

    return hPipe;
}

//----------------------------------------------------------------------------
// It tried to create a named queue and it tried to connect it after creation
// when there is no named queue.
// It tried to connect the named queue when there is named queue.
// [input]
// chan_id:  channel ID
// side: which side of message (host or target)
// [return]
// !=NULL: OK
// NULL: something wrong, and 'error_msg_num' will have error number
//----------------------------------------------------------------------------
static void *
_chn_create_connect (int chan_id, int side) 
{
    HANDLE hPipe; // typedef PVOID HANDLE; typedef void* PVOID 
    LPTSTR lpszPipename;
    char *pipename;
    BOOL fConnected;
    int fConnect_try = 0;

    //------------------------------------------------------------------------
    if((side!=CHAN_HOST)&&(side!=CHAN_TARGET)) {
        error_msg_num[chan_id] = 1;
        return NULL;
    }
    if(side==CHAN_HOST) {
        cc_printf(1,"Creating/connecting channel %d for host.\n", chan_id);
    } else {
        cc_printf(1,"Creating/connecting channel %d for target.\n", chan_id);
    }
    side_ht[chan_id] = side;

    //------------------------------------------------------------------------
    pipename = (char *)calloc(strlen(PIPE_NAME)+2, sizeof(char));
    if(pipename == NULL) {
        error_msg_num[chan_id] = 1;
        return NULL;
    }
    sprintf(pipename, "%s%02d", PIPE_NAME, chan_id);
    lpszPipename = pipename;
    
    //------------------------------------------------------------------------
    // Create Named Pipe
    // HANDLE CreateNamedPipe(
    //             LPCTSTR lpName,
    //             DWORD dwOpenMode,
    //             DWORD dwPipeMode,
    //             DWORD nMaxInstances,
    //             DWORD nOutBufferSize,
    //             DWORD nInBufferSize,
    //             DWORD nDefaultTimeOut,
    //             LPSECURITY_ATTRIBUTES lpSecurityAttributes
    // );
    hPipe = CreateNamedPipe (lpszPipename,    // pipe name 
                             PIPE_ACCESS_DUPLEX,    // read/write access 
                             //FILE_FLAG_WRITE_THROUGH,    // write-through cache
                             PIPE_TYPE_BYTE | PIPE_WAIT,    // blocking mode 
                             1,    // number of instances 
                             PIPE_BUFSIZE,    // output buffer size 
                             PIPE_BUFSIZE,    // input buffer size 
                             NMPWAIT_USE_DEFAULT_WAIT,    // client time-out 
                             NULL);    // no security attributes 

    //------------------------------------------------------------------------
    if (hPipe == INVALID_HANDLE_VALUE) {
        // Try to connect to the existing pipe
        cc_printf (1, "Warning! Requested pipe already exists.\n");
        cc_printf (1, "Try to connect to the existing pipe..\n");
        hPipe = _chn_connect (chan_id, side);
        if(hPipe == NULL) {
            error_msg_num[chan_id] = 3;
            return NULL;
        } else {
            cc_printf (1, "Connected to pipe with channel id %d.\n", chan_id);
        }
    } else {
        // Activate pipe
        fConnected = 0;
#if 0
        while(fConnected == 0) {
            fConnected = ConnectNamedPipe (hPipe, NULL);
            if (!fConnected) {
                if(fConnect_try>=CONNECT_TRY_TIMEOUT) {
                    //cc_printf(1, "Failed to create message queue. Error is %d.\n", GetLastError());
                    error_msg_num[chan_id] = 4;
                    return NULL;
                }
                fConnect_try ++;
                Sleep(1000);
            } else {
                cc_printf (1, "Connected to pipe with channel id %d.\n", chan_id);
                break;
            }
        }
#else
        fConnected = 1;
        fConnected = ConnectNamedPipe (hPipe, NULL);
        // If the operation is synchronous, ConnectNamedPipe does not return
        // until the operation has completed. If the function succeeds,
        // the return value is nonzero. If the function fails,
        // the return value is zero. To get extended error information,
        // call GetLastError.
        // If a client connects before the function is called,
        // the function returns zero and GetLastError returns
        // ERROR_PIPE_CONNECTED. This can happen if a client connects
        // in the interval between the call to CreateNamedPipe and
        // the call to ConnectNamedPipe. In this situation,
        // there is a good connection between client and server,
        // even though the function returns zero.
        if (!fConnected) {
            error_msg_num[chan_id] = 4;
            return NULL;
        } else {
            cc_printf (1, "Connected to pipe with channel id %d.\n", chan_id);
        }

#endif
    }

    //------------------------------------------------------------------------
    cc_printf (1, "Allocated pipe for channel %d with channel handle %d.\n", chan_id, hPipe);
    //cc_printf (1, "Changing state of handle %d..\n", hPipe);

    return hPipe;
}

//----------------------------------------------------------------------------
#if defined(MULTI_THREADING_SAFE)
#define DELETE_LOCK_FILE {\
    int i, flag;\
    flag = 0;\
    for (i=0; i<MAX_NUM_CHAN; i++) {\
         if (chan_handle[i]!=NULL) { flag=1; break; }\
    }\
    if (!flag) { DeleteFile(file_lock_name); }}
#else
#define DELETE_LOCK_FILE
#endif
//----------------------------------------------------------------------------
static int 
_chn_close (int chan_id) //chn_close (HANDLE hPipe) 
{
    HANDLE hPipe = (HANDLE)chan_handle[chan_id];
    if(DisconnectNamedPipe ((HANDLE)hPipe) == 0) {
        //chan_handle[chan_id] = NULL;
        DELETE_LOCK_FILE
        //cc_printf(2, "Failed to disconnect pipe with error %d\n", GetLastError());
        error_msg_num[chan_id] = 6;
        return 6;
    }
    if(CloseHandle (hPipe) == 0) {
        //chan_handle[chan_id] = NULL;
        DELETE_LOCK_FILE
        //cc_printf(2, "Failed to close pipe with error %d\n", GetLastError());
        error_msg_num[chan_id] = 7;
        return 7;
    }
    cc_printf (1, "Closed message queue with channel id: %d\n", chan_id);
    error_msg_num[chan_id] = 0;
    chan_handle[chan_id] = NULL;
    DELETE_LOCK_FILE
    return 0;
} 
    
//----------------------------------------------------------------------------
static int
_chn_change_mode(int chan_id, int nbon)
{
    HANDLE hndl = (HANDLE)chan_handle[chan_id];
#if 1
    DWORD dwmode;

    if(nbon) {
        dwmode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
        if(!SetNamedPipeHandleState((HANDLE)hndl, &dwmode,
                    NULL, NULL)) { 
            if(GetLastError() != ERROR_PIPE_BUSY)
                //cc_printf (2, "Failed to set message queue %d to nowait mode. Error is %d.\n", hndl, GetLastError());
            return -1;
        }
    } else {
        dwmode = PIPE_READMODE_BYTE | PIPE_WAIT;
        if(!SetNamedPipeHandleState((HANDLE)hndl, &dwmode,
                    NULL, NULL)) { 
            if(GetLastError() != ERROR_PIPE_BUSY)
                //cc_printf (2, "Failed to set message queue %d to wait mode. Error is %d.\n", hndl, GetLastError());
            return -1;
        }
    }
#endif
    return 0;
}

//----------------------------------------------------------------------------
// chan_id:
// buf[]:
// len:
// nbon: 0 for blocking, 1 for non-blocking
//
// return the num of bytes sent that will be >=0 on successful.
// return < 0 on failure.
static int 
_chn_send (int chan_id, int len, char buf[], int nbon) 
{
    HANDLE hPipe = (HANDLE)chan_handle[chan_id];
    DWORD cbBytes = 0;
    BOOL fSuccess;

    error_msg_num[chan_id] = 0;

    if(len >= PIPE_BUFSIZE) {
        error_msg_num[chan_id] = 8;
        return -1;
    }

    fSuccess = WriteFile (hPipe, buf, len, &cbBytes, NULL);
    if (fSuccess) {
        return cbBytes;
    } else {
       if (GetLastError()==ERROR_IO_PENDING) { //
           return cbBytes;
       }
//printf("Error! send cid=%d error with error message %d.\n", chan_id, GetLastError()); fflush(stdout);
       //cc_printf (2, "Error! send error with error message %d.\n", GetLastError());
       error_msg_num[chan_id] = 9;
       return -2;
    }
}

//----------------------------------------------------------------------------
// chan_id:
// buf[]:
// len:
// nbon: 0 for blocking, 1 for non-blocking
//
// return the num of bytes received that will be >0 on successful.
// return < 0 on failure.
static int 
_chn_recv (int chan_id, int len, char buf[], int nbon) 
{
    HANDLE hPipe = (HANDLE)chan_handle[chan_id];
    DWORD cbBytes = 0;
    BOOL fSuccess;

    error_msg_num[chan_id] = 0;

    if(len >= PIPE_BUFSIZE) {
        error_msg_num[chan_id] = 10;
        return -1;
    }

    if (nbon) {
        DWORD avBytes;
        fSuccess = PeekNamedPipe (hPipe, NULL, 0, NULL, &avBytes, NULL);
        if (!fSuccess) {
            if (GetLastError()==ERROR_IO_PENDING) {
                return cbBytes;
            }
            error_msg_num[chan_id] = 11;
            return -1;
        }
        if (avBytes==0) return 0;
    }
    fSuccess = ReadFile (hPipe, buf, len, &cbBytes, NULL);
    if (fSuccess) {
        return cbBytes;
    } else {
       if (GetLastError()==ERROR_IO_PENDING) {
           return cbBytes;
       }
       error_msg_num[chan_id] = 11;
       return -1;
    }
}

//----------------------------------------------------------------------------
// Revision history:
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//----------------------------------------------------------------------------
#endif
