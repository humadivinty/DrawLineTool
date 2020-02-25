#ifndef COMMONDEF_H
#define COMMONDEF_H
#include<stdint.h>


//typedef int BOOL;
//typedef  unsigned int UINT;
//typedef  unsigned char BYTE;
//typedef  int INT32;
//typedef  long long INT64;
//typedef  unsigned long DWORD;
//typedef  unsigned char* PBYTE;
//typedef  unsigned short WORD;
//typedef  unsigned long long DWORD64;
//typedef  const char* LPCSTR;
//typedef  void* LPVOID;
//typedef  void* PVOID;



//#ifndef BOOL
//#define BOOL int
//#endif

//#ifndef UINT
//#define UINT unsigned int
//#endif


//#ifndef BYTE
//#define BYTE unsigned char
//#endif

//#ifndef INT32
//#define INT32 int
//#endif

//#ifndef UINT32
//#define UINT32 uint32_t
//#endif


//#ifndef INT64
//#define INT64 long long
//#endif

//#ifndef UINT64
//#define UINT64 uint64_t
//#endif


//#ifndef DWORD
//#define DWORD unsigned long
//#endif

//#ifndef PBYTE
//#define PBYTE unsigned char*
//#endif

//#ifndef WORD
//#define WORD short int
//#endif

//#ifndef DWORD64
//#define DWORD64 unsigned long long
//#endif

//#ifndef LPCSTR
//#define LPCSTR const char*
//#endif

//#ifndef LPVOID
//#define LPVOID void*
//#endif

//#ifndef PVOID
//#define PVOID PVOID
//#endif

#ifndef MAX_PATH
#define MAX_PATH (256)
#endif

#ifndef GET_LAST_ERROR
#define GET_LAST_ERROR 0
#endif

#ifndef WM_USER
#define WM_USER 0x0400
#endif

#endif // COMMONDEF_H
