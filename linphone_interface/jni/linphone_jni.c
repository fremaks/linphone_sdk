#include <jni.h>
#include "linphone_base.h"
#include "linphone_event.h"
#include "fms_type.h"
#include "fms_log.h"


#define NELEM(x)                   (sizeof(x)/sizeof((x)[0]))
#define LINPHONE_INTERFACE_CLASS   "com/example/linphone/LinphoneInterface"
#define FMS_CAMERA_CLASS       	   "com/example/linphone/FmsCamera"
#define H264_CODEC_CLASS           "com/example/linphone/H264Codec"
#define VIDEO_DISPLAY_CLASS    	   "com/example/linphone/VideoDisplay"

typedef struct _linphone_jni_context {
	jobject interface_obj;
	jmethodID callback_method_id;
} linphone_jni_context;

extern jclass h264_codec_class;
extern jclass video_display_class;
extern jclass fms_camera_class;
static linphone_jni_context *jni_ctx = NULL;

static fms_void 
linphone_jni_event_callback(linphone_event *event) {
	JNIEnv *env = 0;
    JavaVM *jvm = ms_get_jvm();
	jstring data = 0;

	FMS_EQUAL_RETURN(event, NULL);
	FMS_EQUAL_RETURN(jvm, NULL);
	//FMS_EQUAL_RETURN(jni_ctx, NULL);
	
	FMS_WARN("linphone_jni_event_callback:type=%d data=%s\n", event->type, event->data);	

	(*jvm)->AttachCurrentThread(jvm, &env, NULL);

	data = (*env)->NewStringUTF(env, event->data);
	(*env)->CallVoidMethod(env, jni_ctx->interface_obj, jni_ctx->callback_method_id,
	 						   	 event->type, data);
	
	(*env)->DeleteLocalRef(env, data);
	(*jvm)->DetachCurrentThread(jvm);

}

JNIEXPORT jint JNICALL 
linphone_jni_init(JNIEnv* env, jobject thiz, jstring jconfigfile_name) {
	jint ret = FMS_FAILED;
	const fms_s8 *configfile_name = NULL;
	jclass interface_class = 0;
	
	if (jni_ctx != NULL) {
		FMS_WARN("linphone jni has aleardy init\n");
		return ret;
	}

	jni_ctx = (linphone_jni_context *)malloc(sizeof(linphone_jni_context));
	FMS_ASSERT(jni_ctx != NULL);
	jni_ctx->interface_obj = 0;
	jni_ctx->callback_method_id = 0;

	interface_class = (*env)->GetObjectClass(env, thiz);
	jni_ctx->interface_obj = (*env)->NewGlobalRef(env, thiz);
 	jni_ctx->callback_method_id = (*env)->GetMethodID(env, interface_class,
                                            "linphoneCallback", "(ILjava/lang/String;)V");
	(*env)->DeleteLocalRef(env, interface_class);
	
	configfile_name = (*env)->GetStringUTFChars(env, jconfigfile_name, NULL);
	ret = linphone_base_init(configfile_name, linphone_jni_event_callback);
	(*env)->ReleaseStringUTFChars(env, jconfigfile_name, configfile_name);

	return ret;
}

JNIEXPORT fms_void JNICALL 
linphone_jni_uninit(JNIEnv* env, jobject thiz, jint jexit_status) {
	FMS_EQUAL_RETURN(jni_ctx, NULL);
	
	free(jni_ctx);
	jni_ctx = NULL;

	linphone_base_uninit(jexit_status);
}

JNIEXPORT fms_void JNICALL 
linphone_jni_set_native_window_id(JNIEnv* env, jobject thiz, jobject window_id) {
	FMS_EQUAL_RETURN(jni_ctx, NULL);
	
	if (window_id != 0) {
		window_id = (*env)->NewGlobalRef(env, window_id);
	}
	linphone_base_set_native_window_id((fms_uintptr)window_id);
}

JNIEXPORT fms_void JNICALL 
linphone_jni_add_event(JNIEnv* env, jobject thiz, jint jevent_type, 
								jstring jevent_data) {
	const fms_s8* event_data = NULL; 
	linphone_event *event = NULL;
	
	FMS_EQUAL_RETURN(jni_ctx, NULL);
	
	event_data = (*env)->GetStringUTFChars(env, jevent_data, NULL);
	FMS_WARN("linphone_jni_add_event->[%d]event_data=%s\n", jevent_type, event_data);
	event = linphone_event_init(jevent_type, event_data);
	linphone_base_add_event(event);
	
	(*env)->ReleaseStringUTFChars(env, jevent_data, event_data);
}

