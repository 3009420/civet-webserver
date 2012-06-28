// Copyright (c) 2004-2012 Sergey Lyubka
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef MONGOOSE_SYS_PORTING_INCLUDE
#define MONGOOSE_SYS_PORTING_INCLUDE

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS // Disable deprecation warning in VS2005
#else
#define _XOPEN_SOURCE 600     // For PATH_MAX and flockfile() on Linux
#define _LARGEFILE_SOURCE     // Enable 64-bit file offsets
#endif
#define __STDC_FORMAT_MACROS  // <inttypes.h> wants this for C++
#define __STDC_LIMIT_MACROS   // C++ wants that for INT64_MAX

#if defined(__SYMBIAN32__)
#define NO_SSL // SSL is not supported
#define NO_CGI // CGI is not supported
#define PATH_MAX FILENAME_MAX
#endif // __SYMBIAN32__

#ifndef _WIN32_WCE // Some ANSI #includes are not available on Windows CE
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#endif // !_WIN32_WCE

#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#if defined(_WIN32) && !defined(__SYMBIAN32__) // Windows specific
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 // To make it link in VS200x (with IPv6 support from Win2K onwards, with improved support from Vista onwards)
#endif
// load winSock2 before windows.h or you won't be able to access to IPv6 goodness due to windows.h loading winsock.h (v1):
#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <winsvc.h>

//
// _WIN32_WINNT version constants           <sdkddkver.h>
//
#ifndef _WIN32_WINNT_NT4
#define _WIN32_WINNT_NT4                    0x0400
#endif
#ifndef _WIN32_WINNT_NT4_SP3
#define _WIN32_WINNT_NT4_SP3                0x0403
#endif
#ifndef _WIN32_WINNT_WIN2K
#define _WIN32_WINNT_WIN2K                  0x0500
#endif
#ifndef _WIN32_WINNT_WINXP
#define _WIN32_WINNT_WINXP                  0x0501
#endif
#ifndef _WIN32_WINNT_WS03
#define _WIN32_WINNT_WS03                   0x0502
#endif
#ifndef _WIN32_WINNT_WIN6
#define _WIN32_WINNT_WIN6                   0x0600
#endif
#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA                  0x0600
#endif
#ifndef _WIN32_WINNT_WS08
#define _WIN32_WINNT_WS08                   0x0600
#endif
#ifndef _WIN32_WINNT_LONGHORN
#define _WIN32_WINNT_LONGHORN               0x0600
#endif
#ifndef _WIN32_WINNT_WIN7
#define _WIN32_WINNT_WIN7                   0x0601
#endif

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

#ifndef _WIN32_WCE
#include <process.h>
#include <direct.h>
#include <io.h>
#else // _WIN32_WCE
#define NO_CGI // WinCE has no pipes

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef long off_t;
#define BUFSIZ  4096

#define errno   ((int)GetLastError())
#define strerror(x)  _ultoa(x, (char *) _alloca(sizeof(x) *3 ), 10)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _WIN32_WCE

#define MAKEUQUAD(lo, hi) ((uint64_t)(((uint32_t)(lo)) | \
      ((uint64_t)((uint32_t)(hi))) << 32))
#define RATE_DIFF 10000000 // 100 nsecs
#define EPOCH_DIFF MAKEUQUAD(0xd53e8000, 0x019db1de)
#define SYS2UNIX_TIME(lo, hi) \
  (time_t) ((MAKEUQUAD((lo), (hi)) - EPOCH_DIFF) / RATE_DIFF)

// Visual Studio 6 does not know __func__ or __FUNCTION__
// The rest of MS compilers use __FUNCTION__, not C99 __func__
// Also use _strtoui64 on modern M$ compilers
#if defined(_MSC_VER)
#if _MSC_VER < 1300
#define STRX(x) #x
#define STR(x) STRX(x)
#define __func__ "line " STR(__LINE__)
#define strtoull(x, y, z) strtoul(x, y, z)
#define strtoll(x, y, z) strtol(x, y, z)
#else
#define __func__  __FUNCTION__
#define strtoull(x, y, z) _strtoui64(x, y, z)
#define strtoll(x, y, z) _strtoi64(x, y, z)
#endif // _MSC_VER
#endif // _MSC_VER

