/*
mediastreamer2 library - modular sound and video processing and streaming
This is the video capture filter for Android.
It uses one of the JNI wrappers to access Android video capture API.
See:
	org.linphone.mediastream.video.capture.AndroidVideoApi9JniWrapper
	org.linphone.mediastream.video.capture.AndroidVideoApi8JniWrapper
	org.linphone.mediastream.video.capture.AndroidVideoApi5JniWrapper

 * Copyright (C) 2010  Belledonne Communications, Grenoble, France

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

extern "C" {
	#include "mediastreamer2/msvideo.h"
	#include "mediastreamer2/msfilter.h"
	#include "mediastreamer2/mswebcam.h"
	#include "mediastreamer2/msjava.h"
	#include "mediastreamer2/msticker.h"
}

#include <jni.h>
#include "fms_log.h"

#define UNDEFINED_ROTATION -1

struct AndroidReaderContext {
	AndroidReaderContext(MSFilter *f, MSWebCam *cam):filter(f), webcam(cam),frame(0),fps(5){
		ms_message("Creating AndroidReaderContext for Android VIDEO capture filter");
		ms_mutex_init(&mutex,NULL);
		androidCamera = 0;
		previewWindow = 0;
		rotation = rotationSavedDuringVSize = UNDEFINED_ROTATION;
	};

	~AndroidReaderContext(){
		if (frame != 0) {
			freeb(frame);
		}
		ms_mutex_destroy(&mutex);
	};

	MSFrameRateController fpsControl;
	MSAverageFPS averageFps;

	MSFilter *filter;
	MSWebCam *webcam;

	mblk_t *frame;
	float fps;
	MSVideoSize requestedSize, hwCapableSize, usedSize;
	ms_mutex_t mutex;
	int rotation, rotationSavedDuringVSize;
	int useDownscaling;
	char fps_context[64];

	jobject androidCamera;
	jobject previewWindow;
	jclass helperClass;
	jmethodID openID;
	jmethodID closeID;
};

jclass fms_camera_class;

static AndroidReaderContext *readerCtx = NULL;

static void video_capture_detect(MSWebCamManager *obj);

static AndroidReaderContext *getContext(MSFilter *f) {
	return (AndroidReaderContext*) f->data;
}


static void video_capture_init(MSFilter *f) {
	AndroidReaderContext* d = new AndroidReaderContext(f, 0);
	jmethodID constructID = 0;
	JNIEnv *env = NULL;
	JavaVM *jvm = ms_get_jvm();

	f->data = d;
	readerCtx = d;	

	jvm->AttachCurrentThread(&env, NULL);
		
	constructID = env->GetMethodID(fms_camera_class, "<init>", "()V");
	d->androidCamera = env->NewObject(fms_camera_class, constructID);  
	d->androidCamera = (jobject)env->NewGlobalRef(d->androidCamera);

	d->openID = env->GetMethodID(fms_camera_class, "openCamera", "(II)V");
	d->closeID = env->GetMethodID(fms_camera_class, "closeCamera", "()V");
	
	jvm->DetachCurrentThread();	
}

static void video_capture_preprocess(MSFilter *f){
	AndroidReaderContext *d = getContext(f);
	JNIEnv *env = NULL;
	JavaVM *jvm = ms_get_jvm();
	
	jvm->AttachCurrentThread(&env, NULL);

	if (d->openID) {
		ms_mutex_lock(&d->mutex);
		env->CallVoidMethod(d->androidCamera, d->openID, d->usedSize.width, d->usedSize.height);
		ms_mutex_unlock(&d->mutex);
	}
	
	jvm->DetachCurrentThread();	
}

static void video_capture_process(MSFilter *f){
	AndroidReaderContext* d = getContext(f);
	if (d->frame == 0) {
		return;
	}
	ms_mutex_lock(&d->mutex);
	ms_queue_put(f->outputs[0], d->frame);
	d->frame = 0;
	ms_mutex_unlock(&d->mutex);	

}

static void video_capture_postprocess(MSFilter *f){
	AndroidReaderContext *d = getContext(f);
	JNIEnv *env = NULL;
	JavaVM *jvm = ms_get_jvm();
	
	jvm->AttachCurrentThread(&env, NULL);
	
	if (d->closeID) {
		env->CallVoidMethod(d->androidCamera, d->closeID);
	}
	
	jvm->DetachCurrentThread();	
}

static void video_capture_uninit(MSFilter *f) {
	AndroidReaderContext* d = getContext(f);
	delete d;
}


static int video_capture_set_fps(MSFilter *f, void *arg){
	AndroidReaderContext* d = (AndroidReaderContext*) f->data;
	d->fps=*((float*)arg);
	return 0;
}

static int video_capture_get_fps(MSFilter *f, void *arg){
	AndroidReaderContext* d = (AndroidReaderContext*) f->data;
	*((float*)arg) = ms_average_fps_get(&d->averageFps);
	return 0;
}

static int video_capture_set_vsize(MSFilter *f, void* data){
	AndroidReaderContext* d = (AndroidReaderContext*) f->data;
	d->usedSize = *(MSVideoSize*)data;	
	return 0;
}

static int video_capture_get_vsize(MSFilter *f, void* data){
	AndroidReaderContext* d = (AndroidReaderContext*) f->data;
	*(MSVideoSize*)data=d->usedSize;
	return 0;
}

static int video_capture_get_pix_fmt(MSFilter *f, void *data){
	*(MSPixFmt*)data=MS_YUV420P;
	return 0;
}


static MSFilterMethod video_capture_methods[]={
		{	MS_FILTER_SET_FPS,	&video_capture_set_fps},
		{	MS_FILTER_GET_FPS,	&video_capture_get_fps},
		{	MS_FILTER_SET_VIDEO_SIZE, &video_capture_set_vsize},
		{	MS_FILTER_GET_VIDEO_SIZE, &video_capture_get_vsize},
		{	MS_FILTER_GET_PIX_FMT, &video_capture_get_pix_fmt},
		{	0,0 }
};



MSFilterDesc ms_video_capture_desc={
		MS_ANDROID_VIDEO_READ_ID,
		"MSAndroidVideoCapture",
		N_("A filter that captures Android video."),
		MS_FILTER_OTHER,
		NULL,
		0,
		1,
		video_capture_init,
		video_capture_preprocess,
		video_capture_process,
		video_capture_postprocess,
		video_capture_uninit,
		video_capture_methods
};

MS_FILTER_DESC_EXPORT(ms_video_capture_desc)


static void video_capture_cam_init(MSWebCam *cam) {

}




static MSFilter *video_capture_create_reader(MSWebCam *obj){
	ms_message("Instanciating Android VIDEO capture MS filter");

	MSFilter* lFilter = ms_filter_new_from_desc(&ms_video_capture_desc);
	getContext(lFilter)->webcam = obj;
	
	return lFilter;
}


MSWebCamDesc ms_android_video_capture_desc={
		"AndroidVideoCapture",
		&video_capture_detect,
		&video_capture_cam_init,
		&video_capture_create_reader,
		NULL
};

static void video_capture_detect(MSWebCamManager *obj) {
	JNIEnv *env = ms_get_jni_env();
	jmethodID detectID = 0;
	int ret = -1;
	
	detectID = env->GetStaticMethodID(fms_camera_class, "detectCamera", "()I");
		
    ret = env->CallStaticIntMethod(fms_camera_class, detectID);
	if (0 == ret) {
		MSWebCam *cam = ms_web_cam_new(&ms_android_video_capture_desc);
		ms_web_cam_manager_add_cam(obj, cam);
	}
	
}

extern "C" void fmscamera_put_image(void *env, void *yuvframe, int length) {
	JNIEnv *jenv = (JNIEnv *)env; 
	jbyteArray *jyuvframe = (jbyteArray *)yuvframe;
	AndroidReaderContext* d = readerCtx;
	jbyte* jinternal_buff = jenv->GetByteArrayElements(*jyuvframe, 0);
	MSPicture pict;
	
	
	mblk_t *yuv_block = ms_yuv_buf_alloc(&pict, d->usedSize.width, d->usedSize.height);
	memcpy(yuv_block->b_rptr, jinternal_buff, yuv_block->b_wptr - yuv_block->b_rptr);

	ms_mutex_lock(&d->mutex);
	if (d->frame != 0) {
		ms_message("Android video capture: putImage replacing old frame with new one");
		freemsg(d->frame);
		d->frame = 0;
	}
	d->frame = yuv_block;
	ms_mutex_unlock(&d->mutex);
	
	jenv->ReleaseByteArrayElements(*jyuvframe, jinternal_buff, 0);
}

