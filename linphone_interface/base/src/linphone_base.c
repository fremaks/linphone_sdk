#include "fms_type.h"
#include "fms_queue.h"
#define LOG_PRINT_THRESHOLD FMS_LOG_INFO
#include "fms_log.h"
#include "linphone_event.h"
#include "linphone_base.h"
#include <pthread.h>
#include "linphonecore.h"
#include "private.h"
#if HAVE_CONFIG_H
#include "config.h"
#endif

#if ANDROID
#include "msjava.h"
#endif
#include <unistd.h>

#define LINPHONE_VERSION "3.8.5"

#define LOOP_TIME 200000
#define VIDEO_ALIVE_CHECK_NUM  1000000/LOOP_TIME


typedef struct _linphone_base_conext {
	LinphoneCore *lc; 
	linphone_event_callback event_callback_ptr;
	fms_bool runing_flag;
	pthread_t event_thread_t;
	fms_queue *event_queue;
	pthread_mutex_t event_queue_lock;
	fms_s8 display_status[512];
} linphone_base_context;

static linphone_base_context *base_ctx = NULL;

static fms_void 
linphone_registration_state_changed(LinphoneCore *lc, LinphoneCall *call, 
									             LinphoneCallState st, const fms_s8 *msg) {
	linphone_event *event = NULL;
	linphone_event_type event_type = LINPHONE_EVENT_UNKNOW;
	fms_s8 event_data[DATA_MAX_LEN] = {0};
	
	if (base_ctx->event_callback_ptr != NULL) {
		switch (st) {
			case LinphoneRegistrationOk : {
				event_type = LINPHONE_REGISTER_RESPBONSE;
				sprintf(event_data, "%d>", FMS_SUCCESS);
				break;
			}
			
			case LinphoneRegistrationFailed : {
				event_type = LINPHONE_REGISTER_RESPBONSE;
				sprintf(event_data, "%d>", FMS_FAILED);
				break;
			}
			
			default :
				break;
		}
		
		if (event_type != LINPHONE_EVENT_UNKNOW) {
			event = linphone_event_init(event_type, event_data);
			(*(base_ctx->event_callback_ptr))(event);
			linphone_event_uninit(event);			
		}
	}

}

static fms_void 
linphone_call_state_changed(LinphoneCore *lc, LinphoneCall *call, 
                                       LinphoneCallState st, const fms_s8 *msg) {
	linphone_event *event = NULL;
	linphone_event_type event_type = LINPHONE_EVENT_UNKNOW;
	fms_s8 event_data[DATA_MAX_LEN] = {0};
	static fms_s32 last_state = LinphoneCallIdle;
	
	FMS_INFO("linphone_call_state_changed->msg=%s[%d]\n", msg, st);
	
	if (base_ctx->event_callback_ptr != NULL) {
		switch (st) {
			case LinphoneCallIncomingReceived : { //remote call,msgÖÐÃ»ÓÐ¶Ô·½µÄ
				event_type = LINPHONE_CALL_REQUEST;
				sprintf(event_data, "%s>", base_ctx->display_status);//houseno
				break;
			}
			
			case LinphoneCallOutgoingRinging : //remote ring
			case LinphoneCallOutgoingEarlyMedia : {
				event_type = LINPHONE_CALL_RESPBONSE;
				sprintf(event_data, "%d>", FMS_SUCCESS);
				break;
			}
			
			case LinphoneCallError : { //local clall error
				event_type = LINPHONE_CALL_RESPBONSE;
				sprintf(event_data, "%d>", CALL_NOT_FIND);				
				break;
			}

			case LinphoneCallStreamsRunning : {
			//case LinphoneCallConnected : { 
				if (strncmp(msg, "Connected", strlen("Connected")) == 0) {//local answer
					event_type = LINPHONE_ANSWER_RESPBONSE;
					sprintf(event_data, "%d>", FMS_SUCCESS);	
				} else { //remote answer
					event_type = LINPHONE_ANSWER_REQUEST;
				}
				
				break;
			}

			case LinphoneCallEnd : {  //¹Ò¶Ï		
				if (strncmp(msg, "Call declined", strlen("Call declined")) == 0) {//remote hangup(not connect)
					//event_type = LINPHONE_CALL_RESPBONSE;
					//sprintf(event_data, "%d>", CALL_NO_ANSWER);
					event_type = LINPHONE_HANGUP_REQUEST;
				} else if (strncmp(msg, "Call ended", strlen("Call ended")) == 0) {//remote hangup(connected)
					event_type = LINPHONE_HANGUP_REQUEST;
				} else if (strncmp(msg, "Call terminated", strlen("Call terminated")) == 0) {//local hangup
					if (LinphoneCallOutgoingProgress == last_state) {
						event_type = LINPHONE_CALL_RESPBONSE;
						sprintf(event_data, "%d>", CALL_NOT_FIND);	
					} else {
						event_type = LINPHONE_HANGUP_RESPBONSE;
						sprintf(event_data, "%d>", FMS_SUCCESS); 
					}
				}
				break;
			}
		}
		
		if (event_type != LINPHONE_EVENT_UNKNOW) {
			event = linphone_event_init(event_type, event_data);
			(*(base_ctx->event_callback_ptr))(event);
			linphone_event_uninit(event);			
		}
	}
	last_state = st;
}

