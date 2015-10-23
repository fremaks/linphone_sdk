#include "fms_mem.h"
#include "fms_log.h"
#include <stdlib.h>

//BinderÊÇÍ¨¹ýÇ°ºóµÄ½ÚµãÀ´¼ÆËãÄÚ´æ¿éµÄ´óÐ¡£¬¶øÎÒÊÇÖ±½Ó¸³Öµ
//ÔÚÓ¦ÓÃ²ãÄÚ´æÐèÒª×öÄÚ´æ¶ÔÆëÃ´?ÄÇÃ´BinderÄÚºË²ãÄØ?

#define MEM_ALIGN_BIT 4

#define mem_align_ptr(p, a) \
    (fms_u8 *) (((fms_u32) (p) + ((fms_u32) (a) - 1)) & ~((fms_u32) (a) - 1))

//×¢Òâ freeºìºÚÊ÷ÒÔ»º³åÇø´óÐ¡×éÖ¯£¬¶øallocatedÒÔµØÖ·×éÖ¯?why?¿ÉÒÔÖ±½ÓÓÃÁ´±í×éÖ¯Ã´?·´ÕýÃ»ÓÐÓÃµ½²éÑ¯µÄ¹¦ÄÜ
//ÄÚ²¿º¯Êý²»ÐèÒª¼ì²é²ÎÊý

int my_insert(struct rb_root *root, fms_mem_block *new_block)
{
	struct rb_node **pos = &(root->rb_node), *parent = NULL;
//printf("fremaks 1,*pos=%x\n", *pos);
	/* Figure out where to put new node */
	while (*pos)
	{
//printf("fremaks 2\n");
		fms_mem_block *block = container_of(*pos, fms_mem_block, rb_node);
		parent = *pos;
		
		if (new_block->data_size < block->data_size) {
			pos = &((*pos)->rb_left);
		} else {
			pos = &((*pos)->rb_right);
		}	
	}
//printf("fremaks 3\n");
	/* Add new node and rebalance tree. */
	rb_link_node(&new_block->rb_node, parent, pos);
//printf("fremaks 4, *pos=%x\n", *pos);
	rb_insert_color(&new_block->rb_node, root);
//printf("fremaks 5\n");
}



static void mem_block_insert_free(fms_mem_pool *mem_pool, fms_mem_block *new_block) {
	//printf("@@@@@@@@@@@free:%x alloc:%x\n", mem_pool->free_blocks.rb_node, mem_pool->allocated_blocks.rb_node);
	my_insert(&mem_pool->free_blocks, new_block);
}


void mem_block_insert_allocated(fms_mem_pool *mem_pool, fms_mem_block *new_block) {
	//printf("###########alloc:%x free=%x\n", mem_pool->allocated_blocks.rb_node, mem_pool->free_blocks.rb_node);
	my_insert(&mem_pool->allocated_blocks, new_block);
}


fms_mem_pool *fms_mem_pool_new(fms_u32 capacity) {
	fms_mem_pool *mem_pool = NULL;	
	
	mem_pool = (fms_mem_pool *)malloc(sizeof(fms_mem_pool));
	FMS_ASSERT(mem_pool != NULL);
	memset(mem_pool, 0, sizeof(fms_mem_pool));
	
	INIT_LIST_HEAD(&mem_pool->free);
	INIT_LIST_HEAD(&mem_pool->used);
	//mem_pool->free_blocks = RB_ROOT;//?????????
	//mem_pool->allocated_blocks= RB_ROOT;
	//mem_pool->block_id = 0;

	
	mem_pool->capacity = capacity;

	fms_mem_block *mem_block = (fms_mem_block *)malloc(capacity);
	FMS_ASSERT(mem_block != NULL);
	memset(mem_block, 0, capacity);
	
	mem_block->data_size = capacity - sizeof(fms_mem_block);
	//mem_block->id = mem_pool->block_id;
	mem_block->free = FMS_TRUE;
	list_add(&mem_block->entry, &mem_pool->free);
	pthread_mutex_init(&mem_pool->lock, NULL);
	//mem_block_insert_free(mem_pool, mem_block);
	//printf("@@@@@@@@size=%d\n", sizeof(fms_mem_block));
	//printf(">>>>>>>>>>free=%x allocl=%x\n", mem_pool->free_blocks.rb_node, mem_pool->allocated_blocks.rb_node);
	return mem_pool;
}


