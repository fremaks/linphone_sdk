/*
mediastreamer2 android video display filter
Copyright (C) 2010 Belledonne Communications SARL (simon.morlat@linphone.org)

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

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msjava.h"
#include "layouts.h"

#include <android/bitmap.h>

#include <dlfcn.h>

#include "fms_log.h"
typedef struct AndroidDisplay{
	jobject android_video_window;
	jobject jbitmap;
	jmethodID get_bitmap_id;
	jmethodID update_id;
	jmethodID request_orientation_id;
	AndroidBitmapInfo bmpinfo;
	MSScalerContext *sws;
	MSVideoSize vsize;
	bool_t orientation_change_pending;
}AndroidDisplay;

struct LntAndroidDisplay_t{
	jclass video_window_class;
	jobject android_video_window;
	jobject jbitmap;
	jmethodID get_bitmap_id;
	jmethodID update_id;
	jmethodID request_orientation_id;
	AndroidBitmapInfo bmpinfo;
};

static int (*sym_AndroidBitmap_getInfo)(JNIEnv *env,jobject bitmap, AndroidBitmapInfo *bmpinfo)=NULL;
static int (*sym_AndroidBitmap_lockPixels)(JNIEnv *env, jobject bitmap, void **pixels)=NULL;
static int (*sym_AndroidBitmap_unlockPixels)(JNIEnv *env, jobject bitmap)=NULL;

jclass g_lntWindowClass;
jobject g_lntVideoWindow; 

static void android_display_init(MSFilter *f){
	AndroidDisplay *ad=(AndroidDisplay*)ms_new0(AndroidDisplay,1);
    JNIEnv *env;
	JavaVM *jvms= ms_get_jvm();
	
	(*jvms)->AttachCurrentThread(jvms, &env, NULL);     
	ad->get_bitmap_id=(*env)->GetMethodID(env,g_lntWindowClass,"getBitmap", "()Landroid/graphics/Bitmap;");
	ad->update_id=(*env)->GetMethodID(env,g_lntWindowClass,"update","()V");
	ad->request_orientation_id=(*env)->GetMethodID(env,g_lntWindowClass,"requestOrientation","(I)V");
	
    (*jvms)->DetachCurrentThread(jvms);
	f->data=ad;
	ad->vsize.width = 800;
	ad->vsize.height = 480;
}

static void android_display_uninit(MSFilter *f){
	AndroidDisplay *ad=(AndroidDisplay*)f->data;
	if (ad->sws){
		ms_scaler_context_free (ad->sws);
		ad->sws=NULL;
	}
	ms_free(ad);
}

static void android_display_preprocess(MSFilter *f){

}

#define LANDSCAPE 0
#define PORTRAIT 1

#if 0
static int vsize_get_orientation(MSVideoSize vs){
	return vs.width>=vs.height ? LANDSCAPE : PORTRAIT;
}

static void select_orientation(AndroidDisplay *ad, MSVideoSize wsize, MSVideoSize vsize){
	int wo,vo;
	JNIEnv *jenv=ms_get_jni_env();
	wo=vsize_get_orientation(wsize);
	vo=vsize_get_orientation(vsize);
	if (wo!=vo){
		ms_message("Requesting orientation change !");
		(*jenv)->CallVoidMethod(jenv,ad->android_video_window,ad->request_orientation_id,vo);
		ad->orientation_change_pending=TRUE;
	}
}
#endif


static void android_display_process(MSFilter *f){
	AndroidDisplay *ad=(AndroidDisplay*)f->data;
	MSPicture pic;
	mblk_t *m;
    JNIEnv *jenv;
	JavaVM *jvms= ms_get_jvm();
	int err;
	(*jvms)->AttachCurrentThread(jvms, &jenv, NULL);  
	ms_filter_lock(f);
    FMS_WARN("0000000000000000000000android_display_process 1\n");
    if (g_lntVideoWindow!=NULL && ad->jbitmap == NULL) {
		FMS_WARN("0000000000000000000000android_display_process 2\n");
        ad->jbitmap=(*jenv)->CallObjectMethod(jenv,g_lntVideoWindow,ad->get_bitmap_id);
        ad->orientation_change_pending=FALSE;
        if (ad->jbitmap!=NULL){
            ad->jbitmap = (*jenv)->NewGlobalRef(jenv, ad->jbitmap);
		    err=sym_AndroidBitmap_getInfo(jenv,ad->jbitmap,&ad->bmpinfo);
		    if (err!=0){
			    FMS_WARN("AndroidBitmap_getInfo() failed.");
			    ad->jbitmap=0;
			    ms_filter_unlock(f);
			    return;
		    }
	    }
        if (ad->jbitmap!=NULL) FMS_WARN("New java bitmap given with w=%i,h=%i,stride=%i,format=%i",
	           ad->bmpinfo.width,ad->bmpinfo.height,ad->bmpinfo.stride,ad->bmpinfo.format);
    }
    FMS_WARN("0000000000000000000000android_display_process 3\n");
	if (ad->jbitmap!=0 && !ad->orientation_change_pending){
		FMS_WARN("0000000000000000000000android_display_process 4\n");
		if ((m=ms_queue_peek_last(f->inputs[0]))!=NULL){
			FMS_WARN("0000000000000000000000android_display_process 5\n");
			if (ms_yuv_buf_init_from_mblk (&pic,m)==0){
				MSVideoSize wsize={ad->bmpinfo.width,ad->bmpinfo.height};
				//MSVideoSize vsize={pic.w, pic.h};
				MSVideoSize vsize={800, 480};
				MSRect vrect;
				MSPicture dest={0};
				void *pixels=NULL;
				FMS_WARN("0000000000000000000000android_display_process 6, wsize=[%d:%d] vsize=[%d:%d]\n",
						wsize.width, wsize.height, vsize.width, vsize.height);
			
				if (!ms_video_size_equal(vsize,ad->vsize)){
					FMS_WARN("Video to display has size %ix%i\n",vsize.width,vsize.height);
					ad->vsize=vsize;
					if (ad->sws){
						ms_scaler_context_free(ad->sws);
						ad->sws=NULL;
					}
					/*select_orientation(ad,wsize,vsize);*/
				}
				FMS_WARN("0000000000000000000000android_display_process 6-0\n");
				ms_layout_compute(wsize,vsize,vsize,-1,0,&vrect, NULL);
				FMS_WARN("0000000000000000000000android_display_process 6-00, vrect=%d:%d\n", vrect.w,vrect.h);
				if (ad->sws==NULL){
					FMS_WARN("0000000000000000000000android_display_process 6-1\n");
					ad->sws=ms_scaler_create_context (vsize.width,vsize.height,MS_YUV420P,
					                           vrect.w,vrect.h,MS_RGB565,MS_SCALER_METHOD_BILINEAR);
					if (ad->sws==NULL){
						ms_fatal("Could not obtain sws context !");
					}
				}
				FMS_WARN("0000000000000000000000android_display_process 6-2\n");
				if (sym_AndroidBitmap_lockPixels(jenv,ad->jbitmap,&pixels)==0){
					FMS_WARN("0000000000000000000000android_display_process 6-3\n");
					if (pixels!=NULL){
						FMS_WARN("0000000000000000000000android_display_process 6-4\n");
						dest.planes[0]=(uint8_t*)pixels+(vrect.y*ad->bmpinfo.stride)+(vrect.x*2);
						dest.strides[0]=ad->bmpinfo.stride;
						FMS_WARN("0000000000000000000000android_display_process 6-41\n");
						ms_scaler_process(ad->sws,pic.planes,pic.strides,dest.planes,dest.strides);
					}else ms_warning("Pixels==NULL in android bitmap !");
					FMS_WARN("0000000000000000000000android_display_process 6-5\n");
					sym_AndroidBitmap_unlockPixels(jenv,ad->jbitmap);
				}else{
					FMS_WARN("0000000000000000000000android_display_process 6-6\n");
					ms_error("AndroidBitmap_lockPixels() failed !");
				}
				FMS_WARN("**********************android_display_process:<%d:%d>\n", ad->bmpinfo.width,ad->bmpinfo.height);
				(*jenv)->CallVoidMethod(jenv,g_lntVideoWindow,ad->update_id);
			}
		}
	}
	ms_filter_unlock(f);

	ms_queue_flush(f->inputs[0]);
	//ms_queue_flush(f->inputs[1]);
  (*jvms)->DetachCurrentThread(jvms);
}

