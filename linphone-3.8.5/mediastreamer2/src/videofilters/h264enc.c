
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

#define VIDEO_ENC_MAX_WIDTH      1080
#define VIDEO_ENC_MAX_HEIGHT     720
#define CODEC_ENCODE             0
#define START_CODE_LEN           4
#define SPS_LEN                  9
#define PPS_LEN                  4

#define GET_TYPE(m) ((m) & 0X1F)

enum {
	IDR_TYPE = 5,
	SPS_TYPE = 7,
	PPS_TYPE = 8
};

typedef struct _EncData {
	MSVideoSize vsize;
	int bitrate;
	float fps;
	uint64_t framenum;
	Rfc3984Context packer;
	int keyframe_int;
	bool_t generate_keyframe;
	jbyteArray input_data;
	jbyteArray output_data;
	jobject h264_encode_obj;
	jmethodID h264_construct_id;
	jmethodID h264_encode_id;
	jmethodID h264_close_id;
	uint8_t *bitstream;
	int bitstream_size;
} EncData;

jclass h264_codec_class;

static void enc_init(MSFilter *f) {
	EncData *d = ms_new(EncData, 1);
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();

	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	
	d->h264_construct_id = (*jni_env)->GetMethodID(jni_env, h264_codec_class, "<init>", "(IIIII)V");
	d->h264_encode_id = (*jni_env)->GetMethodID(jni_env, h264_codec_class, "encode", "([B[B)I");
	d->h264_close_id = (*jni_env)->GetMethodID(jni_env, h264_codec_class, "close", "()V");
	d->input_data = (*jni_env)->NewByteArray(jni_env, VIDEO_ENC_MAX_WIDTH*VIDEO_ENC_MAX_HEIGHT*3/2); 
	d->input_data = (*jni_env)->NewGlobalRef(jni_env, d->input_data);
	d->output_data = (*jni_env)->NewByteArray(jni_env, VIDEO_ENC_MAX_WIDTH*VIDEO_ENC_MAX_HEIGHT*3/2); 
	d->output_data = (*jni_env)->NewGlobalRef(jni_env, d->output_data);
	
	(*jvm)->DetachCurrentThread(jvm);

	d->bitrate = 384000;//2000000;//384000;
	d->fps = 25;//30;
	d->keyframe_int = 10; /*10 seconds */
	d->framenum = 0;
	f->data = d;
	d->bitstream_size = VIDEO_ENC_MAX_WIDTH*VIDEO_ENC_MAX_HEIGHT*3/2;
	d->bitstream = (uint8_t*)ms_malloc0(d->bitstream_size);
}

static void enc_uninit(MSFilter *f) {
	EncData *d = (EncData*)f->data;
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	
	if (d->input_data != 0) {
		(*jni_env)->DeleteGlobalRef(jni_env, d->input_data);	
	}
	if (d->output_data != 0) {
		(*jni_env)->DeleteGlobalRef(jni_env, d->output_data);
	}
	
	(*jvm)->DetachCurrentThread(jvm);
	
	ms_free(d->bitstream);
	ms_free(d);		
}

static void enc_preprocess(MSFilter *f) {
	EncData *d = (EncData*)f->data;
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	
	d->h264_encode_obj = (*jni_env)->NewObject(jni_env, h264_codec_class, d->h264_construct_id, 
								CODEC_ENCODE, d->vsize.width, d->vsize.height, d->keyframe_int, d->bitrate);  
	d->h264_encode_obj = (*jni_env)->NewGlobalRef(jni_env, d->h264_encode_obj);
	
	(*jvm)->DetachCurrentThread(jvm);
	
	rfc3984_init(&d->packer);
	rfc3984_set_mode(&d->packer, 1);
	rfc3984_enable_stap_a(&d->packer, FALSE);
	d->framenum = 0;
}

static void enc_postprocess(MSFilter *f){
	EncData *d = (EncData*)f->data;
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	
	(*jni_env)->CallIntMethod(jni_env, d->h264_encode_obj, d->h264_close_id);
	(*jni_env)->DeleteGlobalRef(jni_env, d->h264_encode_obj);
	d->h264_encode_obj = 0;

	(*jvm)->DetachCurrentThread(jvm);

	rfc3984_uninit(&d->packer);
}