#define ERRNO   mgW32_get_errno()
#if !(defined(_MSC_VER) && (_MSC_VER >= 1600) && defined(_WINSOCK2API_)) // Microsoft: socklen_t exists in ws2tcpip.h in Windows SDK 7.0A+
#define NO_SOCKLEN_T
#endif
#define SSL_LIB   "ssleay32.dll"
#define CRYPTO_LIB  "libeay32.dll"
#define DIRSEP '\\'
#define IS_DIRSEP_CHAR(c) ((c) == '/' || (c) == '\\')
#ifndef PATH_MAX // MingW fix
#define PATH_MAX MAX_PATH
#endif
#ifndef S_ISDIR // MingW fix
#define S_ISDIR(x) ((x) & _S_IFDIR)
#endif
#define O_NONBLOCK  0

#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK  WSAEWOULDBLOCK
#endif // !EWOULDBLOCK

#ifndef ETIMEOUT
#define ETIMEOUT    WSAETIMEDOUT
#endif

#define _POSIX_

// make sure code which hasn't been properly migrated, crashes at compile time:
#define INT64_FMT  ++++"do not use; use the C99 format define(s) instead, e.g. PRId64"++++

/*
Since MSVC and MingW GCC don't have inttypes.h, we provide those printf/scanf formats here:

GCC inttypes.h says:
   "The ISO C99 standard specifies that these macros must only be
   defined if explicitly requested."
   
We also check whether someone else has gone before us setting up these C99 defines.
*/
#if (!defined __cplusplus || defined __STDC_FORMAT_MACROS) && !defined(PRId64)

#define __PRI64_PREFIX	"I64"

/* Macros for printing format specifiers.  */

/* Decimal notation.  */
#define PRId8		"d"
#define PRId16		"d"
#define PRId32		"d"
#define PRId64		__PRI64_PREFIX "d"

#define PRIdLEAST8	"d"
#define PRIdLEAST16	"d"
#define PRIdLEAST32	"d"
#define PRIdLEAST64	__PRI64_PREFIX "d"

#define PRIdFAST8	"d"
#define PRIdFAST16	"d"
#define PRIdFAST32	"d"
#define PRIdFAST64	__PRI64_PREFIX "d"


#define PRIi8		"i"
#define PRIi16		"i"
#define PRIi32		"i"
#define PRIi64		__PRI64_PREFIX "i"

#define PRIiLEAST8	"i"
#define PRIiLEAST16	"i"
#define PRIiLEAST32	"i"
#define PRIiLEAST64	__PRI64_PREFIX "i"

#define PRIiFAST8	"i"
#define PRIiFAST16	"i"
#define PRIiFAST32	"i"
#define PRIiFAST64	__PRI64_PREFIX "i"

/* Octal notation.  */
#define PRIo8		"o"
#define PRIo16		"o"
#define PRIo32		"o"
#define PRIo64		__PRI64_PREFIX "o"

#define PRIoLEAST8	"o"
#define PRIoLEAST16	"o"
#define PRIoLEAST32	"o"
#define PRIoLEAST64	__PRI64_PREFIX "o"

#define PRIoFAST8	"o"
#define PRIoFAST16	"o"
#define PRIoFAST32	"o"
#define PRIoFAST64	__PRI64_PREFIX "o"

/* Unsigned integers.  */
#define PRIu8		"u"
#define PRIu16		"u"
#define PRIu32		"u"
#define PRIu64		__PRI64_PREFIX "u"

#define PRIuLEAST8	"u"
#define PRIuLEAST16	"u"
#define PRIuLEAST32	"u"
#define PRIuLEAST64	__PRI64_PREFIX "u"

#define PRIuFAST8	"u"
#define PRIuFAST16	"u"
#define PRIuFAST32	"u"
#define PRIuFAST64	__PRI64_PREFIX "u"

/* lowercase hexadecimal notation.  */
#define PRIx8		"x"
#define PRIx16		"x"
#define PRIx32		"x"
#define PRIx64		__PRI64_PREFIX "x"

#define PRIxLEAST8	"x"
#define PRIxLEAST16	"x"
#define PRIxLEAST32	"x"
#define PRIxLEAST64	__PRI64_PREFIX "x"

