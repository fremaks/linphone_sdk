
//这种链表唯一的缺点就在于删除节点时很麻烦，时间复杂度高为N,而内核链表删除时间复杂度为1
#include "fms_list.h"
#include "fms_log.h"
#include <stdio.h>
#include "fms_type.h"

typedef struct _fms_list {
	fms_void *data;
	struct _fms_list *prev;
	struct _fms_list *next;
} fms_list;


//#define fms_list_next(list) ((list)->next)
//#define fms_list_prev(list) ((list)->prev)
#define fms_list_loop(first, list) {\
	if (((list) = (list)->next) == (first)) {\
		break;\
	}\
}

fms_list *fms_list_new(fms_void *data);

fms_list *fms_list_find(fms_list *list, fms_void *data);

fms_list *fms_list_append_link(fms_list *list, fms_list *new_list);

fms_list *fms_list_append(fms_list *list, fms_void *data);

fms_list *fms_list_prepend_link(fms_list *list, fms_list *new_list);

fms_list *fms_list_prepend(fms_list *list, fms_void *data);

fms_list *fms_list_remove_link(fms_list * list, fms_list *old_list);

fms_list *fms_list_remove(fms_list *list, fms_void *data);

static fms_void fms_list_link(fms_list *prev, fms_list *next, fms_list *new_list) {
	prev->next = new_list;
	new_list->next = next;
	new_list->prev = prev;
	next->prev = new_list;
}

static fms_void fms_list_unlink(fms_list *list) {
	list->next->prev = list->prev;
	list->prev->next = list->next;
}

fms_list *fms_list_new(fms_void *data) {
	fms_list *list = NULL;

	list = (fms_list *)malloc(sizeof(fms_list));
	FMS_ASSERT(list != NULL);

	list->prev = list->next = list;
	list->data = data;

	return list;
}


fms_list *fms_list_find(fms_list *list, fms_void *data) {
	fms_list *pos = list;
	
//	FMS_EQUAL_RETURN_VALUE(list, NULL, NULL);
	FMS_EQUAL_RETURN_VALUE(data, NULL, NULL);

	while (pos) {
		if (pos->data == data) {
			return pos;
		}
		fms_list_loop(list, pos);
	}

	return NULL;	
}



fms_list *fms_list_append_link(fms_list *list, fms_list *new_list) {
	if (NULL == list) {
		return new_list;
	}
	
	if (new_list != NULL) {
		fms_list_link(list->prev, list, new_list);	
	}	

	return list;
}

fms_list *fms_list_append(fms_list *list, fms_void *data) {
	fms_list *new_list = fms_list_new(data);
	
	return fms_list_append_link(list, new_list);
}

fms_list *fms_list_prepend_link(fms_list *list, fms_list *new_list) {
	if (NULL == new_list) {
		return list;
	}
	
	if (list != NULL) {
		fms_list_link(list->prev, list, new_list);
	}
	
	return new_list;
}

fms_list *fms_list_prepend(fms_list *list, fms_void *data) {
	fms_list *new_list = fms_list_new(data);

	return fms_list_prepend_link(list, new_list);
}


fms_list *fms_list_remove_link(fms_list * list, fms_list *old_list) {
	if (NULL == list || NULL == old_list) {
		return list;
	}

	if (list == old_list) {
		list = list->next;
		if (list == old_list) { //only one elem
			list = NULL;
		}
	} 
	
	fms_list_unlink(old_list);
	free(old_list);

	return list;
}

fms_list *fms_list_remove(fms_list *list, fms_void *data) {
	//fms_list *old_list = fms_list_find(list, data);
	fms_list *old_list = (fms_list *)data;//这种转换更加有效率
	
	if (old_list != NULL) {
		return fms_list_remove_link(list, old_list);
	} 
	
	return list;	
}

