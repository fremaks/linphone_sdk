
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/rfc3984.h"
#include <jni.h>
#include "mediastreamer2/msjava.h"
#include <sys/types.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

//#define ANDROID_MEDIACODEC
jclass g_h264_codec_class;
static jobject g_h264_encode_obj;
static jmethodID g_h264_codec_encode_id;
static jmethodID g_h264_codec_close_id;

typedef struct _EncData {
	MSVideoSize vsize;
	int bitrate;
	float fps;
	int mode;
	uint64_t framenum;
	Rfc3984Context packer;
	int keyframe_int;
	bool_t generate_keyframe;
} EncData;

#include "fms_log.h"
static void enc_init(MSFilter *f) {
	MSVideoSize size = {640, 480};
	EncData *d = ms_new(EncData, 1);
	JNIEnv *jni_env;
	JavaVM *jvm = ms_get_jvm();
	jmethodID h264_codec_init_id;
	jobject h264_encode_obj;
	//d->enc = NULL;
	d->bitrate = 384000;//2000000;//384000;
	d->vsize = size;

	d->fps = 25;//30;
	d->keyframe_int = 10; /*10 seconds */
	d->mode = 0;
	d->framenum = 0;
	d->generate_keyframe = FALSE;
	f->data = d;

	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	//h264_codec_class = (*jni_env)->FindClass(jni_env, "com/example/linphone/H264Codec");
	//g_h264_codec_class = (jclass)(*jni_env)->NewGlobalRef(jni_env, h264_codec_class);
	h264_codec_init_id = (*jni_env)->GetMethodID(jni_env, g_h264_codec_class, "<init>", "(IIIII)V");
	h264_encode_obj = (*jni_env)->NewObject(jni_env, g_h264_codec_class, h264_codec_init_id, 0, 
										d->vsize.width, d->vsize.height, d->keyframe_int, d->bitrate);  
	g_h264_encode_obj = (*jni_env)->NewGlobalRef(jni_env, h264_encode_obj);
	(*jni_env)->DeleteLocalRef(jni_env, h264_encode_obj);
	g_h264_codec_encode_id = (*jni_env)->GetMethodID(jni_env, g_h264_codec_class, "encode", "([B[B)I");
	g_h264_codec_close_id = (*jni_env)->GetMethodID(jni_env, g_h264_codec_class, "close", "()V");
	(*jvm)->DetachCurrentThread(jvm);
}

static void enc_uninit(MSFilter *f) {
	EncData *d = (EncData*)f->data;
	ms_free(d);		
}

static void enc_preprocess(MSFilter *f) {
	EncData *d = (EncData*)f->data;
	rfc3984_init(&d->packer);
	rfc3984_set_mode(&d->packer, 1);
	rfc3984_enable_stap_a(&d->packer, FALSE);
	d->framenum=0;
}


void enc_process(MSFilter *f){

	EncData *d=(EncData*)f->data;
	uint32_t ts=f->ticker->time*90LL;
	mblk_t *im;
	//MSPicture pic;
	MSQueue nalus;
	
	//static uint64_t last_time = 0;
	static unsigned char out_buf[640*480*10];
	static char sps[9];
	static char pps[4];
	int h264_encode_size = 0;
	
	JNIEnv *jni_env;
	JavaVM *jvm= ms_get_jvm();	
	jbyteArray input_data;
	jbyteArray output_data;
	jint out_sizes;
	char *pos = NULL;
	mblk_t *sps_t = NULL;
	mblk_t *pps_t = NULL;
	mblk_t *om;	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);	

	ms_queue_init(&nalus);
	while((im=ms_queue_get(f->inputs[0]))!=NULL){

		h264_encode_size = im->b_wptr - im->b_rptr;	
		//last_time = f->ticker->time; 	

		input_data = (*jni_env)->NewByteArray(jni_env, h264_encode_size); 
		(*jni_env)->SetByteArrayRegion(jni_env, input_data, 0, h264_encode_size, (jbyte*)im->b_rptr);
		output_data = (*jni_env)->NewByteArray(jni_env, h264_encode_size); 
	
		out_sizes = (*jni_env)->CallIntMethod(jni_env, g_h264_encode_obj, g_h264_codec_encode_id, input_data, output_data);
		(*jni_env)->GetByteArrayRegion(jni_env, output_data, 0, out_sizes, (jbyte*)out_buf);
	
		(*jni_env)->DeleteLocalRef(jni_env, input_data);
		(*jni_env)->DeleteLocalRef(jni_env, output_data);
		
		//FMS_WARN("++++++++++++++++++++====================enc_process size=%d\n", out_sizes);
		if (out_sizes > 0) { //encode ok 4+9+4+4
			pos = (char *)out_buf;
			pos += 10;
			
			if(out_buf[4] == 0x67 && out_buf[17] == 0x68) {  //SPS PPS
				memcpy(sps, out_buf+4, 9);
				
				sps_t =allocb(9, 0); 
				memcpy(sps_t->b_rptr, sps, 9);
				sps_t->b_wptr+= 9;
				ms_queue_put(&nalus, sps_t);
				pos = (char *)sps_t->b_rptr;
				
				memcpy(pps, out_buf+17, 4);
				
				pps_t =allocb(4, 0); 
				memcpy(pps_t->b_rptr, pps, 4);
				pps_t->b_wptr+= 4;	
				ms_queue_put(&nalus, pps_t);
				pos = (char *)pps_t->b_rptr;
				
				om=allocb(out_sizes-4-21 ,0); // delete start_code
				memcpy(om->b_wptr, out_buf+4+21, out_sizes-4-21);
				om->b_wptr+= out_sizes-4-21;
				ms_queue_put(&nalus,om);
			} else {
				if (out_buf[4] == 0x65) {
					sps_t =allocb(9, 0); 
					memcpy(sps_t->b_rptr, sps, 9);
					sps_t->b_wptr+= 9;
					ms_queue_put(&nalus, sps_t);
				
					pps_t =allocb(4, 0); 
					memcpy(pps_t->b_rptr, pps, 4);
					pps_t->b_wptr+= 4;	
					ms_queue_put(&nalus, pps_t);					
				}
				
				om=allocb(out_sizes -4 ,0); // delete start_code
				memcpy(om->b_wptr, out_buf+4, out_sizes-4);
				om->b_wptr+= out_sizes-4;
				ms_queue_put(&nalus,om);
			}

		}
		rfc3984_pack(&d->packer,&nalus,f->outputs[0],ts);
		d->framenum++;
		freemsg(im);
		
	}
	
	(*jvm)->DetachCurrentThread(jvm);

}

static void enc_postprocess(MSFilter *f){
	EncData *d = (EncData*)f->data;
	rfc3984_uninit(&d->packer);
}


/*static MSFilterMethod enc_methods[]={

};*/

MSFilterDesc ms_h264_enc_desc={
	MS_H264_ENC_ID,
	"MSH264Enc",
	N_("A H264 encoder based on MediaCodec project (with multislicing enabled)"),
	MS_FILTER_ENCODER,
	"H264",
	1,
	1,
	enc_init,
	enc_preprocess,
	enc_process,
	enc_postprocess,
	enc_uninit,
	NULL //enc_methods
};

MS_FILTER_DESC_EXPORT(ms_h264_enc_desc)
