#ifndef _FREMKAS_MEM__H__
#define _FREMAKS_MEM__H__

#include "fms_type.h"
#include "list.h"
#include "rbtree.h"


#ifdef __cplusplus
extern  "C"
{
#endif


typedef struct _fms_mem_block {
	struct list_head entry;
	struct rb_node rb_node;
	fms_u32 data_size;
	fms_u32 id;
	fms_bool free;
	fms_u8 data[0];
}fms_mem_block;

typedef struct _fms_mem_pool {
	struct list_head blocks;
	struct rb_root free_blocks;
	struct rb_root allocated_blocks;
	fms_u32 block_id;
	fms_u32 capacity;
}fms_mem_pool;

fms_mem_pool *fms_mem_pool_new(fms_u32 capacity);
void fms_mem_pool_free(fms_mem_pool *mem_pool);
void *fms_mem_alloc(fms_mem_pool *mem_pool, fms_u32 data_size);
void fms_mem_free(fms_mem_pool *mem_pool, void *data);

#ifdef __cplusplus
}
#endif

#endif