static int android_display_set_window(MSFilter *f, void *arg){
#if 0
	AndroidDisplay *ad=(AndroidDisplay*)f->data;
	unsigned long id=*(unsigned long*)arg;
	int err;
	JNIEnv *jenv=ms_get_jni_env();
	jobject window=(jobject)id;

	ms_filter_lock(f);
	if (window!=NULL)
		ad->jbitmap=(*jenv)->CallObjectMethod(jenv,window,ad->get_bitmap_id);
	else
		ad->jbitmap=NULL;
	ad->android_video_window=window;
	if (ad->jbitmap!=NULL){
		err=sym_AndroidBitmap_getInfo(jenv,ad->jbitmap,&ad->bmpinfo);
		if (err!=0){
			ms_error("AndroidBitmap_getInfo() failed.");
			ad->jbitmap=0;
			ms_filter_unlock(f);
			return -1;
		}
	}
	if (ad->sws){
		ms_scaler_context_free(ad->sws);
		ad->sws=NULL;
	}
	ad->orientation_change_pending=FALSE;
	ms_filter_unlock(f);
	if (ad->jbitmap!=NULL) ms_message("New java bitmap given with w=%i,h=%i,stride=%i,format=%i",
	           ad->bmpinfo.width,ad->bmpinfo.height,ad->bmpinfo.stride,ad->bmpinfo.format);
#endif
	return 0;
}

