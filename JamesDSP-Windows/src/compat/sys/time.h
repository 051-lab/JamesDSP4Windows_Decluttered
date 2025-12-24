/*
 * Windows compatibility layer for POSIX headers
 * Used to compile libjamesdsp on Windows
 */

#ifndef _WIN_COMPAT_SYS_TIME_H
#define _WIN_COMPAT_SYS_TIME_H

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>

// timeval is already defined in winsock2.h

static inline int gettimeofday(struct timeval* tp, void* tzp) {
    (void)tzp;
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    
    GetSystemTimeAsFileTime(&ft);
    
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
    
    // Converting file time to unix epoch
    tmpres -= 116444736000000000ULL;
    tmpres /= 10;  // Convert to microseconds
    
    tp->tv_sec = (long)(tmpres / 1000000UL);
    tp->tv_usec = (long)(tmpres % 1000000UL);
    
    return 0;
}

#else
#include_next <sys/time.h>
#endif

#endif /* _WIN_COMPAT_SYS_TIME_H */