static fms_void
linphone_notify_presence_received(LinphoneCore *lc, LinphoneFriend *fid) {
	FMS_INFO("linphone_notify_presence_received\n");

}

static fms_void
linphone_new_unknown_subscriber(LinphoneCore *lc, LinphoneFriend *lf,
								                const fms_s8 *url) {
	FMS_INFO("linphone_new_unknown_subscriber->url=%s\n", url);

}

static fms_void
linphone_prompt_for_auth(LinphoneCore *lc, const fms_s8 *realm, 
								    const fms_s8 *username, const fms_s8 *domain) {
	FMS_INFO("linphone_prompt_for_auth->realm=%s username=%s domain=%s\n", realm, username, domain);
}

static fms_void 
linphone_display_status(LinphoneCore * lc, const fms_s8 *something) {
	FMS_INFO("linphone_display_status->something=%s\n", something);
	memset(base_ctx->display_status, 0, sizeof(base_ctx->display_status));
	strcpy(base_ctx->display_status, something);
}

static fms_void
linphone_display_something(LinphoneCore * lc, const fms_s8 *something) {
	FMS_INFO("linphone_display_something->something=%s\n", something);
}


static fms_void
linphone_display_warning(LinphoneCore * lc, const fms_s8 *something) {
	FMS_INFO("linphone_display_warning->something=%s\n", something);
}



static fms_void 
linphone_display_url(LinphoneCore * lc, const fms_s8 *something, const fms_s8 *url) {
	FMS_INFO("linphone_display_url->something=%s url=%s\n", something, url);
}


static fms_void
linphone_text_received(LinphoneCore *lc, LinphoneChatRoom *cr,
						       const LinphoneAddress *from, const fms_s8 *msg) {
	FMS_INFO("linphone_text_received->msg=%s\n", msg);
}


static fms_void
linphone_dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf) {
	FMS_INFO("linphone_dtmf_received->dtmf=%d\n", dtmf);
}

static fms_void
linphone_display_refer (LinphoneCore * lc, const fms_s8 *refer_to) {
	FMS_INFO("linphone_display_refer->refer_to=%s\n", refer_to);
}

static fms_void
linphone_transfer_state_changed(LinphoneCore *lc, LinphoneCall *call, 
                                             LinphoneCallState new_call_state) {
	FMS_INFO("linphone_transfer_state_changed->new_call_state=%d\n", new_call_state);
}

static fms_void 
linphone_call_encryption_changed(LinphoneCore *lc, LinphoneCall *call, 
                                              fms_bool encrypted, const fms_s8 *auth_token) {
	FMS_INFO("linphone_call_encryption_changed->auth_token=%s\n", auth_token);
}


