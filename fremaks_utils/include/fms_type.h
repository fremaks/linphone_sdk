#ifndef __FREMAKS_TYPE_H__
#define __FREMAKS_TYPE_H__

#ifdef __cplusplus
extern  "C"
{
#endif


#if __WORDSIZE == 64

/** Signed 64bit integer. */
typedef long int fms_s64;

/** Unsigned 64bit integer. */
typedef unsigned long int	fms_u64;

#else

/** Signed 64bit integer. */
typedef long long int fms_s64;

/** Unsigned 64bit integer. */
typedef unsigned long long int	fms_u64;

#endif


/** Signed 32bit integer. */
typedef int		fms_s32;

/** Unsigned 32bit integer. */
typedef unsigned int	fms_u32;

/** Signed 16bit integer. */
typedef short		fms_s16;

/** Unsigned 16bit integer. */
typedef unsigned short	fms_u16;

/** Signed 8bit integer. */
typedef char fms_s8;

/** Unsigned 8bit integer. */
typedef unsigned char	fms_u8;


typedef int	 fms_bool;

typedef volatile fms_s32  fms_atomic;

#define SPIN_LOCK_INIT_VALUE 0


typedef long int               fms_intptr;
typedef unsigned long int      fms_uintptr;



#define FMS_TRUE 1
#define FMS_FALSE 0

#define FMS_SUCCESS 0
#define FMS_FAILED  -1

#define fms_void void

#ifdef __cplusplus
}
#endif

#endif