JNIEXPORT fms_void JNICALL 
linphone_jni_openglesdisplay_init(JNIEnv* env, jobject thiz, jint ptr, jint width, 
								             jint height) {
	linphone_base_openglesdisplay_init(ptr, width, height);
}

JNIEXPORT fms_void JNICALL 
linphone_jni_openglesdisplay_render(JNIEnv* env, jobject thiz, jint ptr) {
	linphone_base_openglesdisplay_render(ptr);
}

JNIEXPORT fms_void JNICALL 
linphone_jni_fmscamera_put_image(JNIEnv *env, jobject thiz, jbyteArray jyuvframe, 
                                                jint length){

	linphone_base_fmscamera_put_image((void *)env, (void *)&jyuvframe, length);
}

static JNINativeMethod linphone_interface_methods[] = { 
	{"linphone_init", "(Ljava/lang/String;)I", (fms_void *)linphone_jni_init},
	{"linphone_uninit", "(I)V", (fms_void *)linphone_jni_uninit},
	{"linphone_set_native_window_id", "(L"VIDEO_DISPLAY_CLASS";)V",
	                            (fms_void *)linphone_jni_set_native_window_id},
	{"linphone_add_event", "(ILjava/lang/String;)V", (fms_void *)linphone_jni_add_event},
	{"openglesdisplay_init", "(III)V", (fms_void *)linphone_jni_openglesdisplay_init},
	{"openglesdisplay_render", "(I)V", (fms_void *)linphone_jni_openglesdisplay_render},
	{"fmscamera_put_image", "([BI)V", (fms_void *)linphone_jni_fmscamera_put_image}
};


static fms_s32
register_native_methods(JNIEnv *env, const fms_s8 *class_name,
					 const JNINativeMethod* methods, const fms_s32 methods_num) {

    int ret = FMS_FAILED;  
    jclass dst_class = 0;
	
    dst_class = (*env)->FindClass(env, class_name);  
    if (NULL == dst_class) {  
 	    FMS_ERROR("register_native_methods unable to find class %s\n", class_name);  
        return ret;  
    }
	
    if ((*env)->RegisterNatives(env, dst_class, methods, methods_num) < 0) {  
        FMS_ERROR("register_native_methods failed for %s \n", class_name);  
        return ret;  
    }  
	
	(*env)->DeleteLocalRef(env, dst_class);
	
	ret = FMS_SUCCESS;

	return ret; 
}


static jclass 
get_global_class(JNIEnv *env, fms_s8 *class_name) {
	jclass local_class = 0;
	jclass global_class = 0;
	
	//FMS_EQUAL_RETURN(env, NULL);
	FMS_EQUAL_RETURN(class_name, NULL);
	
	local_class = (*env)->FindClass(env, class_name);
	global_class = (jclass)(*env)->NewGlobalRef(env, local_class);
	(*env)->DeleteLocalRef(env, local_class);
	
	return global_class;
}


JNIEXPORT jint JNICALL  
JNI_OnLoad(JavaVM *jvm, void *reserved) {
	JNIEnv* env = NULL;
	jint result = -1;

	if ((*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		FMS_ERROR("GetEnv failed\n");
        goto fail;
    }
	FMS_ASSERT(env != NULL);

	if (register_native_methods(env, LINPHONE_INTERFACE_CLASS, linphone_interface_methods, 
	                          NELEM(linphone_interface_methods)) != FMS_SUCCESS) {
		FMS_ERROR("registerNativeMethods(%s) failed\n", LINPHONE_INTERFACE_CLASS);
		goto fail;
	}
	
	h264_codec_class = get_global_class(env, H264_CODEC_CLASS);
	video_display_class = get_global_class(env, VIDEO_DISPLAY_CLASS);
	fms_camera_class = get_global_class(env, FMS_CAMERA_CLASS);
	linphone_base_set_jvm(jvm);
    result = JNI_VERSION_1_4;
fail:		
	return JNI_VERSION_1_4;
}

