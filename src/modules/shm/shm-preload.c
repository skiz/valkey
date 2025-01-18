#include <dlfcn.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

// #define X(...)
#define X printf

// Function pointers for the real system calls
int (*real_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout) = NULL;
ssize_t (*real_read)(int, void *, size_t) = NULL;
ssize_t (*real_write)(int, const void *, size_t) = NULL;

// Function pointers for shared memory interface
void (*ModuleSHM_BeforeSelect)() = NULL;
void (*ModuleSHM_AfterSelect)() = NULL;
ssize_t (*ModuleSHM_ReadUnusual)(int fd, void *buf, size_t count) = NULL;
ssize_t (*ModuleSHM_WriteUnusual)(int fd, const void *buf, size_t count) = NULL;

// Load shared memory functions dynamically
void load_shm_functions() {
    void *handle = dlopen("module-shm.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load module-shm.so: %s\n", dlerror());
        return;
    }

    ModuleSHM_BeforeSelect = dlsym(handle, "ModuleSHM_BeforeSelect_Impl");
    ModuleSHM_AfterSelect = dlsym(handle, "ModuleSHM_AfterSelect_Impl");
    ModuleSHM_ReadUnusual = dlsym(handle, "ModuleSHM_ReadUnusual_Impl");
    ModuleSHM_WriteUnusual = dlsym(handle, "ModuleSHM_WriteUnusual_Impl");
}

// Constructor to initialize function loading
__attribute__((constructor))
void init() {
    real_epoll_wait = dlsym(RTLD_NEXT, "epoll_wait");
    real_read = dlsym(RTLD_NEXT, "read");
    real_write = dlsym(RTLD_NEXT, "write");

    load_shm_functions();
}

// Intercepted epoll_wait function
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
     if (!real_epoll_wait) real_epoll_wait = dlsym(RTLD_NEXT, "epoll_wait");

    if (ModuleSHM_BeforeSelect) ModuleSHM_BeforeSelect();
    
    int res = real_epoll_wait(epfd, events, maxevents, timeout);
    
    if (ModuleSHM_AfterSelect) ModuleSHM_AfterSelect();

    return res;
}

// Intercepted read function
ssize_t read(int fd, void *buf, size_t count) {
    if (!real_read) real_read = dlsym(RTLD_NEXT, "read");

    if (fd == -1 && ModuleSHM_ReadUnusual) {
        X("Intercepted read(fd=%d, count=%zu)\n", fd, count);
        return ModuleSHM_ReadUnusual(fd, buf, count);
    }

    return real_read(fd, buf, count);
}

// Intercepted write function
ssize_t write(int fd, const void *buf, size_t count) {
    if (!real_write) real_write = dlsym(RTLD_NEXT, "write");

    if (fd == -1 && ModuleSHM_WriteUnusual) {
        X("Intercepted write(fd=%d, count=%zu)\n", fd, count);
        return ModuleSHM_WriteUnusual(fd, buf, count);
    }

    return real_write(fd, buf, count);
}
