#ifndef __FMS_QUEUE_H__
#define __FMS_QUEUE_H__
/*
1.ÕâÖÖÄÚÁªº¯ÊýÐèÒª¶Ô²ÎÊý×öÅÐ¶ÏÃ´?
2.Èç¹ûÒª¶Ô²ÎÊýÅÐ¶Ï£¬ÄÇÃ´ÒýÈëfms_logÊÇ²»ÊÇ²»Ì«ºÃÄØå 
*/

#ifdef __cplusplus
extern  "C"
{
#endif

#include "fms_type.h"
#include "fms_list.h"
#include "fms_log.h"


typedef struct _fms_queue {
	fms_list head;
	fms_s32 size;
} fms_queue;



inline static fms_void fms_queue_init(fms_queue *const queue) {
	FMS_EQUAL_RETURN(queue, NULL);

	fms_list_init(&queue->head);
	queue->size = 0;
}


inline static fms_queue *fms_queue_new(fms_void) {
	fms_queue *queue = NULL;

	queue = (fms_queue *)malloc(sizeof(fms_queue));
	FMS_ASSERT(queue != NULL);

	fms_queue_init(queue);

	return queue;
}

inline static fms_s32 fms_queue_size(const fms_queue *const queue) {
	FMS_EQUAL_RETURN_VALUE(queue, NULL, 0);

	return queue->size;
}

inline static fms_bool fms_queue_is_empty(const fms_queue *queue) {
	FMS_EQUAL_RETURN_VALUE(queue, NULL, FMS_TRUE);
	
	return fms_queue_size(queue) <= 0;
}


inline static fms_list *fms_queue_front(fms_queue *const queue) {
	fms_list *list = NULL;
	
	FMS_EQUAL_RETURN_VALUE(queue, NULL, NULL);	
	FMS_EQUAL_RETURN_VALUE(fms_list_is_empty(&queue->head), FMS_TRUE, NULL);
	
	list = fms_list_next(&queue->head);

	return list;
}

inline static fms_list *fms_queue_tear(fms_queue *const queue) {
	fms_list *list = NULL;
	
	FMS_EQUAL_RETURN_VALUE(queue, NULL, NULL);	
	FMS_EQUAL_RETURN_VALUE(fms_list_is_empty(&queue->head), FMS_TRUE, NULL);
	
	list = fms_list_prev(&queue->head);

	return list;
}

inline static fms_void fms_enqueue(fms_queue *const queue, fms_list *const list) {
	FMS_EQUAL_RETURN(queue, NULL);
	FMS_EQUAL_RETURN(list, NULL);

	fms_list_add_tail(&queue->head, list);
	queue->size++;
}

inline static fms_list *fms_dequeue(fms_queue *const queue) {
	fms_list *list = NULL;
	
	FMS_EQUAL_RETURN_VALUE(queue, NULL, NULL);	
	//FMS_EQUAL_RETURN_VALUE(fms_list_is_empty(&queue->head), FMS_TRUE, NULL);	
	if (fms_list_is_empty(&queue->head)) {
		return NULL;
	}
	list = fms_queue_front(queue);
	fms_list_del(list);
	queue->size--;
	
	return list;
}


#ifdef __cplusplus
}
#endif

#endif




