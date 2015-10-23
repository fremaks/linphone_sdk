#include <stdio.h>
#include <stdlib.h>
//#define LOG_WRITE_THRESHOLD FMS_LOG_FATAL
#define FMS_NDEBUG

#include "fms_log.h"
#include "fremaks_mem.h"
#include "fms_mem.h"
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

void *log_fun(void *arg) {

	int i = 0;

	for (i = 0;  i< 2; i++) {
		FMS_DEBUG("HELLO WORLD i=%d[%lu]\n", i, pthread_self());
		usleep(200000);
	}

	return (void *)0;
}
#include <sys/time.h>

void mem_test1() {
	fms_mem_pool *pool = fms_mem_pool_new(200*1024*1024);
	int i = 0;
	int d = 0;
	char *ptr= NULL;
	struct timeval begin, end;
	
	srand((int)time(0));
	//  1~128

	gettimeofday(&begin, NULL);
	for (i = 0; i < 1024*1024; i++) {
		d = 1 + (int)(128.0 *rand()/(RAND_MAX + 1.0));
		ptr = fms_mem_alloc(pool, 1);
		if (d > 64)
		  fms_mem_free(pool, ptr);
	}
	gettimeofday(&end, NULL);
	printf("fms_mem cost %ld ms\n", (end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000);
		

	
	gettimeofday(&begin, NULL);
	for (i = 0; i <1024*1024; i++) {
		d = 1 + (int)(128.0 *rand()/(RAND_MAX + 1.0));
		ptr = malloc(1);
		if (d > 64)
			free(ptr);
	}
	gettimeofday(&end, NULL);
	printf("malloc cost %ld ms\n", (end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000);


}

#if 0
void mem_test2() {

	mem_pool *pool = mem_pool_create(200*1024*1024);
	int i = 0;
	int d = 0;
	char *ptr= NULL;
	struct timeval begin, end;
	
	srand((int)time(0));
	//  1~128

	gettimeofday(&begin, NULL);
	for (i = 0; i < 1024*1024; i++) {
		d = 12 + (int)(128.0 *rand()/(RAND_MAX + 1.0));
		ptr = mem_pool_alloc(pool, 1);
		//if (d % 2)
		//fms_mem_free(pool, ptr);
	}
	gettimeofday(&end, NULL);
	printf("fms_mem cost %ld ms\n", (end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000);
		

	
	gettimeofday(&begin, NULL);
	for (i = 0; i <1024*1024; i++) {
		d = 12 + (int)(128.0 *rand()/(RAND_MAX + 1.0));
		ptr = malloc(1);
		//if (d % 2)
		//free(ptr);
	}
	gettimeofday(&end, NULL);
	printf("malloc cost %ld ms\n", (end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000);


}
#endif


int main(int agrc, char *argv[]) {
#if 0
	//int num = atoi(argv[1]);
	fms_log_init(256*1024, "./tmp/");
	pthread_t num;
	pthread_t num1;
	pthread_t num2;
	pthread_create(&num, NULL, log_fun, NULL);
	pthread_create(&num1, NULL, log_fun, NULL);
	//pthread_create(&num2, NULL, log_fun, NULL);
	pthread_join(num, NULL);
	pthread_join(num1, NULL);
	//pthread_join(num2, NULL);
	FMS_INFO("HELLO WORLD 1\n");
	FMS_WARN("HELLO WORLD 2\n");
	FMS_ERROR("HELLO WORLD 3\n");

#endif
	//要取得a到b之间的随机整数，另一种表示：a + (int)b * rand() / (RAND_MAX + 1)
	//srand((int) time(0));
	//a + (int)(b.0 *rand()/(RAND_MAX + 1.0))
#if 0
	int i = 0;
	fms_mem_pool *pool = fms_mem_pool_new(1000);
	fms_mem_block *block = (fms_mem_block *)malloc(sizeof(fms_mem_block) + 1000);
	memset(block, 0, sizeof(fms_mem_block) + 1000);
printf("fms 1\n");	
	block->data_size = 8;
	mem_block_insert_allocated(pool, block); 
printf("fms 2\n");	
	block = block->data + block->data_size;
	block->data_size = 4;
	mem_block_insert_allocated(pool, block); 
printf("fms 3\n");	
	block = block->data + block->data_size;
	block->data_size = 12;
	mem_block_insert_allocated(pool, block); 
printf("fms 4\n");	

	struct rb_node *node = NULL;
	printf("@@@@@@@@@@@@@@alloc@@@@@@@@@@@@@@@@@@\n");
    for (node = rb_first(&pool->allocated_blocks); node; node = rb_next(node))
        printf("size = %u\n", rb_entry(node, fms_mem_block, rb_node)->data_size);
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");	
#endif
	mem_test1();



	return 0;
}