static fms_s32  
linphone_register(LinphoneCore *lc, const fms_s8 *username, const fms_s8 *password, 
				  const fms_s8* proxy) {
	fms_s8 identity[HOUSENO_MAX_LEN + IP_MAX_LEN + 10] = {0};
	LinphoneProxyConfig *cfg = NULL;
	LinphoneAuthInfo *info = NULL;
	
	FMS_INFO("linphone_register:username=%s password=%s proxy=%s\n", username, password, proxy);

	linphone_core_clear_proxy_config(lc);

	info = linphone_auth_info_new(username, NULL, password, NULL, NULL, proxy);
	linphone_core_add_auth_info(lc, info);
	linphone_auth_info_destroy(info);

	cfg=linphone_proxy_config_new();
	
	sprintf(identity, "<sip:%s@%s>", username, proxy);	
	//linphone_core_set_sip_transports(lc,&transport);
	linphone_proxy_config_set_expires(cfg, 300);//default 300s
	FMS_EQUAL_RETURN_VALUE(linphone_proxy_config_set_identity(cfg, identity), FMS_FAILED, FMS_FAILED);
	FMS_EQUAL_RETURN_VALUE(linphone_proxy_config_set_server_addr(cfg, proxy), FMS_FAILED, FMS_FAILED);
	linphone_proxy_config_enable_register(cfg, FMS_TRUE);
	linphone_proxy_config_enable_publish(cfg, FMS_TRUE);
	FMS_EQUAL_RETURN_VALUE(linphone_core_add_proxy_config(lc,cfg), FMS_FAILED, FMS_FAILED);
	linphone_core_set_default_proxy(lc, cfg);

	return FMS_SUCCESS;
}

static fms_s32 
linphone_call(LinphoneCore *lc, fms_s8 *args, fms_bool has_video, fms_bool early_media) {
	LinphoneCall *call = NULL;
	LinphoneCallParams *cp = linphone_core_create_default_call_parameters(lc);
	fms_s32 ret = FMS_FAILED;

	FMS_INFO("linphone_call:arg=%s\n", args);
	if (linphone_core_in_call(lc)) {
		FMS_ERROR("Terminate or hold on the current call first\n");
		return ret;
	}
	

	linphone_call_params_enable_video(cp, has_video);
	linphone_call_params_enable_early_media_sending(cp, early_media);

	if (NULL == (call = linphone_core_invite_with_params(lc, args, cp))) {
		FMS_ERROR("Error from linphone_core_invite.\n");
	} else {
		ret = FMS_SUCCESS;
		//snprintf(callee_name,sizeof(callee_name),"%s",args);
	}

	linphone_call_params_destroy(cp);

	return ret;
}

static fms_s32 
linphone_hangup(LinphoneCore *lc) {
	FMS_EQUAL_RETURN_VALUE(linphone_core_get_calls(lc), NULL, FMS_FAILED);

	return linphone_core_terminate_call(lc, NULL);
}

static fms_s32
linphone_answer(LinphoneCore *lc){
	linphone_core_mute_mic(lc, FMS_FALSE);
	return linphone_core_accept_call(lc, NULL);
}

