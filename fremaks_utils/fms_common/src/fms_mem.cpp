#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "fms_mem.h"
#include "fms_log.h"

static fms_void *mem_block_alloc(fms_mem_pool *mem_pool, fms_s32 size);
static fms_void *mem_large_alloc(fms_mem_pool *mem_pool, fms_s32 size);

fms_mem_pool *fms_mem_new(fms_s32 block_size) {
	fms_mem_pool *mem_pool = NULL;
	fms_mem_block *mem_block = NULL;

	if (block_size <= 0) {
		block_size = FMS_PAGE_SIZE - 1;
		//FMS_WARN("block_size <= 0, use default size 4096 - 1\n");
	}
	
	mem_pool = (fms_mem_pool *)malloc(sizeof(fms_mem_pool));
	//FMS_ASSERT(mem_pool != NULL);
	mem_pool->block_size = block_size;
	fms_list_init(&mem_pool->head_block);
	fms_list_init(&mem_pool->head_large);

	mem_block = (fms_mem_block *)malloc(sizeof(fms_mem_block) + block_size);
    mem_block->cur = (fms_s8 *)((fms_s8 *)mem_block + sizeof(fms_mem_block));
    mem_block->end = mem_block->cur + mem_pool->block_size;
	mem_block->failed = 0;
	
	fms_list_add_tail(&mem_pool->head_block, &mem_block->list);
	mem_pool->current_block = mem_block;	

	return mem_pool;
}

		
fms_void *fms_mem_alloc(fms_mem_pool *const mem_pool, const fms_s32 size) {
    fms_void *ptr = NULL;
	
	//FMS_EQUAL_RETURN_VALUE(size <= 0, FMS_TRUE, NULL);
	
    if (size <= FMS_PAGE_SIZE - 1) {
		fms_list *pos = NULL;
		fms_mem_block *tmp_block = NULL;
		
		pos = &mem_pool->current_block->list;
		while (fms_list_is_loop(&mem_pool->head_block, pos)) {
			tmp_block = fms_list_data(pos, fms_mem_block, list);
			ptr = fms_align_ptr(tmp_block->cur, FMS_MEM_POOL_ALIGN_MENT);

            if ((size_t)(tmp_block->end - (fms_s8 *)ptr) >= size) {
                tmp_block->cur = (fms_s8 *)ptr + size;
                return ptr;
            }

            pos = fms_list_next(pos);
		}	

        return mem_block_alloc(mem_pool, size);
    }

    return mem_large_alloc(mem_pool, size);
}


fms_void fms_mem_free(fms_mem_pool *const mem_pool, fms_void *const ptr) {
	fms_list *pos = NULL;
	fms_mem_large *tmp_large = NULL;
	
	pos = fms_list_next(&mem_pool->head_large);
	while (fms_list_is_loop(&mem_pool->head_large, pos)) {
		tmp_large = fms_list_data(pos, fms_mem_large, list);
		if (tmp_large->alloc == ptr) {
			free(ptr);
			tmp_large->alloc = NULL;
		}
	}

}

fms_void fms_mem_destroy(fms_mem_pool *const mem_pool) {
	fms_mem_block *tmp_block = NULL;
	fms_mem_large *tmp_large = NULL;
	fms_list *pos = NULL;

	pos = fms_list_next(&mem_pool->head_block);
	while (fms_list_is_loop(&mem_pool->head_block, pos)) {
		tmp_block = fms_list_data(pos, fms_mem_block, list);
		pos = fms_list_next(pos);
		free(tmp_block);	
	}
	
	pos = fms_list_next(&mem_pool->head_large);
	while (fms_list_is_loop(&mem_pool->head_large, pos)) {
		tmp_large = fms_list_data(pos, fms_mem_large, list);
		pos = fms_list_next(pos);
		
		if (tmp_large->alloc != NULL) {
			free(tmp_large->alloc);
		}
		free(tmp_large);
	}

	free(mem_pool);
}

static fms_void *mem_block_alloc(fms_mem_pool *const mem_pool, const fms_s32 size) {
	fms_void *ptr = NULL;
	fms_list *pos = NULL; 
	fms_s8 *start = NULL;
	fms_mem_block *new_block = NULL;
	fms_mem_block *tmp_block = NULL;
	
	new_block = (fms_mem_block *)malloc(sizeof(fms_mem_block) + mem_pool->block_size);
	//FMS_ASSERT(new_block != NULL);
	start = (fms_s8 *)((fms_s8 *)new_block + sizeof(fms_mem_block));
	new_block->end = start + mem_pool->block_size;
	ptr = fms_align_ptr(start, FMS_MEM_POOL_ALIGN_MENT);
	new_block->cur = (fms_s8 *)ptr + size;
	new_block->failed = 0;

	pos = fms_list_next(&mem_pool->head_block);
	while (fms_list_is_loop(&mem_pool->head_block, pos)) {
		tmp_block = fms_list_data(pos, fms_mem_block, list);

		if (tmp_block->failed++ > 4) {
			mem_pool->current_block = fms_list_data(fms_list_next(pos), fms_mem_block, list);
		}
		
		pos = fms_list_next(pos);
	}

	fms_list_add_tail(&mem_pool->head_block, &new_block->list);	

	return ptr;
}

static fms_void *mem_large_alloc(fms_mem_pool *const mem_pool, const fms_s32 size) {
  	fms_void  *ptr = NULL;
	fms_list *pos = NULL;
	fms_s8 n = 0;
	fms_mem_large *new_large = NULL;
	fms_mem_large *tmp_large = NULL;
	
    ptr = (fms_void  *)malloc(size);
	//FMS_ASSERT(ptr != NULL);

	pos = fms_list_next(&mem_pool->head_large);
	
	while(fms_list_is_loop(&mem_pool->head_large, pos)) {
		tmp_large = fms_list_data(pos, fms_mem_large, list);
		if (NULL == tmp_large->alloc) {
			tmp_large->alloc = ptr;
			return ptr;
		}
		
		if (n++ > 3) {
			break;
		}
	}

	new_large = (fms_mem_large *)malloc(sizeof(fms_mem_large));
	//FMS_ASSERT(new_large != NULL);
	new_large->alloc = ptr;
	
	fms_list_add_tail(&mem_pool->head_large, &new_large->list);

    return ptr;	

}

