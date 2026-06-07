/*══════════════════════════════════════════════════════════════════════════════
 * Debug/_winstub/windows.h — Linux test-harness stub for <windows.h>.
 *
 *   The portable g++ tests in this directory exercise the parts of
 *   Elle's code that don't depend on Windows runtime APIs (algorithms,
 *   parsers, offline NDJSON queue file format, JWT/HMAC, embeddings,
 *   etc).  Several of the headers they reach through (ElleTypes.h,
 *   ElleConfig.h, ElleSQLConn.h…) include <windows.h> unconditionally
 *   because the production build is always on Windows.
 *
 *   This file provides JUST ENOUGH of the Win32 surface so a test can
 *   syntax-check the production header on Linux without a real Windows
 *   SDK.  It is NEVER linked into a Windows build — the real SDK header
 *   takes precedence whenever `_WIN32` is defined.
 *
 *   Use:    g++ -I Debug/_winstub Debug/test_xxx.cpp ...
 *══════════════════════════════════════════════════════════════════════════════*/
#pragma once
#ifndef _WINDOWS_LINUX_TEST_STUB_
#define _WINDOWS_LINUX_TEST_STUB_

#ifdef _WIN32
#  error "Debug/_winstub/windows.h must NEVER be reached on a real Windows build"
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>

/*──────────────────────────────────────────────────────────────────────────────
 * Sized integer typedefs
 *──────────────────────────────────────────────────────────────────────────────*/
typedef uint8_t   BYTE;
typedef uint16_t  WORD;
typedef uint32_t  DWORD;
typedef uint64_t  QWORD;
typedef int32_t   LONG;
typedef uint32_t  ULONG;
typedef int64_t   LONGLONG;
typedef uint64_t  ULONGLONG;
typedef int       BOOL;
typedef int       INT;
typedef unsigned  UINT;
typedef wchar_t   WCHAR;
typedef char      CHAR;
typedef CHAR      TCHAR;
typedef CHAR*     LPSTR;
typedef const CHAR*  LPCSTR;
typedef WCHAR*    LPWSTR;
typedef const WCHAR* LPCWSTR;
typedef CHAR*     LPTSTR;
typedef const CHAR* LPCTSTR;
typedef void*     LPVOID;
typedef const void* LPCVOID;
typedef DWORD*    LPDWORD;
typedef WORD*     LPWORD;
typedef BYTE*     LPBYTE;
typedef BOOL*     LPBOOL;
typedef void*     HANDLE;
typedef HANDLE*   PHANDLE;
typedef HANDLE    HMODULE;
typedef HANDLE    HINSTANCE;
typedef HANDLE    HWND;
typedef HANDLE    HKEY;
typedef HANDLE    HLOCAL;
typedef HANDLE    SC_HANDLE;
typedef HANDLE    SERVICE_STATUS_HANDLE;
typedef intptr_t  INT_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t  LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;
typedef int64_t   SSIZE_T;
typedef DWORD     SOCKET;

#ifndef TRUE
#  define TRUE  1
#endif
#ifndef FALSE
#  define FALSE 0
#endif
#ifndef NULL
#  define NULL nullptr
#endif

#define MAX_PATH                 260
#define INFINITE                 0xFFFFFFFFu
#define INVALID_HANDLE_VALUE     ((HANDLE)(LONG_PTR)-1)
#define INVALID_SOCKET           (~(SOCKET)0)
#define WAIT_OBJECT_0            0x00000000
#define WAIT_TIMEOUT             0x00000102
#define WAIT_FAILED              0xFFFFFFFFu
#define ERROR_SUCCESS            0L
#define NO_ERROR                 0L
#define ERROR_ALREADY_EXISTS     183
#define ERROR_FILE_NOT_FOUND     2
#define ERROR_ACCESS_DENIED      5
#define MOVEFILE_REPLACE_EXISTING 0x1
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT             0x0601
#define WINAPI                   /* empty on Linux */
#define APIENTRY                 /* empty on Linux */
#define CALLBACK                 /* empty on Linux */
#ifndef __stdcall
#define __stdcall                /* empty on Linux */
#endif
#ifndef __cdecl
#define __cdecl                  /* empty on Linux */
#endif
#ifndef __fastcall
#define __fastcall               /* empty on Linux */
#endif