#define PRIxFAST8	"x"
#define PRIxFAST16	"x"
#define PRIxFAST32	"x"
#define PRIxFAST64	__PRI64_PREFIX "x"

/* UPPERCASE hexadecimal notation.  */
#define PRIX8		"X"
#define PRIX16		"X"
#define PRIX32		"X"
#define PRIX64		__PRI64_PREFIX "X"

#define PRIXLEAST8	"X"
#define PRIXLEAST16	"X"
#define PRIXLEAST32	"X"
#define PRIXLEAST64	__PRI64_PREFIX "X"

#define PRIXFAST8	"X"
#define PRIXFAST16	"X"
#define PRIXFAST32	"X"
#define PRIXFAST64	__PRI64_PREFIX "X"


/* Macros for printing `intmax_t' and `uintmax_t'.  */
#define PRIdMAX	__PRI64_PREFIX "d"
#define PRIiMAX	__PRI64_PREFIX "i"
#define PRIoMAX	__PRI64_PREFIX "o"
#define PRIuMAX	__PRI64_PREFIX "u"
#define PRIxMAX	__PRI64_PREFIX "x"
#define PRIXMAX	__PRI64_PREFIX "X"

/* Macros for printing `intptr_t' and `uintptr_t'.  */
#define PRIdPTR	__PRI64_PREFIX "d"
#define PRIiPTR	__PRI64_PREFIX "i"
#define PRIoPTR	__PRI64_PREFIX "o"
#define PRIuPTR	__PRI64_PREFIX "u"
#define PRIxPTR	__PRI64_PREFIX "x"
#define PRIXPTR	__PRI64_PREFIX "X"


/* Macros for scanning format specifiers.  */

/* Signed decimal notation.  */
//#define SCNd8		"hhd"
#define SCNd16		"hd"
#define SCNd32		"d"
#define SCNd64		__PRI64_PREFIX "d"

//#define SCNdLEAST8	"hhd"
#define SCNdLEAST16	"hd"
#define SCNdLEAST32	"d"
#define SCNdLEAST64	__PRI64_PREFIX "d"

//#define SCNdFAST8	"hhd"
#define SCNdFAST16	"d"
#define SCNdFAST32	"d"
#define SCNdFAST64	__PRI64_PREFIX "d"

/* Signed decimal notation.  */
//#define SCNi8		"hhi"
#define SCNi16		"hi"
#define SCNi32		"i"
#define SCNi64		__PRI64_PREFIX "i"

//#define SCNiLEAST8	"hhi"
#define SCNiLEAST16	"hi"
#define SCNiLEAST32	"i"
#define SCNiLEAST64	__PRI64_PREFIX "i"

//#define SCNiFAST8	"hhi"
#define SCNiFAST16	"i"
#define SCNiFAST32	"i"
#define SCNiFAST64	__PRI64_PREFIX "i"

/* Unsigned decimal notation.  */
//#define SCNu8		"hhu"
#define SCNu16		"hu"
#define SCNu32		"u"
#define SCNu64		__PRI64_PREFIX "u"

//#define SCNuLEAST8	"hhu"
#define SCNuLEAST16	"hu"
#define SCNuLEAST32	"u"
#define SCNuLEAST64	__PRI64_PREFIX "u"

//#define SCNuFAST8	"hhu"
#define SCNuFAST16	"u"
#define SCNuFAST32	"u"
#define SCNuFAST64	__PRI64_PREFIX "u"

/* Octal notation.  */
//#define SCNo8		"hho"
#define SCNo16		"ho"
#define SCNo32		"o"
#define SCNo64		__PRI64_PREFIX "o"

//#define SCNoLEAST8	"hho"
#define SCNoLEAST16	"ho"
#define SCNoLEAST32	"o"
#define SCNoLEAST64	__PRI64_PREFIX "o"

//#define SCNoFAST8	"hho"
#define SCNoFAST16	"o"
#define SCNoFAST32	"o"
#define SCNoFAST64	__PRI64_PREFIX "o"

/* Hexadecimal notation.  */
//#define SCNx8		"hhx"
#define SCNx16		"hx"
#define SCNx32		"x"
#define SCNx64		__PRI64_PREFIX "x"

