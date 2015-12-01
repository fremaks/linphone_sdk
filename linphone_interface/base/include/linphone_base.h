#ifndef __LINPHONE_BASE_H__
#define __LINPHONE_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "linphone_event.h"

#if 0
typedef enum {
	STATE_UNREGISTER,
	STATE_REGISTERED,
	STATE_CALL_PRE,
	STATE_CALL_RUN

} linphone_state;
#endif


typedef fms_void (*linphone_event_callback)(linphone_event *event);

fms_s32 linphone_base_init(const fms_s8 *configfile_name, linphone_event_callback event_callback);

fms_void linphone_base_uninit(fms_s32 exit_status);

fms_void linphone_base_set_native_window_id(fms_uintptr window_id);

fms_void linphone_base_add_event(linphone_event *event);

#if ANDROID

fms_void linphone_base_set_jvm(fms_void *vm);

fms_void linphone_base_openglesdisplay_init(fms_s32 ptr, fms_s32 width, fms_s32 height);

fms_void linphone_base_openglesdisplay_render(fms_s32 ptr);

fms_void linphone_base_fmscamera_put_image(void *env, void *yuvframe, int length);
	
#endif

#ifdef __cplusplus
}
#endif

#endif
