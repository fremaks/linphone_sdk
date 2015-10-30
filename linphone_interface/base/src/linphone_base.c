#include "fms_type.h"
#include "fms_queue.h"
#define LOG_PRINT_THRESHOLD FMS_LOG_INFO
#include "fms_log.h"
#include "linphone_event.h"
#include "linphone_base.h"
#include <pthread.h>
#include <linphonecore.h>
#if HAVE_CONFIG_H
#include "config.h"
#endif

#if ANDROID
#include "msjava.h"
#endif
#include <unistd.h>

#define LINPHONE_VERSION "3.8.5"

static LinphoneCore *g_linphone_core = NULL;
static linphone_event_callback g_event_callback = NULL;
static fms_bool g_linphone_runing_flag = FMS_FALSE; 
static pthread_t g_linphone_event_thread_t;
static fms_queue *g_linphone_event_queue = NULL;
static pthread_mutex_t g_linphone_event_queue_lock;
static fms_s8 g_display_status[512] = {0};

static fms_void 
linphone_registration_state_changed(LinphoneCore *lc, LinphoneCall *call, 
									             LinphoneCallState st, const fms_s8 *msg) {
	linphone_event *event = NULL;
	linphone_event_type event_type = LINPHONE_EVENT_UNKNOW;
	fms_s8 event_data[DATA_MAX_LEN] = {0};
	
	if (g_event_callback != NULL) {
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
			(*g_event_callback)(event);
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
	
	FMS_INFO("linphone_call_state_changed->msg=%s[%d]\n", msg, st);
	
	if (g_event_callback != NULL) {
		switch (st) {
			case LinphoneCallIncomingReceived : { //remote call,msg��û�жԷ���
				event_type = LINPHONE_CALL_REQUEST;
				sprintf(event_data, "%s>", g_display_status);//houseno
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

			case LinphoneCallEnd : {  //�Ҷ�		
				if (strncmp(msg, "Call declined", strlen("Call declined")) == 0) {//remote hangup(not connect)
					//event_type = LINPHONE_CALL_RESPBONSE;
					//sprintf(event_data, "%d>", CALL_NO_ANSWER);
					event_type = LINPHONE_HANGUP_REQUEST;
				} else if (strncmp(msg, "Call ended", strlen("Call ended")) == 0) {//remote hangup(connected)
					event_type = LINPHONE_HANGUP_REQUEST;
				} else if (strncmp(msg, "Call terminated", strlen("Call terminated")) == 0) {//local hangup
					event_type = LINPHONE_HANGUP_RESPBONSE;
					sprintf(event_data, "%d>", FMS_SUCCESS);
				}
				break;
			}
		}
		
		if (event_type != LINPHONE_EVENT_UNKNOW) {
			event = linphone_event_init(event_type, event_data);
			(*g_event_callback)(event);
			linphone_event_uninit(event);			
		}
	}
	
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
	memset(g_display_status, 0, sizeof(g_display_status));
	strcpy(g_display_status, something);
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
	linphone_core_set_default_proxy(lc,cfg);

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
	return linphone_core_accept_call(lc, NULL);
}

static fms_void
linphone_event_handle(linphone_event *event) {
	linphone_event *ret_event = NULL;
	fms_s8 ret_data[DATA_MAX_LEN] = {0};
	linphone_event_type ret_type = LINPHONE_EVENT_UNKNOW;
	fms_s32 ret = FMS_FAILED;
	
	FMS_EQUAL_RETURN(event, NULL);

	FMS_INFO("event->type=%d data=%s\n", event->type, event->data);
	switch (event->type) {	
		case LINPHONE_REGISTER_REQUEST : {
			fms_s8 houseno[HOUSENO_MAX_LEN] = {0};
			fms_s8 password[PASSWORD_MAX_LEN] = {0};
			fms_s8 proxy[IP_MAX_LEN] = {0};
			
			sscanf(event->data, "%[^>]>%[^>]>%[^>]>", houseno, password, proxy);
			ret = linphone_register(g_linphone_core, houseno, password, proxy);
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
			ret = linphone_call(g_linphone_core, call_houseno, has_video, early_media);
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
			ret = linphone_answer(g_linphone_core);
			//ret_type = LINPHONE_ANSWER_RESPBONSE;
			//sprintf(ret_data, "%d>", ret);
			break;
		}
		
		case LINPHONE_HANGUP_REQUEST : {
			ret = linphone_hangup(g_linphone_core);
			//ret_type = LINPHONE_HANGUP_RESPBONSE;
			//sprintf(ret_data, "%d>", ret);
			break;
		}
		
		default : {
			FMS_ERROR("unknow event type=%d\n", event->type);
			break;
		}
	}
	
	if (g_event_callback != NULL && ret_type != LINPHONE_EVENT_UNKNOW) {
		ret_event = linphone_event_init(ret_type, ret_data);
		(*g_event_callback)(ret_event);
		linphone_event_uninit(ret_event);
	}
}


//��ʱ����???
void *linphone_event_thread(void *arg) {
	linphone_event *event = NULL;
	fms_list *list = NULL;

	FMS_DEBUG("linphone_event_thread start\n");
	
	while (g_linphone_runing_flag) {
		//��ѯ�����¼����У������������������߸��Ӹ߼����÷��
		pthread_mutex_lock(&g_linphone_event_queue_lock);
		list = fms_dequeue(g_linphone_event_queue);
		pthread_mutex_unlock(&g_linphone_event_queue_lock);
		if (list != NULL) {
			printf("linphone_event_thread 2\n");
			event = fms_list_data(list, linphone_event, list);
			linphone_event_handle(event);
			linphone_event_uninit(event);
		}
		linphone_core_iterate(g_linphone_core);
		usleep(200000);
	}

	return (void *)0;
}



fms_s32 linphone_base_init(const fms_s8 *configfile_name, linphone_event_callback event_callback, 
					  fms_bool vcap_enable, fms_bool vdisplay_enable, fms_uintptr window_id) {

	LinphoneCoreVTable linphone_vtable = {0};

	if (g_linphone_runing_flag) {
		FMS_ERROR("linphone has aleardy init\n");
		return FMS_FAILED;
	}

	g_event_callback = event_callback;
	linphone_vtable.registration_state_changed = linphone_registration_state_changed;
	linphone_vtable.call_state_changed = linphone_call_state_changed;
	linphone_vtable.notify_presence_received = linphone_notify_presence_received;
	linphone_vtable.new_subscription_requested = linphone_new_unknown_subscriber;
	linphone_vtable.auth_info_requested = linphone_prompt_for_auth;
	linphone_vtable.display_status = linphone_display_status;
	linphone_vtable.display_message = linphone_display_something;
	linphone_vtable.display_warning = linphone_display_warning;
	linphone_vtable.display_url=linphone_display_url;
	linphone_vtable.text_received=linphone_text_received;
	linphone_vtable.dtmf_received=linphone_dtmf_received;
	linphone_vtable.refer_received=linphone_display_refer;
	linphone_vtable.transfer_state_changed=linphone_transfer_state_changed;
	linphone_vtable.call_encryption_changed=linphone_call_encryption_changed;

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


	g_linphone_core = linphone_core_new(&linphone_vtable, configfile_name, NULL, NULL);

	linphone_core_set_user_agent(g_linphone_core, "Linphone_base",  LINPHONE_VERSION);
	//linphone_core_set_zrtp_secrets_file(linphonec, zrtpsecrets);
	//linphone_core_set_user_certificates_path(linphonec,usr_certificates_path);
	FMS_WARN("linphone_base_init: cap=%d display=%d\n", vcap_enable, vdisplay_enable);
	linphone_core_enable_video_capture(g_linphone_core, vcap_enable);
	linphone_core_enable_video_display(g_linphone_core, vdisplay_enable);
	//set window ID ?
	if (vdisplay_enable && window_id != 0) {
		printf("Setting window_id: 0x%x\n", window_id);
		linphone_core_set_native_video_window_id(g_linphone_core, window_id);
	}
	//linphone_core_enable_video_preview(linphonec,preview_enabled);

	g_linphone_event_queue = fms_queue_new(); 
	pthread_mutex_init(&g_linphone_event_queue_lock, NULL);
	g_linphone_runing_flag = FMS_TRUE;
	pthread_create(&g_linphone_event_thread_t, NULL, linphone_event_thread, NULL);

	return FMS_SUCCESS;
}


fms_void linphone_base_add_event(linphone_event *event) {
	FMS_EQUAL_RETURN(event, NULL);

	pthread_mutex_lock(&g_linphone_event_queue_lock);
	fms_enqueue(g_linphone_event_queue, &event->list);
	pthread_mutex_unlock(&g_linphone_event_queue_lock);
}



fms_void linphone_base_uninit(fms_s32 exit_status) {

	if (!g_linphone_runing_flag) {
		FMS_ERROR("linphone has not init\n");
		return;
	}

	/* Terminate any pending call */
	linphone_core_terminate_all_calls(g_linphone_core);

	linphone_core_destroy(g_linphone_core);
	g_linphone_core = NULL;

	g_linphone_runing_flag = FALSE;
	//�ȴ��߳̽���
}

#if ANDROID
fms_void linphone_base_set_jvm(fms_void *jvm) {
	ms_set_jvm((JavaVM *)jvm);
}
#endif