//#define SCNxLEAST8	"hhx"
#define SCNxLEAST16	"hx"
#define SCNxLEAST32	"x"
#define SCNxLEAST64	__PRI64_PREFIX "x"

//#define SCNxFAST8	"hhx"
#define SCNxFAST16	"x"
#define SCNxFAST32	"x"
#define SCNxFAST64	__PRI64_PREFIX "x"


/* Macros for scanning `intmax_t' and `uintmax_t'.  */
#define SCNdMAX	__PRI64_PREFIX "d"
#define SCNiMAX	__PRI64_PREFIX "i"
#define SCNoMAX	__PRI64_PREFIX "o"
#define SCNuMAX	__PRI64_PREFIX "u"
#define SCNxMAX	__PRI64_PREFIX "x"

/* Macros for scanning `intptr_t' and `uintptr_t'.  */
#define SCNdPTR	__PRI64_PREFIX "d"
#define SCNiPTR	__PRI64_PREFIX "i"
#define SCNoPTR	__PRI64_PREFIX "o"
#define SCNuPTR	__PRI64_PREFIX "u"
#define SCNxPTR	__PRI64_PREFIX "x"

#endif	/* C++ && format macros */

// See also: http://stackoverflow.com/questions/2354784/attribute-formatprintf-1-2-for-msvc/6849629#6849629
#undef FORMAT_STRING
#if _MSC_VER >= 1400
# include <sal.h>
# if _MSC_VER > 1400
#  define FORMAT_STRING(p) _Printf_format_string_ p
# else
# define FORMAT_STRING(p) __format_string p
# endif /* FORMAT_STRING */
#else
# define FORMAT_STRING(p) p
#endif /* _MSC_VER */

#define WINCDECL __cdecl

#ifndef SHUT_WR
#define SHUT_WR   SD_SEND
#define SHUT_RD   SD_RECEIVE
#define SHUT_RDWR SD_BOTH
#endif

#if !defined(IPV6_V6ONLY) && (_WIN32_WINNT >= 0x0501)
#define IPV6_V6ONLY           27 // Treat wildcard bind as AF_INET6-only.
#endif

#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define mg_sleep(x) Sleep(x)

#define pipe(x) _pipe(x, BUFSIZ, _O_BINARY | _O_NOINHERIT)
#define popen(x, y) _popen(x, y)
#define pclose(x) _pclose(x)
#define close(x) _close(x)
#define dlsym(x,y) GetProcAddress((HINSTANCE) (x), (y))
#define RTLD_LAZY  0
#define fseeko(x, y, z) fseek((x), (y), (z))
#if !defined(_POSIX_)
#define fdopen(x, y) _fdopen((x), (y))
#endif
#define write(x, y, z) _write((x), (y), (unsigned) z)
#define read(x, y, z) _read((x), (y), (unsigned) z)

#if (NTDDI_VERSION >= NTDDI_VISTA)
  // Only Windoze Vista (and newer) have inet_ntop(); MingW doesn't seem to provide it though
  #if !defined(__MINGW32__) && !defined(__MINGW64__) // http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers#MinGW_and_MinGW-w64
    #define HAVE_INET_NTOP
  #endif
#endif
#if defined(_WIN32_WINNT) && defined(_WIN32_WINNT_WINXP) && (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)
  #define HAVE_GETNAMEINFO
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int mgW32_get_errno(void);

void mgW32_flockfile(FILE *x);
void mgW32_funlockfile(FILE *x);

#ifdef __cplusplus
}
#endif // __cplusplus

#define flockfile mgW32_flockfile
#define funlockfile mgW32_funlockfile

#if !defined(fileno) && (!defined(_POSIX_) || defined(_fileno))
#define fileno(x) _fileno(x)
#endif // !fileno MINGW #defines fileno; so does MSVC when in _POSIX_ mode, but _fileno is a faster(?) macro there.

#if !defined(HAVE_PTHREAD)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef HANDLE pthread_mutex_t;
typedef struct {HANDLE signal, broadcast;} pthread_cond_t;
typedef DWORD pthread_t;

struct timespec {
  long tv_nsec;
  long tv_sec;
};

#ifdef __cplusplus
}
#endif // __cplusplus

#else

#include <pthread.h>

#endif

#define pid_t HANDLE // MINGW typedefs pid_t to int. Using #define here.  It also overrides the pid_t typedef in pthread.h (--> sched.h) for pthread-win32