void enc_process(MSFilter *f){
	EncData *d = (EncData*)f->data;
	uint32_t ts = f->ticker->time*90LL;
	mblk_t *im =NULL;
	MSQueue nalus;
	int in_size = 0;
	jint out_size = 0;
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();	
	uint8_t *pos = NULL;	
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);	

	ms_queue_init(&nalus);
	while((im = ms_queue_get(f->inputs[0]))!=NULL){
        in_size = im->b_wptr - im->b_rptr;	
		(*jni_env)->SetByteArrayRegion(jni_env, d->input_data, 0, in_size, (jbyte*)im->b_rptr);
		out_size = (*jni_env)->CallIntMethod(jni_env, d->h264_encode_obj, d->h264_encode_id, 
											 d->input_data, d->output_data);
		(*jni_env)->GetByteArrayRegion(jni_env, d->output_data, 0, out_size, (jbyte*)d->bitstream);

		pos = (uint8_t *)d->bitstream;
		while (out_size > START_CODE_LEN) {
			if (GET_TYPE(pos[4]) == SPS_TYPE && out_size > SPS_LEN + START_CODE_LEN) {
				mblk_t *sps_t = allocb(SPS_LEN, 0);
				
				memcpy(sps_t->b_rptr, pos + START_CODE_LEN, SPS_LEN);
				sps_t->b_wptr += SPS_LEN;
				ms_queue_put(&nalus, sps_t);
				pos += SPS_LEN + START_CODE_LEN;
				out_size -= SPS_LEN + START_CODE_LEN;
			} else if (GET_TYPE(pos[4]) == PPS_TYPE && out_size > PPS_LEN + START_CODE_LEN) {
				mblk_t *pps_t = allocb(PPS_LEN, 0); 
				
				memcpy(pps_t->b_rptr, pos + START_CODE_LEN, PPS_LEN);
				pps_t->b_wptr += PPS_LEN;
				ms_queue_put(&nalus, pps_t);
				pos += PPS_LEN + START_CODE_LEN;
				out_size -= PPS_LEN + START_CODE_LEN;
			} else {
				mblk_t *om = allocb(out_size - START_CODE_LEN, 0); // delete start_code
				
				memcpy(om->b_wptr, pos + START_CODE_LEN, out_size - START_CODE_LEN);
				om->b_wptr += out_size - START_CODE_LEN;
				ms_queue_put(&nalus,om);
				out_size = 0;
			}
					
		}

		rfc3984_pack(&d->packer, &nalus, f->outputs[0], ts);
		d->framenum++;
		freemsg(im);	
	}
	
	(*jvm)->DetachCurrentThread(jvm);

}


static int enc_support_pixfmt(MSFilter *f, void *arg) { 
	MSVideoEncoderPixFmt *encoder_supports_source_format = (MSVideoEncoderPixFmt *)arg;
	
	if (encoder_supports_source_format->pixfmt  != MS_YUV420P) {
		return -1;
	}
	encoder_supports_source_format->supported = TRUE;
	
	return 0;
}

static int enc_set_video_size(MSFilter *f, void *arg) {
	EncData *d = (EncData*)f->data;
	MSVideoSize *vs = (MSVideoSize *)arg;
	
	d->vsize = *vs;
	
	return 0;
}

static int enc_get_video_size(MSFilter *f, void *arg) {
	EncData *d = (EncData*)f->data;
	MSVideoSize *vs = (MSVideoSize *)arg;
	
	*vs = d->vsize;
	
	return 0;
}

static int enc_set_fps(MSFilter *f, void *arg) {
	EncData *d = (EncData*)f->data;
	float *fps = (float *)arg;
	
	d->fps= *fps;
	
	return 0;
}

static int enc_get_fps(MSFilter *f, void *arg) {
	EncData *d = (EncData*)f->data;
	float *fps = (float *)arg;
	
	*fps = d->fps;
	
	return 0;
}

static MSFilterMethod enc_methods[]={
	{	MS_VIDEO_ENCODER_SUPPORTS_PIXFMT, enc_support_pixfmt},
	{   MS_FILTER_SET_VIDEO_SIZE, enc_set_video_size},
	{	MS_FILTER_GET_VIDEO_SIZE, enc_get_video_size},
	{ 	MS_FILTER_SET_FPS, enc_set_fps},
	{ 	MS_FILTER_GET_FPS, enc_get_fps},
};

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
	enc_methods
};

MS_FILTER_DESC_EXPORT(ms_h264_enc_desc)
