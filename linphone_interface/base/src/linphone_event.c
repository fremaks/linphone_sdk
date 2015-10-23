#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "linphone_event.h"
#include "fms_log.h"

linphone_event *linphone_event_init(linphone_event_type type, fms_s8 *data) {
	linphone_event *new_event = NULL;
	fms_s32 data_len = 0;

	if (data != NULL) {
		data_len = strlen(data);
		if (data_len != 0) {
			data_len++;
		}
	}
	
	new_event = (linphone_event *)malloc(sizeof(linphone_event) + data_len);
	FMS_ASSERT(new_event != NULL);

	new_event->type = type;
	fms_list_init(&new_event->list);
	if (data_len != 0) {
		strcpy(new_event->data, data);
	}
	
	return new_event;
}

fms_void linphone_event_uninit(linphone_event *event) {
	//FMS_EQUAL_RETURN(event, NULL);

	free(event);
}

