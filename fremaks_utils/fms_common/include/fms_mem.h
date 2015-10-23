#ifndef __FMS_MEM_POOL_H__
#define __FMS_MEM_POOL_H__
#pragma pack(8)
#define FREMAKS_MEM 105
#ifdef __cplusplus
extern  "C"
{
#endif

#include "fms_type.h"
#include "fms_list.h"
#include <stdint.h>

#define FMS_PAGE_SIZE   4096  

//why use 16, not 8 ? 4?
#define FMS_MEM_POOL_ALIGN_MENT       16

#define fms_align_ptr(p, a) \
 (fms_s8 *) (((fms_uintptr)(p) + ((fms_uintptr)(a) - 1)) & ~((fms_uintptr)(a) - 1))

    
typedef struct _fms_mem_large{
    fms_void *alloc;
    fms_list list;
} fms_mem_large;

typedef struct _fms_mem_block {
    fms_s8 *cur;
    fms_s8 *end;
	fms_s8 failed;
    fms_list list;
} fms_mem_block;


typedef struct  _fms_mem_pool {
	fms_s32 block_size;
    fms_mem_block *current_block;
    fms_list head_block;
	fms_list head_large;
} fms_mem_pool;

fms_mem_pool *fms_mem_new(fms_s32 block_size);

fms_void *fms_mem_alloc(fms_mem_pool *const mem_pool, const fms_s32 size);

fms_void fms_mem_free(fms_mem_pool *const mem_pool, fms_void *const ptr);

fms_void fms_mem_destroy(fms_mem_pool *const mem_pool);

#ifdef __cplusplus
}
#endif

#endif