void fms_mem_pool_free(fms_mem_pool *mem_pool) {

	
}



void *fms_mem_alloc(fms_mem_pool *mem_pool, fms_u32 data_size) {

	fms_mem_block *block = NULL;
	struct rb_node *best_fit = NULL;
	fms_u32 block_data_size = 0;
	data_size = mem_align_ptr(data_size, 4);//²»¼ÓÕâÒ»¾ä¾Í¶Î´íÎóå why ?½¨Òé×öÒ»´ÎµØÖ·¸ú×Ù²âÊÔ
	//printf("@@@@@@@@@@fms_mem_alloc:datasize=%u\n", data_size);
	struct list_head *pos = NULL;
	pthread_mutex_lock(&mem_pool->lock);
	list_for_each(pos, &mem_pool->free) {
		block = list_entry(pos, fms_mem_block, entry);
		if (block->data_size >= data_size) {
			if (block->data_size - data_size > sizeof(fms_mem_block) + 4) {//²Ã¼ô
				fms_u32 new_data_size =  block->data_size - data_size - sizeof(fms_mem_block);
				block->data_size = data_size;
				fms_mem_block *new_block = (fms_mem_block *)((fms_u8 *)block->data + block->data_size);
				new_block->data_size = new_data_size;
				list_del(&block->entry);
				new_block->free = FMS_TRUE;
				list_add(&new_block->entry, &mem_pool->free);
				block->free = FMS_FALSE;
				list_add(&block->entry, &mem_pool->used);
			}
			break;
		}
	}
	pthread_mutex_unlock(&mem_pool->lock);
	
	return (void *)block->data;
}

//ÊÍ·ÅµÄÊ±ºòºÏ²¢,Ïà¶ÔÓÚbinder¶øÑÔ£¬ÕâÀïµÄ»º³åÇø»áÀ©Õ¹£¬ËùÒÔÄÚ´æ²»ÊÇÈ«²¿Á¬Ðø£¬¶øÊÇÒ»¶Î¶ÎÁ¬Ðø
void fms_mem_free(fms_mem_pool *mem_pool, void *data) {
	struct list_head *pos = NULL;
	fms_mem_block *new_block = NULL;
	fms_mem_block *block = (fms_s8 *)data - sizeof(fms_mem_block);
	pthread_mutex_lock(&mem_pool->lock);
	list_del(&block->entry);

	pos = block->entry.next;
	if (pos != &mem_pool->free) {
		new_block = list_entry(pos, fms_mem_block, entry);
		if (new_block->free) {
			list_del(&new_block->entry);
			block->data_size += new_block->data_size + sizeof(fms_mem_block);
		}
	}

	pos = block->entry.prev;
	if (pos != &mem_pool->free) {
		new_block = list_entry(pos, fms_mem_block, entry);
		if (new_block->free) {
			new_block->data_size += block->data_size + sizeof(fms_mem_block);
		}
	}

	list_add(&block->entry, &mem_pool->free);
	pthread_mutex_unlock(&mem_pool->lock);
}


#if 0
int my_insert(struct rb_root *root, struct mynode *data)
{
      struct rb_node **new1 = &(root->rb_node), *parent = NULL;

      /* Figure out where to put new node */
      while (*new1) {
          struct mynode *this1 = container_of(*new1, struct mynode, node);
          int result = strcmp(data->string, this1->string);

        parent = *new1;
          if (result < 0)
              new1 = &((*new1)->rb_left);
          else 
              new1 = &((*new1)->rb_right);
    
      }

      /* Add new node and rebalance tree. */
      rb_link_node(&data->node, parent, new1);
      rb_insert_color(&data->node, root);

    return 1;
}
#endif

