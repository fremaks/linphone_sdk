

#include <stdio.h>
#include <stdlib.h>
//#define LOG_WRITE_THRESHOLD FMS_LOG_FATAL
#define FMS_NDEBUG

#include "fms_log.h"
#include "fms_mem.h"
#include "fms_list.h"
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "fms_queue.h"
#if 0
void fms_mem_test() {
	printf("lroent 1\n");
	int i = 0;
	int d = 0;
	char *ptr= NULL;
	struct timeval begin, end;


	fms_mem_pool *pool = NULL;
	printf("lroent 2\n");
	srand((int)time(0));
	//  1~128
	pool = fms_mem_new(1024*1024);
	gettimeofday(&begin, NULL);
	for (i = 0; i < 1024*1024; i++) {
		d = 1 + (int)(128.0 *rand()/(RAND_MAX + 1.0));
		ptr = fms_mem_alloc(pool, d);
		//if (d > 64)
		  //fms_mem_free(pool, ptr);
	}
	printf("lroent 3\n");
	gettimeofday(&end, NULL);


	
	printf("fms_mem cost %ld ms\n", (end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000);

	gettimeofday(&begin, NULL);
	for (i = 0; i <1024*1024; i++) {
		d = 1 + (int)(128.0 *rand()/(RAND_MAX + 1.0));
		ptr = malloc(d);
		//if (d > 64)
		 //free(ptr);
	}
	gettimeofday(&end, NULL);
	printf("malloc cost %ld ms\n", (end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000);
	//fms_mem_destroy(pool);
}
#endif


typedef struct _tmp_node {
	fms_list list;
	int data;
} tmp_node;


fms_void fms_list_test(fms_void) {
	int data[10] = {5, 2, 6, 9, 4, 10, 3, 1, 7, 8};
	int i = 0;

	fms_list *head = fms_list_new();
	
	for (i = 0; i < 10; i++) {
		tmp_node *node = (tmp_node *)malloc(sizeof(tmp_node));
		node->data = data[i];
		fms_list_add_tail(head, &node->list);
	}
	i = 0;
	fms_list *pos = fms_list_next(head);
	while(fms_list_is_loop(head, pos)) {
		tmp_node *node  =  fms_list_data(pos, tmp_node, list);
		printf("data[%d] = [%d]\n", i++, node->data);
		pos = fms_list_next(pos);
	}
	
	
}


fms_void fms_queue_test (fms_void) {
	fms_queue *queue = fms_queue_new();
	int data[10] = {5, 2, 6, 9, 4, 10, 3, 1, 7, 8};
	int i = 0;
	fms_list *pos = NULL;
	
	printf("enqueue:\n");
	for (i = 0; i < 10; i++) {
		tmp_node *node = (tmp_node *)malloc(sizeof(tmp_node));
		node->data = data[i];
		fms_enqueue(queue, &node->list);
	}
	
	printf("after enqueue:size=%d\n", fms_queue_size(queue));
	pos = fms_queue_front(queue);
	tmp_node *node = (tmp_node *)fms_list_data(pos, tmp_node, list);
	printf("front=%d\n", node->data);

	printf("dequeue:\n");
	while (!fms_queue_is_empty(queue)) {
		//pos = fms_dequeue(queue);
		tmp_node *node = (tmp_node *)fms_list_data(pos, tmp_node, list);
		printf("data=%d\n", node->data);
	}
	printf("dequeue:after size=%d\n", fms_queue_size(queue));

}




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

	//fms_mem_test();
	fms_list_test();
	fms_queue_test();
	return 0;
}
