#ifndef __FMS_LIST_H__
#define __FMS_LIST_H__
/*
1.ÕâÖÖÄÚÁªº¯ÊıĞèÒª¶Ô²ÎÊı×öÅĞ¶ÏÃ´?
2.Èç¹ûÒª¶Ô²ÎÊıÅĞ¶Ï£¬ÄÇÃ´ÒıÈëfms_logÊÇ²»ÊÇ²»Ì«ºÃÄØå 
*/
#ifdef __cplusplus
extern  "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>

typedef struct _fms_list {
	struct _fms_list *prev;
	struct _fms_list *next;
} fms_list;


inline static fms_void fms_list_init(fms_list *const head) {	
	head->prev = head->next = head;
}

inline static fms_list *fms_list_new(fms_void) {	
	fms_list *list = NULL;
	
	list = (fms_list *)malloc(sizeof(fms_list));
	//FMS_ASSERT(list != NULL);

	fms_list_init(list);

	return list;
}

inline static fms_void fms_list_add(fms_list *prev, fms_list *next, fms_list *list) {
	prev->next = list;
	list->next = next;
	list->prev = prev;
	next->prev = list;
}

inline static fms_void fms_list_add_head(fms_list *const head, fms_list *const list) {
	fms_list_add(head, head->next, list);
}

inline static fms_void fms_list_add_tail(fms_list *const head, fms_list *const list) {
	fms_list_add(head->prev, head, list);
}

inline static fms_void fms_list_del(fms_list *const list) {
	list->next->prev = list->prev;
	list->prev->next = list->next;
}

inline static const fms_bool fms_list_is_empty(const fms_list *const head) {
	return head->next == head;
}

inline static fms_list *fms_list_prev(const fms_list *const list) {
	return list->prev;
}

inline static fms_list *fms_list_next(const fms_list *const list) {
	return list->next;
}


inline static const fms_bool fms_list_is_loop(const fms_list *const head, const fms_list *const list) {
	return list != head;
}



#define fms_list_data(ptr, type, member) \
	((type *)((fms_s8 *)(ptr)-(fms_uintptr)(&((type *)0)->member)))
	


#ifdef __cplusplus
}
#endif



#endif
