#define FMS_NDEBUG 1
#include <stdio.h>
#include <stdlib.h>
#include "linphone_event.h"
#include "linphone_base.h"
#include "fms_log.h"
//#include <string.h>

fms_void linphone_native_event_callback(linphone_event *event) {
	FMS_EQUAL_RETURN(event, NULL);

	printf("------------------------------------linphone_native_event_callback:type=%d msg=%s\n", event->type, event->data);

}

fms_s32 main(fms_s32 argc, fms_s8 *argv[]) {
	fms_s8 cmd_buff[512] = {0};
	linphone_event *event = NULL;

	linphone_base_init("./linphone.cfg", linphone_native_event_callback, FMS_FALSE, FMS_FALSE);
	while (1) {
		memset(cmd_buff, 0 , sizeof(cmd_buff));
		fgets(cmd_buff, sizeof(cmd_buff), stdin);//»»ÐÐ·ûÒ²¶ÁÈëÁËå
		printf("@@@fgets:cmd_buff=%s len=%d\n", cmd_buff, strlen(cmd_buff));
		if (strncmp(cmd_buff, "reg", 3) == 0) {
			FMS_DEBUG("fms->reg\n");
			event = linphone_event_init(LINPHONE_REGISTER_REQUEST, "2002>2002>192.168.66.73>");
			linphone_base_add_event(event);
		} else if (strncmp(cmd_buff, "call1", 5) == 0) {
			FMS_DEBUG("fms->call1\n");
			event = linphone_event_init(LINPHONE_CALL_REQUEST, "020806>");
			linphone_base_add_event(event);			
		} else if (strncmp(cmd_buff, "call2", 5) == 0) {
			FMS_DEBUG("fms->call2\n");
			event = linphone_event_init(LINPHONE_CALL_REQUEST, "0258>");
			linphone_base_add_event(event);			
		}  else if (strncmp(cmd_buff, "answer", 6) == 0) {
			FMS_DEBUG("fms->answer\n");
			event = linphone_event_init(LINPHONE_ANSWER_REQUEST, NULL);
			linphone_base_add_event(event);			
		}	else if (strncmp(cmd_buff, "hangup", 6) == 0) {
			FMS_DEBUG("fms->hangup\n");
			event = linphone_event_init(LINPHONE_HANGUP_REQUEST, NULL);
			linphone_base_add_event(event);			
		}	
	}
	

	return 0;
}
