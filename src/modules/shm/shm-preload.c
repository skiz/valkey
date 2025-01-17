/*
 * module-shm-preload.c
 *
 *  Created on: Nov 9, 2016
 *      Author: Edgars
 */


/* Rewriting a few system functions to avoid duplicating a fluid Redis code.
 * I know this isn't pretty, but it's the next cleanest thing besides forking Redis.
 * Asking users to compile a custom Redis is not nice. The whole point of modules
 * is to avoid such weirdities.
 */

#include "config.h"

#include <dlfcn.h>
#include <stddef.h>

#include "ae.h"

// #ifdef HAVE_EVPORT
// #include <port.h>
// #endif

// #ifdef HAVE_EPOLL
// #include <sys/epoll.h>
// #endif

// #ifdef HAVE_KQUEUE
// #include <sys/event.h>
// #endif

// #include <sys/select.h>


void (*ModuleSHM_BeforeSelect)();
void (*ModuleSHM_AfterSelect)();
ssize_t (*ModuleSHM_ReadUnusual)(int fd, void *buf, size_t count);
ssize_t (*ModuleSHM_WriteUnusual)(int fd, const void *buf, size_t count);


int aePoll(aeEventLoop *eventLoop, struct timeval *tvp) {
    AE_LOCK(eventLoop);
    ModuleSHM_BeforeSelect();
    int ret = aeApiPoll(eventLoop, tvp);
    ModuleSHM_AfterSelect();
    AE_UNLOCK(eventLoop);
    return ret;
}

// int select(int nfds, fd_set *readfds, fd_set *writefds,
//            fd_set *exceptfds, struct timeval *timeout)
// {
//     static int (*real_select)(int nfds, fd_set *readfds, fd_set *writefds,
//                               fd_set *exceptfds, struct timeval *timeout) = NULL;
//     if (real_select == NULL) {
//         real_select = dlsym(RTLD_NEXT, "select");
//     }
    
//     ModuleSHM_BeforeSelect();
//     int res = real_select(nfds, readfds, writefds, exceptfds, timeout);
//     ModuleSHM_AfterSelect();
    
//     return res;
// }

// ssize_t read(int fd, void *buf, size_t count)
// {
//     static int (*real_read)(int fd, void *buf, size_t count) = NULL;
//     if (real_read == NULL) {
//         real_read = dlsym(RTLD_NEXT, "read");
//     }
    
//     if (fd == -1) {
//         return ModuleSHM_ReadUnusual(fd, buf, count);
//     } else {
//         return real_read(fd, buf, count);
//     }
// }

// ssize_t write(int fd, const void *buf, size_t count)
// {
//     static int (*real_write)(int fd, const void *buf, size_t count) = NULL;
//     if (real_write == NULL) {
//         real_write = dlsym(RTLD_NEXT, "write");
//     }
    
//     if (fd == -1) {
//         return ModuleSHM_WriteUnusual(fd, buf, count);
//     } else {
//         return real_write(fd, buf, count);
//     }
// }