/*──────────────────────────────────────────────────────────────────────────────
 * Aggregate types — a few struct/union definitions referenced by the
 * production code.  Kept minimal: the tests don't exercise them, the
 * compiler just needs the layout to be syntactically valid.
 *──────────────────────────────────────────────────────────────────────────────*/
typedef union _LARGE_INTEGER {
    struct { DWORD LowPart; LONG HighPart; } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
    struct { DWORD LowPart; DWORD HighPart; } u;
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct { DWORD Offset; DWORD OffsetHigh; } off;
        void* Pointer;
    } u;
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD  nLength;
    void*  lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay;
    WORD wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;

/*──────────────────────────────────────────────────────────────────────────────
 * Service Control Manager — only the surface used by ElleServiceBase.
 *──────────────────────────────────────────────────────────────────────────────*/
typedef struct _SERVICE_STATUS {
    DWORD dwServiceType;
    DWORD dwCurrentState;
    DWORD dwControlsAccepted;
    DWORD dwWin32ExitCode;
    DWORD dwServiceSpecificExitCode;
    DWORD dwCheckPoint;
    DWORD dwWaitHint;
} SERVICE_STATUS, *LPSERVICE_STATUS;

#define SERVICE_WIN32_OWN_PROCESS  0x00000010
#define SERVICE_INTERACTIVE_PROCESS 0x00000100
#define SERVICE_STOPPED            0x00000001
#define SERVICE_START_PENDING      0x00000002
#define SERVICE_STOP_PENDING       0x00000003
#define SERVICE_RUNNING            0x00000004
#define SERVICE_CONTINUE_PENDING   0x00000005
#define SERVICE_PAUSE_PENDING      0x00000006
#define SERVICE_PAUSED             0x00000007
#define SERVICE_ACCEPT_STOP        0x00000001
#define SERVICE_ACCEPT_PAUSE_CONTINUE 0x00000002
#define SERVICE_ACCEPT_SHUTDOWN    0x00000004
#define SERVICE_CONTROL_STOP       0x00000001
#define SERVICE_CONTROL_PAUSE      0x00000002
#define SERVICE_CONTROL_CONTINUE   0x00000003
#define SERVICE_CONTROL_INTERROGATE 0x00000004
#define SERVICE_CONTROL_SHUTDOWN   0x00000005
#define SERVICE_AUTO_START         0x00000002
#define SERVICE_DEMAND_START       0x00000003
#define SERVICE_ERROR_NORMAL       0x00000001
#define SERVICE_ERROR_IGNORE       0x00000000
#define SERVICE_QUERY_STATUS       0x0004
#define SERVICE_STOP               0x0020
#define SERVICE_ALL_ACCESS         0xF01FF
#define SC_MANAGER_ALL_ACCESS      0xF003F
#define SC_MANAGER_CONNECT         0x0001

typedef void (WINAPI *LPSERVICE_MAIN_FUNCTIONA)(DWORD, LPSTR*);
typedef void (WINAPI *LPHANDLER_FUNCTION)(DWORD);
typedef struct _SERVICE_TABLE_ENTRYA {
    LPSTR lpServiceName;
    LPSERVICE_MAIN_FUNCTIONA lpServiceProc;
} SERVICE_TABLE_ENTRYA, *LPSERVICE_TABLE_ENTRYA;

