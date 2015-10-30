#include <jni.h>
#include "linphone_base.h"
#include "linphone_event.h"
#include "fms_type.h"
#include "fms_log.h"


#define NELEM(x)          (sizeof(x)/sizeof((x)[0]))
//#define APP_LAYER_CLASS   "com/lorent/linphone/LinphoneProxy"
#define APP_LAYER_CLASS   "com/example/linphone/MainActivity"
jmethodID g_linphone_callback_methodid;
jobject  g_linphone_proxy;
extern jclass g_h264_codec_class;

static fms_void 
linphone_jni_event_callback(linphone_event *event) {
	JNIEnv *jni_env;
    JavaVM *jvms= ms_get_jvm();
	jstring dataStr;
	
	FMS_WARN("linphone_jni_event_callback:type=%d data=%s\n", event->type, event->data);	
	if (NULL == jvms) {
		return;
	}
	
	(*jvms)->AttachCurrentThread(jvms, &jni_env, NULL);

	dataStr = (*jni_env)->NewStringUTF(jni_env, event->data);
	(*jni_env)->CallStaticVoidMethod(jni_env, g_linphone_proxy, g_linphone_callback_methodid,
	 						   event->type, dataStr);
	(*jni_env)->DeleteLocalRef(jni_env, dataStr);
	(*jvms)->DetachCurrentThread(jvms);

}

JNIEXPORT jint JNICALL 
linphone_jni_init(JNIEnv* env, jobject thiz, jstring jconfigfile_name, 
					  jboolean jvcap_enable, jboolean jvdisplay_enable, jobject obj) {
	jint ret = FMS_FAILED;
	const fms_s8* configfile_name = (*env)->GetStringUTFChars(env, jconfigfile_name, NULL);
	if (obj != NULL) {
		obj = (*env)->NewGlobalRef(env, obj);
	}
	ret = linphone_base_init(configfile_name, linphone_jni_event_callback, 
							jvcap_enable, jvdisplay_enable, (fms_uintptr)obj);
	(*env)->ReleaseStringUTFChars(env, jconfigfile_name, configfile_name);

	return ret;
}

JNIEXPORT fms_void JNICALL 
linphone_jni_uninit(JNIEnv* env, jobject thiz, jint jexit_status) {
	linphone_base_uninit(jexit_status);
}

JNIEXPORT fms_void JNICALL 
linphone_jni_add_event(JNIEnv* env, jobject thiz, jint jevent_type, 
								jstring jevent_data) {//jevent_dataÈç¹ûÎª¿ÕµÄ»°å
	const fms_s8* event_data = (*env)->GetStringUTFChars(env, jevent_data, NULL);

	FMS_WARN("linphone_jni_add_event->[%d]event_data=%s\n", jevent_type, event_data);
	linphone_event *event = linphone_event_init(jevent_type, event_data);
	linphone_base_add_event(event);
	
	(*env)->ReleaseStringUTFChars(env, jevent_data, event_data);
}

static JNINativeMethod methods[] = { 
	{"linphone_init", "(Ljava/lang/String;ZZLcom/example/linphone/AndroidVideoWindowImpl;)I", (void*)linphone_jni_init},
	{"linphone_uninit", "(I)V", (void*)linphone_jni_uninit},
	{"linphone_add_event", "(ILjava/lang/String;)V", (void*)linphone_jni_add_event},
};

static fms_s32
registerNativeMethods(JNIEnv *env, const fms_s8 *className,
					 const JNINativeMethod* methods, const fms_s32 numMethods) {

    int ret = FMS_FAILED;  
    jclass clazz;
	jobject linphone_proxy;
	
    clazz = (*env)->FindClass(env, className);  
    if (NULL == clazz) {  
 	    FMS_ERROR("Native registration unable to find class '%s", className);  
        return ret;  
    }
	
    if ((*env)->RegisterNatives(env, clazz, methods, numMethods) < 0) {  
        FMS_ERROR("RegisterNatives failed for '%s'", className);  
        return ret;  
    }  
	
	linphone_proxy = (*env)->GetObjectClass(env, clazz);
	g_linphone_proxy = (*env)->NewGlobalRef(env, linphone_proxy);
    (*env)->DeleteLocalRef(env, linphone_proxy);
	g_linphone_callback_methodid = (*env)->GetStaticMethodID(env, clazz, "linphoneCallback", "(ILjava/lang/String;)V");
	
	ret = FMS_SUCCESS;

	return ret; 
}


JNIEXPORT jint JNICALL  
JNI_OnLoad(JavaVM *jvm, void *reserved) {
	JNIEnv* env = NULL;
	jint result = -1;
	
	if ((*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		FMS_ERROR("GetEnv failed\n");
        goto fail;
    }
    //assert(env != NULL);
	FMS_ASSERT(env != NULL);
    if (registerNativeMethods(env, APP_LAYER_CLASS, methods, NELEM(methods)) 
								!= FMS_SUCCESS) {
		FMS_ERROR("registerNativeMethods failed");
		goto fail;
	}
	jclass h264_codec_class = (*env)->FindClass(env, "com/example/linphone/H264Codec");
	g_h264_codec_class = (jclass)(*env)->NewGlobalRef(env, h264_codec_class);
	FMS_WARN("JNI_OnLoad->g_h264_codec_class=%d\n", g_h264_codec_class);
	linphone_base_set_jvm(jvm);
    result = JNI_VERSION_1_4;
fail:		
	return JNI_VERSION_1_4;
}