static fms_s32 
linphone_camera_swicth(LinphoneCore *lc, fms_bool swicth) {
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (FMS_FALSE == linphone_core_video_enabled(lc)) {
		FMS_WARN("Video is disabled, re-run linphonec with video\n");
		return FMS_FAILED;
	}

	if (NULL == call){ 
		FMS_WARN("~~~~~~~~~~~~~~~not happen~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	} else{
		const LinphoneCallParams *cp = linphone_call_get_current_params(call);
		linphone_call_enable_camera(call, swicth);
		if (LinphoneCallStreamsRunning == linphone_call_get_state(call)){
			if ((swicth && !linphone_call_params_video_enabled(cp))) {
				/*update the call to add the video stream*/
				LinphoneCallParams *ncp = linphone_call_params_copy(cp);
				linphone_call_params_enable_video(ncp, FMS_TRUE);
				linphone_core_update_call(lc, call, ncp);
				linphone_call_params_destroy(ncp);
				FMS_WARN("Trying to bring up video stream...\n");
			 }
        }
	}

    return FMS_SUCCESS;
}

static fms_void
linphone_event_handle(linphone_event *event) {
	linphone_event *ret_event = NULL;
	fms_s8 ret_data[DATA_MAX_LEN] = {0};
	linphone_event_type ret_type = LINPHONE_EVENT_UNKNOW;
	fms_s32 ret = FMS_FAILED;
	LinphoneCore *lc = base_ctx->lc;
	
	FMS_EQUAL_RETURN(event, NULL);

	FMS_INFO("event->type=%d data=%s\n", event->type, event->data);
	switch (event->type) {	
		case LINPHONE_REGISTER_REQUEST : {
			fms_s8 houseno[HOUSENO_MAX_LEN] = {0};
			fms_s8 password[PASSWORD_MAX_LEN] = {0};
			fms_s8 proxy[IP_MAX_LEN] = {0};
			
			sscanf(event->data, "%[^>]>%[^>]>%[^>]>", houseno, password, proxy);
			ret = linphone_register(lc, houseno, password, proxy);
			if (ret != FMS_SUCCESS) {
				ret_type = LINPHONE_REGISTER_RESPBONSE;
				sprintf(ret_data, "%d>", FMS_FAILED);
			}
			break;
		}

		case LINPHONE_CALL_REQUEST : { //up
			fms_s8 call_houseno[HOUSENO_MAX_LEN] = {0};
			fms_s8 call_args[IP_MAX_LEN] = {0};
			fms_bool has_video = FMS_FALSE;
			fms_bool early_media = FMS_FALSE;
			
			sscanf(event->data, "%[^>]>%d>%d>", call_houseno, &has_video, &early_media);
			ret = linphone_call(lc, call_houseno, has_video, early_media);
			if (ret != FMS_SUCCESS) {
				ret_type = LINPHONE_CALL_RESPBONSE;
				sprintf(ret_data, "%d>", FMS_FAILED);
			}
			break;
		}

		case LINPHONE_CALL_RESPBONSE : { //down
			
			break;
		}
		
		case LINPHONE_ANSWER_REQUEST : {
			ret = linphone_answer(lc);
			//ret_type = LINPHONE_ANSWER_RESPBONSE;
			//sprintf(ret_data, "%d>", ret);
			break;
		}
		
		case LINPHONE_HANGUP_REQUEST : {
			ret = linphone_hangup(lc);
			//ret_type = LINPHONE_HANGUP_RESPBONSE;
			//sprintf(ret_data, "%d>", ret);
			break;
		}

		case LINPHONE_CAMERA_SWITCH_REQUEST : {
			fms_s32 swicth_code = 0;
			sscanf(event->data, "%d>", &swicth_code);
			ret = linphone_camera_swicth(lc, swicth_code == 0 ? FMS_FALSE : FMS_TRUE);
			break;
		}

		case LINPHONE_SNED_DTMF_REQUEST : {
			fms_s8 dtmf = 0;
			sscanf(event->data, "%c>", &dtmf);
			linphone_core_send_dtmf(lc, dtmf);
			break;
		}

		case LINPHONE_SET_DND_SATTE_REQUEST : {
			fms_s32 state = 0;
			ret_type = LINPHONE_SET_DND_SATTE_RESPBONSE;
			sscanf(event->data, "%d>", &state);
			if (0 == state) {
				linphone_core_set_presence_info(lc, 0, NULL, LinphoneStatusOnline);
			} else {
				linphone_core_set_presence_info(lc, 0, NULL, LinphoneStatusDoNotDisturb);
			}
			sprintf(ret_data, "%d>", state);
			break;
		}

		case LINPHONE_GET_DND_SATTE_REQUEST : {
			fms_s32 state = linphone_core_get_presence_info(lc);
			ret_type = LINPHONE_GET_DND_SATTE_RESPBONSE;		
			if (LinphoneStatusDoNotDisturb == state) {
				state = 1;
			} else {
				state = 0;
			}
			sprintf(ret_data, "%d>", state);
			break;
		}
		
		default : {
			FMS_ERROR("unknow event type=%d\n", event->type);
			break;
		}
	}
	
	if (base_ctx->event_callback_ptr != NULL && ret_type != LINPHONE_EVENT_UNKNOW) {
		ret_event = linphone_event_init(ret_type, ret_data);
		(*(base_ctx->event_callback_ptr))(ret_event);
		linphone_event_uninit(ret_event);
	}
}


fms_bool video_stream_alive(VideoStream * stream){
	const rtp_stats_t *stats;
	static struct timeval last_time = {0, 0};
	struct timeval cur_time = {0, 0};
	static fms_s8 fail_count = 0;
	static fms_bool video_alive_flag = FMS_TRUE;
	
	gettimeofday(&cur_time, NULL);
	
	stats=rtp_session_get_stats(stream->ms.sessions.rtp_session);
	if (stats->recv != 0) {
		if (stats->recv != stream->ms.last_packet_count){
			stream->ms.last_packet_count=stats->recv;
			last_time = cur_time;
		}
		if (!video_alive_flag) {
			video_alive_flag = FMS_TRUE;
		}
		fail_count = 0;
	} else {
		return FMS_TRUE;
	}
	
	if (last_time.tv_sec != cur_time.tv_sec || last_time.tv_usec != cur_time.tv_usec) {
		if (video_alive_flag) {
			if(++fail_count == VIDEO_ALIVE_CHECK_NUM) {
				video_alive_flag = FMS_FALSE;
				return FMS_FALSE;
				
			} 	
		}
	}


	return FMS_TRUE;	
}



void *linphone_event_thread(void *arg) {
	linphone_event *event = NULL;
	fms_list *list = NULL;
	LinphoneCore *lc = base_ctx->lc;
	LinphoneCall *call = NULL;
	
	FMS_DEBUG("linphone_event_thread start\n");
	
	while (base_ctx->runing_flag) {
		//ÂÖÑ¯´¦ÀíÊÂ¼þ¶ÓÁÐ£¬²ÉÓÃÌõ¼þ±äÁ¿£¬»òÕß¸ü¼Ó¸ß¼¶µÄÓÃ·¨å
		pthread_mutex_lock(&base_ctx->event_queue_lock);
		list = fms_dequeue(base_ctx->event_queue);
		pthread_mutex_unlock(&base_ctx->event_queue_lock);
		if (list != NULL) {
			event = fms_list_data(list, linphone_event, list);
			linphone_event_handle(event);
			linphone_event_uninit(event);
		}
		linphone_core_iterate(lc);
		
		call = linphone_core_get_current_call(lc);
		if (call != NULL && call->videostream != NULL && call->videostream->ms.state == MSStreamStarted
			&&(call->state == LinphoneCallIncomingReceived 
			|| call->state == LinphoneCallConnected 
			|| call->state == LinphoneCallOutgoingRinging
			|| call->state == LinphoneCallOutgoingEarlyMedia
			|| call->state == LinphoneCallStreamsRunning)) {	
			
			if (!video_stream_alive(call->videostream)) {
				if (base_ctx->event_callback_ptr != NULL) {
					linphone_event *callback_event = 
						linphone_event_init(LINPHONE_CAMERA_SWITCH_RESPBONSE, "0>");
					(*(base_ctx->event_callback_ptr))(callback_event);
					linphone_event_uninit(callback_event);
				}
			} 

		}
			
		usleep(LOOP_TIME);
	}

	linphone_core_terminate_all_calls(lc);
	linphone_core_destroy(lc);
	free(base_ctx);//need handle queue
	base_ctx = NULL;
	
	return (void *)0;
}

extern void libmswebrtc_init();

fms_s32 linphone_base_init(const fms_s8 *configfile_name, 
								 linphone_event_callback event_callback) {
	LinphoneCoreVTable linphone_vtable = {0};
	
	if (base_ctx != NULL) {
		FMS_ERROR("linphone base has aleardy init\n");
		return FMS_FAILED;
	}
	
	base_ctx = (linphone_base_context *)malloc(sizeof(linphone_base_context));
	FMS_ASSERT(base_ctx != NULL);
	
	
	base_ctx->event_callback_ptr = event_callback;
	linphone_vtable.registration_state_changed = linphone_registration_state_changed;
	linphone_vtable.call_state_changed = linphone_call_state_changed;
	linphone_vtable.notify_presence_received = linphone_notify_presence_received;
	linphone_vtable.new_subscription_requested = linphone_new_unknown_subscriber;
	linphone_vtable.auth_info_requested = linphone_prompt_for_auth;
	linphone_vtable.display_status = linphone_display_status;
	linphone_vtable.display_message = linphone_display_something;
	linphone_vtable.display_warning = linphone_display_warning;
	linphone_vtable.display_url=linphone_display_url;
	linphone_vtable.text_received = linphone_text_received;
	linphone_vtable.dtmf_received = linphone_dtmf_received;
	linphone_vtable.refer_received = linphone_display_refer;
	linphone_vtable.transfer_state_changed = linphone_transfer_state_changed;
	linphone_vtable.call_encryption_changed = linphone_call_encryption_changed;

//log swicth?	
#if 0
	linphonec_parse_cmdline(argc, argv);
	if (trace_level > 0)
	{
		if (logfile_name != NULL)
			mylogfile = fopen (logfile_name, "w+");

		if (mylogfile == NULL)
		{
			mylogfile = stdout;
			fprintf (stderr,
				 "INFO: no logfile, logging to stdout\n");
		}
		linphone_core_enable_logs(mylogfile);
	}
	else
	{
		linphone_core_disable_logs();
	}
	
#endif
	/*
	 * Initialize auth stack
	 */
	//auth_stack.nitems=0; ???


	base_ctx->lc = linphone_core_new(&linphone_vtable, configfile_name, NULL, NULL);
	libmswebrtc_init();
	linphone_core_set_user_agent(base_ctx->lc, "Linphone_base",  LINPHONE_VERSION);
	//linphone_core_set_zrtp_secrets_file(linphonec, zrtpsecrets);
	//linphone_core_set_user_certificates_path(linphonec,usr_certificates_path);
	//linphone_core_enable_video_preview(linphonec,preview_enabled);
	//linphone_core_set_preview_video_size(base_ctx->lc, vsize);

	
	base_ctx->event_queue = fms_queue_new(); 
	pthread_mutex_init(&base_ctx->event_queue_lock, NULL);
	base_ctx->runing_flag = FMS_TRUE;
	pthread_create(&base_ctx->event_thread_t, NULL, linphone_event_thread, NULL);

	return FMS_SUCCESS;
}


fms_void linphone_base_set_native_window_id(fms_uintptr window_id) {
	fms_bool display_enable = linphone_core_video_display_enabled(base_ctx->lc);

	if (display_enable && window_id != 0) {
        linphone_core_set_native_video_window_id(base_ctx->lc, window_id);
    }
}

fms_void linphone_base_add_event(linphone_event *event) {
	FMS_EQUAL_RETURN(event, NULL);

	pthread_mutex_lock(&base_ctx->event_queue_lock);
	fms_enqueue(base_ctx->event_queue, &event->list);
	pthread_mutex_unlock(&base_ctx->event_queue_lock);
}



fms_void linphone_base_uninit(fms_s32 exit_status) {

	if (NULL == base_ctx) {
		FMS_ERROR("linphone has not init\n");
		return;
	}
	base_ctx->runing_flag = FALSE;
	//µÈ´ýÏß³Ì½áÊø
}

#if ANDROID

fms_void linphone_base_set_jvm(fms_void *jvm) {
	ms_set_jvm((JavaVM *)jvm);
}

fms_void linphone_base_openglesdisplay_init(fms_s32 ptr, fms_s32 width, fms_s32 height) {
	OpenGLESDisplay_init(ptr, width, height);
}

fms_void linphone_base_openglesdisplay_render(fms_s32 ptr) {
	OpenGLESDisplay_render(ptr);
}

extern void fmscamera_put_image(void *env, void *yuvframe, int length); 
fms_void linphone_base_fmscamera_put_image(void *env, void *yuvframe, int length) {
	fmscamera_put_image(env, yuvframe, length);	
}

#endif