inline BOOL StartServiceCtrlDispatcherA(const SERVICE_TABLE_ENTRYA*) { return TRUE; }
inline SERVICE_STATUS_HANDLE RegisterServiceCtrlHandlerA(LPCSTR, LPHANDLER_FUNCTION) {
    return (SERVICE_STATUS_HANDLE)(LONG_PTR)1;
}
inline BOOL SetServiceStatus(SERVICE_STATUS_HANDLE, LPSERVICE_STATUS) { return TRUE; }
inline SC_HANDLE OpenSCManagerA(LPCSTR, LPCSTR, DWORD) { return (SC_HANDLE)(LONG_PTR)1; }
inline SC_HANDLE OpenServiceA(SC_HANDLE, LPCSTR, DWORD) { return (SC_HANDLE)(LONG_PTR)1; }
inline SC_HANDLE CreateServiceA(SC_HANDLE, LPCSTR, LPCSTR, DWORD, DWORD, DWORD, DWORD,
                                LPCSTR, LPCSTR, LPDWORD, LPCSTR, LPCSTR, LPCSTR) {
    return (SC_HANDLE)(LONG_PTR)1;
}
inline BOOL CloseServiceHandle(SC_HANDLE) { return TRUE; }
inline BOOL DeleteService(SC_HANDLE) { return TRUE; }
inline BOOL StartServiceA(SC_HANDLE, DWORD, LPCSTR*) { return TRUE; }
inline BOOL ControlService(SC_HANDLE, DWORD, LPSERVICE_STATUS) { return TRUE; }
inline BOOL QueryServiceStatus(SC_HANDLE, LPSERVICE_STATUS) { return TRUE; }

/*──────────────────────────────────────────────────────────────────────────────
 * Stub function decls — bodies live inline so a test never needs to link
 * a real Win32 import library.  Behaviour mirrors a "no-op success" — the
 * tests that call these intentionally don't depend on real return data.
 *──────────────────────────────────────────────────────────────────────────────*/
inline DWORD GetModuleFileNameA(HMODULE, char* buf, DWORD sz) {
    if (sz) buf[0] = '\0';
    return 0;
}
inline DWORD GetModuleFileNameW(HMODULE, wchar_t* buf, DWORD sz) {
    if (sz) buf[0] = L'\0';
    return 0;
}
inline BOOL  CreateDirectoryA(LPCSTR, void*) { return TRUE; }
inline BOOL  RemoveDirectoryA(LPCSTR) { return TRUE; }
inline BOOL  MoveFileExA(LPCSTR, LPCSTR, DWORD) { return TRUE; }
inline BOOL  MoveFileA(LPCSTR, LPCSTR) { return TRUE; }
inline BOOL  DeleteFileA(LPCSTR) { return TRUE; }
inline BOOL  CopyFileA(LPCSTR, LPCSTR, BOOL) { return TRUE; }
inline BOOL  SetCurrentDirectoryA(LPCSTR) { return TRUE; }
inline DWORD GetCurrentDirectoryA(DWORD sz, char* buf) {
    if (sz) buf[0] = '\0';
    return 0;
}
inline DWORD GetLastError(void) { return 0; }
inline void  SetLastError(DWORD) {}
inline void  Sleep(DWORD ms) { ::usleep((useconds_t)ms * 1000); }
inline DWORD GetTickCount(void) {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)(ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL);
}
inline ULONGLONG GetTickCount64(void) {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ULONGLONG)ts.tv_sec * 1000ULL + (ULONGLONG)(ts.tv_nsec / 1000000ULL);
}
inline DWORD WaitForSingleObject(HANDLE, DWORD) { return WAIT_OBJECT_0; }
inline BOOL  CloseHandle(HANDLE) { return TRUE; }
inline HANDLE GetCurrentProcess(void) { return (HANDLE)(LONG_PTR)-1; }
inline DWORD GetCurrentProcessId(void) { return (DWORD)::getpid(); }
inline DWORD GetCurrentThreadId(void) { return (DWORD)::getpid(); }

/*──────────────────────────────────────────────────────────────────────────────
 * Memory + IPC + threading symbols referenced by ElleQueueIPC.cpp
 *──────────────────────────────────────────────────────────────────────────────*/
inline void  ZeroMemory(void* dst, size_t n) { ::memset(dst, 0, n); }
inline HMODULE LoadLibraryA(LPCSTR) { return (HMODULE)(LONG_PTR)0; }
inline BOOL    FreeLibrary(HMODULE) { return TRUE; }
/* On real Win32, FARPROC is a calling-convention-tagged function ptr
 * which the SDK casts to user-defined fn_t types without warning.  GCC
 * flags that with -Wcast-function-type — silence it locally so the
 * portable harness doesn't false-positive on a Win32-idiomatic cast. */
