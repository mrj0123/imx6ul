#ifndef _BASETYPE_H_
#define _BASETYPE_H_

#include "a.h"

//typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef unsigned char  BOOL;

/* When building OpenTV, these types are defined in the OpenTV headers */
typedef unsigned long long u_int64;
typedef unsigned int u_int32;
typedef unsigned short u_int16;
typedef unsigned char u_int8;
typedef int sem_id_t;
#if !defined(TYPEDEF_H) /* Added to keep softmodem code common */
typedef signed long long int64;
typedef signed long int32;
typedef signed short int16;
typedef signed char int8;

/* Added to keep softmodem code common */
typedef unsigned long long uint64;
typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
#endif /* !defined(TYPEDEF_H) */

typedef unsigned char           UINT8;
typedef unsigned int            UINT16;
typedef unsigned long           UINT32;
typedef signed long INT32;
typedef int                 INT;         ///<Not in v2.0. Borrow the name from uITRON 3.0 and later
//typedef unsigned long   DWORD;
//typedef unsigned short  WORD;
//typedef unsigned char  BYTE;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL    0
#endif
typedef  signed short           SHORT;
typedef  unsigned short     USHORT;
typedef  long                   LONG;
typedef  unsigned long          ULONG;
typedef  unsigned int           UINT;
typedef unsigned char  U8;
typedef unsigned short  U16;
typedef unsigned int    U32;
typedef signed int S32;
/* Common signed types */
typedef  signed char    S8;
typedef float           FLOAT;
typedef FLOAT           *PFLOAT;
typedef unsigned int    *PUINT;

//typedef unsigned char       u8;
typedef signed char         s8;
typedef unsigned short int  u16;
typedef signed short int    s16;
typedef unsigned int        u32;
typedef signed int          s32;



#endif /* _BASETYPE_H_ */