#if defined(HAVE_STDINT) || (defined(_MSC_VER) && _MSC_VER >= 1500)
#include <stdint.h>
#else
typedef unsigned int  uint32_t;
typedef unsigned char  uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned __int64 uint64_t;
typedef __int64   int64_t;
#define INT64_MAX  9223372036854775807LL
#endif // HAVE_STDINT

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// POSIX dirent interface
struct dirent {
  char d_name[PATH_MAX];
};

typedef struct DIR {
  HANDLE   handle;
  WIN32_FIND_DATAW info;
  struct dirent  result;
} DIR;

int mg_rename(const char* oldname, const char* newname);
int mg_remove(const char *path);
int mg_mkdir(const char *path, int mode);

#if defined(__MINGW32__) || defined(__MINGW64__) || 1
// fixing the 'implicit declaration' warnings as the MingW headers aren't up to snuff:
#if defined(_DLL) && !defined(_CRTIMP)
#define _CRTIMP __declspec(dllimport)
#endif
_CRTIMP int __cdecl fileno(FILE * _File);
_CRTIMP FILE * __cdecl fdopen(int _FileHandle, const char * _Mode);
_CRTIMP int __cdecl _pclose(FILE * _File);
_CRTIMP FILE * __cdecl _popen(const char * _Command, const char * _Mode);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#else    // UNIX  specific

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdint.h>
#include <inttypes.h>
#include <netdb.h>

#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#if !defined(NO_SSL_DL) && !defined(NO_SSL)
#include <dlfcn.h>
#endif
#include <pthread.h>
#if defined(__MACH__)
#define SSL_LIB   "libssl.dylib"
#define CRYPTO_LIB  "libcrypto.dylib"
#else
#if !defined(SSL_LIB)
#define SSL_LIB   "libssl.so"
#endif
#if !defined(CRYPTO_LIB)
#define CRYPTO_LIB  "libcrypto.so"
#endif
#endif
#define DIRSEP   '/'
#define IS_DIRSEP_CHAR(c) ((c) == '/')
#ifndef O_BINARY
#define O_BINARY  0
#endif // O_BINARY
#define closesocket(a) close(a)
//#define mg_fopen(x, y) fopen(x, y)
#define mg_mkdir(x, y) mkdir(x, y)
#define mg_remove(x) remove(x)
#define mg_rename(x, y) rename(x, y)
#define mg_sleep(x) usleep((x) * 1000)
#define ERRNO errno
#define INVALID_SOCKET (-1)
typedef int SOCKET;
#define WINCDECL

// for now assume all UNIXes have inet_ntop / inet_pton when they have IPv6, otherwise they always have getnameinfo()
#if defined(USE_IPV6)
  #define HAVE_INET_NTOP
#endif
#define HAVE_GETNAMEINFO

#endif // End of Windows and UNIX specific includes

#ifndef FORMAT_STRING
# define FORMAT_STRING(p) p
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define MG_MAX(a, b)      ((a) >= (b) ? (a) : (b))

/*
 * The following VA_COPY was coded following an example in
 * the Samba project.  It may not be sufficient for some
 * esoteric implementations of va_list (i.e. it may need
 * something involving a memcpy) but (hopefully) will be
 * sufficient for mongoose (code taken from libxml2 and augmented).
 */
#ifndef VA_COPY
  #if defined(HAVE_VA_COPY) || defined(va_copy) /* Linux stdarg.h 'regular' flavor */
    #define VA_COPY(dest, src) va_copy(dest, src)
  #else
    #if defined(HAVE___VA_COPY) || defined(__va_copy)  /* Linux stdarg.h 'strict ANSI' flavor */
      #define VA_COPY(dest,src) __va_copy(dest, src)
    #else
      #define VA_COPY(dest,src) do { (dest) = (src); } while (0) /* MSVC: doesn't offer va_copy at all. */
    #endif
  #endif
#endif


/* <bel>: Local fix for some linux SDK headers that do not know these options */
#ifndef SOMAXCONN
#define SOMAXCONN 128
#endif
/* <bel>: end fix */

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif


