/*
 * Windows compatibility layer for unistd.h
 * Used to compile libjamesdsp on Windows
 */

#ifndef _WIN_COMPAT_UNISTD_H
#define _WIN_COMPAT_UNISTD_H

#ifdef _WIN32

#include <io.h>
#include <process.h>
#include <windows.h>

// Sleep function (usleep takes microseconds)
static inline int usleep(unsigned int useconds) {
    Sleep(useconds / 1000);
    return 0;
}

static inline unsigned int sleep(unsigned int seconds) {
    Sleep(seconds * 1000);
    return 0;
}

// File access
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0

#define access _access

#else
#include_next <unistd.h>
#endif

#endif /* _WIN_COMPAT_UNISTD_H */
