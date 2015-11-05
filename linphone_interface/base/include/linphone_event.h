#ifndef __LINPHONE_EVENT_H__
#define __LINPHONE_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "fms_type.h"
#include "fms_list.h"

typedef enum {
	CALL_NOT_REGISTER = -0x2,
	CALL_NOT_FIND = -0x3,
	CALL_BUSY = -0x4,
	CALL_NO_ANSWER = -0x5
} linphone_ret_value;

	
typedef enum {
	LINPHONE_REGISTER_REQUEST = 0x0,    //HouseNo>Password>Proxy_IP>
    LINPHONE_REGISTER_RESPBONSE,        //retValue>
    LINPHONE_CALL_REQUEST,              //HouseNo>HasVideo>EarlyMeidia>
    LINPHONE_CALL_RESPBONSE,            //retValue>
    LINPHONE_ANSWER_REQUEST,          
    LINPHONE_ANSWER_RESPBONSE,          //retValue>
    LINPHONE_HANGUP_REQUEST,
    LINPHONE_HANGUP_RESPBONSE,          //retValue>
    LINPHONE_CAMERA_SWITCH_REQUEST,		//Swicth_code>
    LINPHONE_CAMERA_SWITCH_RESPBONSE,	//retValue
    LINPHONE_EVENT_UNKNOW
} linphone_event_type;

#define HOUSENO_MAX_LEN   20
#define PASSWORD_MAX_LEN  20
#define IP_MAX_LEN        16
#define DATA_MAX_LEN      512

typedef struct _linphone_envent {
	fms_list list;
	linphone_event_type type;	
	fms_s8 data[0];
} linphone_event;

linphone_event *linphone_event_init(linphone_event_type type, fms_s8 *data);

fms_void linphone_event_uninit(linphone_event *event);

#ifdef __cplusplus
}
#endif

#endif