#if defined(DEBUG) || defined(_DEBUG)
#if defined(PTW32_VERSION)
#define MG_PTHREAD_SELF()   pthread_self().p
#else
#define MG_PTHREAD_SELF()   (void *)pthread_self()
#endif
#define DEBUG_TRACE(x) do {                                 \
  flockfile(stdout);                                        \
  printf("*** %lu.%p.%s.%d: ",                              \
         (unsigned long) time(NULL), MG_PTHREAD_SELF(),     \
         __func__, __LINE__);                               \
  printf x;                                                 \
  putchar('\n');                                            \
  fflush(stdout);                                           \
  funlockfile(stdout);                                      \
} while (0)
#else
#define DEBUG_TRACE(x)
#endif // DEBUG

// Darwin prior to 7.0 and Win32 do not have socklen_t
#ifdef NO_SOCKLEN_T
typedef int socklen_t;
#endif // NO_SOCKLEN_T

/* buffer size that will fit both IPv4 and IPv6 addresses formatted by ntoa() / ntop() */
#define SOCKADDR_NTOA_BUFSIZE           42

/* buffer size used when copying data to/from file/socket/... */
#define DATA_COPY_BUFSIZ                MG_MAX(BUFSIZ, 4096)
/* buffer size used to load all HTTP headers into: if the client sends more header data than this, we'll barf a hairball! */
#define HTTP_HEADERS_BUFSIZ             MG_MAX(BUFSIZ, 2048)
/* buffer size used to extract/decode an SSI command line / file path; hence must be equal or larger than PATH_MAX, at least */
#define SSI_LINE_BUFSIZ                 MG_MAX(BUFSIZ, PATH_MAX)




#if defined(_WIN32) && !defined(__SYMBIAN32__)

#if !defined(HAVE_PTHREAD)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * POSIX pthread features support:
 */
#undef _POSIX_THREADS

#undef _POSIX_READER_WRITER_LOCKS
#define _POSIX_READER_WRITER_LOCKS  200809L

#undef _POSIX_SPIN_LOCKS
#define _POSIX_SPIN_LOCKS           200809L

#undef _POSIX_BARRIERS

#undef _POSIX_THREAD_SAFE_FUNCTIONS

#undef _POSIX_THREAD_ATTR_STACKSIZE

int pthread_mutex_init(pthread_mutex_t *mutex, void *unused);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_cond_init(pthread_cond_t *cv, const void *unused);
int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cv, pthread_mutex_t *mutex, const struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *cv);
int pthread_cond_broadcast(pthread_cond_t *cv);
int pthread_cond_destroy(pthread_cond_t *cv);
pthread_t pthread_self(void);

typedef struct {
    unsigned rw: 1;
#if defined(RTL_SRWLOCK_INIT) // Windows 7 / Server 2008 with the correct header files, i.e. this also 'fixes' MingW casualties
    SRWLOCK lock;
#else
    pthread_mutex_t mutex;
#endif
} pthread_rwlock_t;

typedef void pthread_rwlockattr_t;

int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);

#if defined(RTL_SRWLOCK_INIT) // Windows 7 / Server 2008 with the correct header files, i.e. this also 'fixes' MingW casualties
#define PTHREAD_RWLOCK_INITIALIZER          { 0, RTL_SRWLOCK_INIT }
#else
#define PTHREAD_RWLOCK_INITIALIZER          { 0 }
#endif

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

//#define PTHREAD_SPINLOCK_INITIALIZER      PTHREAD_MUTEX_INITIALIZER

typedef pthread_mutex_t pthread_spinlock_t;

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *lock);
int pthread_spin_lock(pthread_spinlock_t *lock);
//int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif


#if defined(_WIN32_WCE)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

time_t time(time_t *ptime);
struct tm *localtime(const time_t *ptime, struct tm *ptm);
struct tm *gmtime(const time_t *ptime, struct tm *ptm);
static size_t strftime(char *dst, size_t dst_size, const char *fmt,
                       const struct tm *tm);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

#endif // _WIN32 -- for pthread and time lib support


#undef UNUSED_PARAMETER
#if defined(__GNUC__)
#define UNUSED_PARAMETER(p)     p __attribute__((unused))
#else
#define UNUSED_PARAMETER(p)     p
#endif






#endif // MONGOOSE_SYS_PORTING_INCLUDE