#pragma GCC diagnostic ignored "-Wcast-function-type"
typedef intptr_t (*FARPROC)();
inline FARPROC GetProcAddress(HMODULE, LPCSTR) { return nullptr; }
inline void* RtlSecureZeroMemory(void* dst, size_t n) { ::memset(dst, 0, n); return dst; }
inline void  CopyMemory(void* dst, const void* src, size_t n) { ::memcpy(dst, src, n); }
inline BOOL  DisconnectNamedPipe(HANDLE) { return TRUE; }
inline HANDLE CreateNamedPipeA(LPCSTR, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, void*) { return INVALID_HANDLE_VALUE; }
inline HANDLE CreateFileA(LPCSTR, DWORD, DWORD, void*, DWORD, DWORD, HANDLE) { return INVALID_HANDLE_VALUE; }
inline BOOL  ConnectNamedPipe(HANDLE, LPOVERLAPPED) { return FALSE; }
inline BOOL  ReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED) { return FALSE; }
inline BOOL  WriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED) { return FALSE; }
inline BOOL  CancelIo(HANDLE) { return TRUE; }
inline BOOL  CancelIoEx(HANDLE, LPOVERLAPPED) { return TRUE; }
inline BOOL  GetOverlappedResult(HANDLE, LPOVERLAPPED, LPDWORD, BOOL) { return FALSE; }
inline HANDLE CreateIoCompletionPort(HANDLE, HANDLE, ULONG_PTR, DWORD) { return INVALID_HANDLE_VALUE; }
inline BOOL  GetQueuedCompletionStatus(HANDLE, LPDWORD, ULONG_PTR*, LPOVERLAPPED*, DWORD) { return FALSE; }
inline BOOL  PostQueuedCompletionStatus(HANDLE, DWORD, ULONG_PTR, LPOVERLAPPED) { return FALSE; }
inline HANDLE CreateEventA(void*, BOOL, BOOL, LPCSTR) { return INVALID_HANDLE_VALUE; }
inline BOOL  SetEvent(HANDLE) { return TRUE; }
inline BOOL  ResetEvent(HANDLE) { return TRUE; }
inline BOOL  WaitNamedPipeA(LPCSTR, DWORD) { return FALSE; }
inline BOOL  SetNamedPipeHandleState(HANDLE, LPDWORD, LPDWORD, LPDWORD) { return TRUE; }

/* Named-pipe constants */
#define PIPE_ACCESS_INBOUND    0x00000001
#define PIPE_ACCESS_OUTBOUND   0x00000002
#define PIPE_ACCESS_DUPLEX     0x00000003
#define FILE_FLAG_OVERLAPPED   0x40000000
#define FILE_FLAG_FIRST_PIPE_INSTANCE 0x00080000
#define PIPE_WAIT              0x00000000
#define PIPE_NOWAIT            0x00000001
#define PIPE_READMODE_BYTE     0x00000000
#define PIPE_READMODE_MESSAGE  0x00000002
#define PIPE_TYPE_BYTE         0x00000000
#define PIPE_TYPE_MESSAGE      0x00000004
#define PIPE_UNLIMITED_INSTANCES 255
#define PIPE_REJECT_REMOTE_CLIENTS 0x00000008

#define GENERIC_READ           0x80000000u
#define GENERIC_WRITE          0x40000000u
#define OPEN_EXISTING          3
#define OPEN_ALWAYS            4
#define CREATE_ALWAYS          2
#define CREATE_NEW             1
#define INVALID_FILE_SIZE      0xFFFFFFFFu
#define ERROR_PIPE_CONNECTED   535
#define ERROR_IO_PENDING       997
#define ERROR_BROKEN_PIPE      109
#define ERROR_NO_DATA          232
#define ERROR_MORE_DATA        234
#define ERROR_PIPE_LISTENING   536
#define ERROR_PIPE_NOT_CONNECTED 233

