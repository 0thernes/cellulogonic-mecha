#ifndef CELLULOGONIC_BARE_CRT_H
#define CELLULOGONIC_BARE_CRT_H

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/*
 * CELLULOGONIC MECHA v1.5.0 — Minimal Zero-Dependency CRT & OS Portability Layer
 * SPDX-License-Identifier: Apache-2.0
 * Compiles cleanly under:
 *   1. Bundled LLVM/Clang targeting x86_64-w64-windows-gnu (no external SDK required)
 *   2. Native GCC / Clang on Linux ELF64 (WSL2 / Ubuntu / standard POSIX)
 *   3. Full MinGW-w64 GCC / Clang toolchains
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#if defined(_WIN32) && defined(CELLULOGONIC_BARE_WIN64)

typedef struct _iobuf FILE;
extern FILE* __cdecl __acrt_iob_func(unsigned index);
#define stdin  (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))

extern int    __cdecl printf(const char* fmt, ...);
extern int    __cdecl fprintf(FILE* stream, const char* fmt, ...);
extern int    __cdecl snprintf(char* buf, size_t n, const char* fmt, ...);
extern int    __cdecl fflush(FILE* stream);
extern FILE*  __cdecl fopen(const char* filename, const char* mode);
extern size_t __cdecl fwrite(const void* ptr, size_t size, size_t count, FILE* stream);
extern size_t __cdecl fread(void* ptr, size_t size, size_t count, FILE* stream);
extern int    __cdecl feof(FILE* stream);
extern int    __cdecl ferror(FILE* stream);
extern char*  __cdecl fgets(char* str, int num, FILE* stream);
extern int    __cdecl fgetc(FILE* stream);
extern int    __cdecl fclose(FILE* stream);
extern int    __cdecl remove(const char* path);
extern int    __cdecl setvbuf(FILE* stream, char* buf, int mode, size_t size);
#ifndef EOF
#define EOF (-1)
#endif
#define _IONBF 0x0004
#define _IOLBF 0x0040

extern void*  __cdecl malloc(size_t size);
extern void*  __cdecl calloc(size_t num, size_t size);
extern void   __cdecl free(void* ptr);
extern long   __cdecl strtol(const char* str, char** endptr, int base);
extern long long __cdecl strtoll(const char* str, char** endptr, int base);
extern unsigned long long __cdecl strtoull(const char* str, char** endptr, int base);

extern void*  __cdecl memcpy(void* dst, const void* src, size_t n);
extern void*  __cdecl memmove(void* dst, const void* src, size_t n);
extern void*  __cdecl memset(void* dst, int c, size_t n);
extern int    __cdecl memcmp(const void* a, const void* b, size_t n);
extern size_t __cdecl strlen(const char* s);
extern int    __cdecl strcmp(const char* a, const char* b);
extern int    __cdecl strncmp(const char* a, const char* b, size_t n);

extern double __cdecl log2(double x);
extern double __cdecl sqrt(double x);
extern double __cdecl fabs(double x);

typedef uint32_t DWORD;
typedef int BOOL;
__declspec(dllimport) BOOL __stdcall QueryPerformanceCounter(int64_t* lpPerformanceCount);
__declspec(dllimport) BOOL __stdcall QueryPerformanceFrequency(int64_t* lpFrequency);

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#endif

#endif /* CELLULOGONIC_BARE_CRT_H */
