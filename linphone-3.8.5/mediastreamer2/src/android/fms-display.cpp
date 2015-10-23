#include <jni.h>  
#include <android_runtime/AndroidRuntime.h>  
#include <android_runtime/android_view_Surface.h>  
#include <gui/Surface.h>  
#include <assert.h>  
#include <utils/Log.h>  
#include <JNIHelp.h>  
#include <media/stagefright/foundation/ADebug.h>  
#include <ui/GraphicBufferMapper.h>  
#include <cutils/properties.h> 

using namespace android;  

static sp<Surface> gSurface; 


static void 
render(const void *data, size_t size, const sp<ANativeWindow> &nativeWindow,
		int width,int height) {  
  
    sp<ANativeWindow> mNativeWindow = nativeWindow;  
    int err;  
    int mCropWidth = width;  
    int mCropHeight = height;  
      
    int halFormat = HAL_PIXEL_FORMAT_YV12;//颜色空间  
    int bufWidth = (mCropWidth + 1) & ~1;//按2对齐  
    int bufHeight = (mCropHeight + 1) & ~1;  
      
    CHECK_EQ(0,  
            native_window_set_usage(  
            mNativeWindow.get(),  
            GRALLOC_USAGE_SW_READ_NEVER | GRALLOC_USAGE_SW_WRITE_OFTEN  
            | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_EXTERNAL_DISP));  
  
    CHECK_EQ(0,  
            native_window_set_scaling_mode(  
            mNativeWindow.get(),  
            NATIVE_WINDOW_SCALING_MODE_SCALE_CROP));  
  
    // Width must be multiple of 32???  
    //很重要,配置宽高和和指定颜色空间yuv420  
    //如果这里不配置好，下面deque_buffer只能去申请一个默认宽高的图形缓冲区  
    CHECK_EQ(0, native_window_set_buffers_geometry(  
                mNativeWindow.get(),  
                bufWidth,  
                bufHeight,  
                halFormat));  
      
      
    ANativeWindowBuffer *buf;//描述buffer  
    //申请一块空闲的图形缓冲区  
    if ((err = native_window_dequeue_buffer_and_wait(mNativeWindow.get(),  
            &buf)) != 0) {  
        ALOGW("Surface::dequeueBuffer returned error %d", err);  
        return;  
    }  
  
    GraphicBufferMapper &mapper = GraphicBufferMapper::get();  
  
    Rect bounds(mCropWidth, mCropHeight);  
  
    void *dst;  
    CHECK_EQ(0, mapper.lock(//用来锁定一个图形缓冲区并将缓冲区映射到用户进程  
                buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &dst));//dst就指向图形缓冲区首地址  
  
    if (true){  
        size_t dst_y_size = buf->stride * buf->height;  
        size_t dst_c_stride = ALIGN(buf->stride / 2, 16);//1行v/u的大小  
        size_t dst_c_size = dst_c_stride * buf->height / 2;//u/v的大小  
          
        memcpy(dst, data, dst_y_size + dst_c_size*2);//将yuv数据copy到图形缓冲区  
    }  
  
    CHECK_EQ(0, mapper.unlock(buf->handle));  
  
    if ((err = mNativeWindow->queueBuffer(mNativeWindow.get(), buf,  
            -1)) != 0) {  
        ALOGW("Surface::queueBuffer returned error %d", err);  
    }  
    buf = NULL;  
}  


static void fms_display_init(MSFilter *f){
	
}

static void fms_display_uninit(MSFilter *f){

}

static void fms_display_process(MSFilter *f){

}

static MSFilterMethod methods[]={
	//{	MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID , android_display_set_window },
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
	.process=fms_display_process,
	.uninit=fms_display_uninit,
	.methods=methods
};





static void  
Java_com_example_linphone_FmsRender_setNativeWindowID(JNIEnv *env,
	jobject thiz, jobject jsurface){  
	  
    gSurface = android_view_Surface_getSurface(env, jsurface);  
    if(android::Surface::isValid(gSurface)){  
        FMS_WARN("surface is valid\n");  
    }else {  
        FMS_WARN("surface is invalid\n");  
        return false;  
    }  
	
    FMS_WARN("setNativeWindowID END\n"); 
	
    return true;  
}  

static void  
Java_com_example_linphone_FmsRender_display(JNIEnv *env, 
	jobject thiz, jbyteArray yuvData, jint width, jint height){  
   
    jint len = env->GetArrayLength(yuvData);  
    FMS_WARN("FmsRender_display len = %d\n",len);  
	
    jbyte *byteBuf = env->GetByteArrayElements(yuvData, 0);  
    render(byteBuf,len, gSurface, width, height);  
	env->ReleaseByteArrayElements(yuvData, byteBuf, 0);
}  