static MSFilterMethod methods[]={
	{	MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID , android_display_set_window },
	{	0, NULL}
};

MSFilterDesc ms_android_display_desc={
	.id=MS_ANDROID_DISPLAY_ID,
	.name="MSAndroidDisplay",
	.text="Video display filter for Android.",
	.category=MS_FILTER_OTHER,
	.ninputs=1, /*number of inputs*/
	.noutputs=0, /*number of outputs*/
	.init=android_display_init,
	.preprocess=android_display_preprocess,
	.process=android_display_process,
	.uninit=android_display_uninit,
	.methods=methods
};


bool_t libmsandroiddisplay_init(MSFactory *factory){
	/*See if we can use AndroidBitmap_* symbols (only since android 2.2 normally)*/
	void *handle=NULL;
	handle=dlopen("libjnigraphics.so",RTLD_LAZY);
	if (handle!=NULL){
		sym_AndroidBitmap_getInfo=dlsym(handle,"AndroidBitmap_getInfo");
		sym_AndroidBitmap_lockPixels=dlsym(handle,"AndroidBitmap_lockPixels");
		sym_AndroidBitmap_unlockPixels=dlsym(handle,"AndroidBitmap_unlockPixels");

		if (sym_AndroidBitmap_getInfo==NULL || sym_AndroidBitmap_lockPixels==NULL
			|| sym_AndroidBitmap_unlockPixels==NULL){
			ms_warning("AndroidBitmap not available.");
		}else{
			JNIEnv *jenv = ms_get_jni_env();
			jclass wc; 
			jmethodID windowInitMethod;
			jobject lntVideoWindow;
			ms_factory_register_filter(factory,&ms_android_display_desc);
            wc = (jclass)(*jenv)->FindClass(jenv, "com/example/linphone/LntVideoWindow");
            g_lntWindowClass = (jclass)(*jenv)->NewGlobalRef(jenv, wc);
			(*jenv)->DeleteLocalRef(jenv, wc);

			windowInitMethod = (*jenv)->GetMethodID(jenv, g_lntWindowClass, "<init>", "()V");
			lntVideoWindow = (*jenv)->NewObject(jenv, g_lntWindowClass, windowInitMethod);  
			g_lntVideoWindow = (jobject)(*jenv)->NewGlobalRef(jenv, lntVideoWindow);
			(*jenv)->DeleteLocalRef(jenv, lntVideoWindow);
			
			return TRUE;
		}
	}else{
		ms_warning("libjnigraphics.so cannot be loaded.");
	}
	return FALSE;
}