#define FILE_SHARE_READ        0x00000001
#define FILE_SHARE_WRITE       0x00000002
#define FILE_ATTRIBUTE_NORMAL  0x00000080

/*──────────────────────────────────────────────────────────────────────────────
 * High-resolution timer surface — referenced by ElleTypes.h's inline
 * Elle_HighResTimestamp().  CLOCK_MONOTONIC backs both call patterns.
 *──────────────────────────────────────────────────────────────────────────────*/
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* freq) {
    if (!freq) return FALSE;
    freq->QuadPart = 1000000000LL;  /* nanoseconds → 1e9 ticks/sec */
    return TRUE;
}
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* counter) {
    if (!counter) return FALSE;
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    counter->QuadPart = (LONGLONG)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    return TRUE;
}
inline void GetSystemTime(LPSYSTEMTIME) {}
inline void GetSystemTimeAsFileTime(LPFILETIME ft) {
    if (!ft) return;
    ft->dwLowDateTime = ft->dwHighDateTime = 0;
}

/*──────────────────────────────────────────────────────────────────────────────
 * Console / Stdio surface — referenced by ElleLogger.cpp.
 *──────────────────────────────────────────────────────────────────────────────*/
#define STD_INPUT_HANDLE   ((DWORD)-10)
#define STD_OUTPUT_HANDLE  ((DWORD)-11)
#define STD_ERROR_HANDLE   ((DWORD)-12)
#define FOREGROUND_BLUE      0x0001
#define FOREGROUND_GREEN     0x0002
#define FOREGROUND_RED       0x0004
#define FOREGROUND_INTENSITY 0x0008
typedef WORD CONSOLE_TEXTATTRIBUTE;
inline HANDLE GetStdHandle(DWORD) { return (HANDLE)(LONG_PTR)1; }
inline BOOL   SetConsoleTextAttribute(HANDLE, WORD) { return TRUE; }
inline BOOL   WriteConsoleA(HANDLE, LPCVOID, DWORD, LPDWORD, void*) { return TRUE; }

/*──────────────────────────────────────────────────────────────────────────────
 * String helpers used in path normalisation
 *──────────────────────────────────────────────────────────────────────────────*/
inline int  lstrlenA(LPCSTR s) { return s ? (int)::strlen(s) : 0; }
inline LPSTR lstrcpyA(LPSTR dst, LPCSTR src) {
    if (!dst || !src) return dst;
    ::strcpy(dst, src);
    return dst;
}

/*──────────────────────────────────────────────────────────────────────────────
 * MSVC safe-string + safe-time helpers (errno_t variants)
 *──────────────────────────────────────────────────────────────────────────────*/
typedef int errno_t;
inline errno_t strncpy_s(char* dst, const char* src, size_t n) {
    if (!dst || !src) return 22;
    ::strncpy(dst, src, n);
    dst[n] = '\0';
    return 0;
}
template <size_t N>
inline errno_t strncpy_s(char (&dst)[N], const char* src, size_t n) {
    return strncpy_s(dst, src, n < N ? n : N - 1);
}
inline errno_t localtime_s(struct tm* out, const time_t* t) {
    if (!out || !t) return 22;
    ::localtime_r(t, out);
    return 0;
}
inline void GetLocalTime(LPSYSTEMTIME st) {
    if (!st) return;
    time_t t = ::time(nullptr);
    struct tm lt; ::localtime_r(&t, &lt);
    st->wYear   = (WORD)(lt.tm_year + 1900);
    st->wMonth  = (WORD)(lt.tm_mon  + 1);
    st->wDayOfWeek = (WORD)lt.tm_wday;
    st->wDay    = (WORD)lt.tm_mday;
    st->wHour   = (WORD)lt.tm_hour;
    st->wMinute = (WORD)lt.tm_min;
    st->wSecond = (WORD)lt.tm_sec;
    st->wMilliseconds = 0;
}

#endif /* _WINDOWS_LINUX_TEST_STUB_ */
