/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2010  Belledonne Communications SARL
Author: Simon Morlat <simon.morlat@linphone.org>

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
#include "mediastreamer2/rfc3984.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msticker.h"
#include "stream_regulator.h"

#include "ffmpeg-priv.h"

#include "ortp/b64.h"
#include "fms_log.h"
#include <jni.h>

#define VIDEO_DEC_MAX_WIDTH      1080
#define VIDEO_DEC_MAX_HEIGHT     720
#define CODEC_DECODE             1    
#define START_CODE_LEN           4
  
#define GET_TYPE(m) ((m) & 0X1F)

enum {
	IDR_TYPE = 5,
	SPS_TYPE = 7,
	PPS_TYPE = 8
};

typedef struct _DecData{
	mblk_t *yuv_msg;
	mblk_t *sps,*pps;
	Rfc3984Context unpacker;
	unsigned int packet_num;
	uint8_t *bitstream;
	int bitstream_size;
	jbyteArray input_data;
	jbyteArray output_data;
	bool_t IsFirstImageDec;
	bool_t IsRecivedFirstIframe;
	bool_t IsRecivedSps;
	bool_t IsRecivedPps;
	jobject h264_decode_obj;
	jmethodID h264_construct_id;
	jmethodID h264_decode_id;
	jmethodID h264_close_id;
} DecData;

extern jclass h264_codec_class;


static void dec_init(MSFilter *f) {
	DecData *d = (DecData*)ms_new(DecData,1);
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();

	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	
	d->h264_construct_id = (*jni_env)->GetMethodID(jni_env, h264_codec_class, "<init>", "(IIIII)V");
	d->h264_decode_id = (*jni_env)->GetMethodID(jni_env, h264_codec_class, "decode", "([BI[B)I");
	d->h264_close_id = (*jni_env)->GetMethodID(jni_env, h264_codec_class, "close", "()V");

	d->input_data = (*jni_env)->NewByteArray(jni_env, VIDEO_DEC_MAX_WIDTH*VIDEO_DEC_MAX_HEIGHT*3/2); 
	d->input_data = (*jni_env)->NewGlobalRef(jni_env, d->input_data);
	d->output_data = (*jni_env)->NewByteArray(jni_env, VIDEO_DEC_MAX_WIDTH*VIDEO_DEC_MAX_HEIGHT*3/2);
	d->output_data = (*jni_env)->NewGlobalRef(jni_env, d->output_data);
	
	(*jvm)->DetachCurrentThread(jvm);	

	d->yuv_msg = NULL;
	d->sps = NULL;
	d->pps = NULL;
	rfc3984_init(&d->unpacker);
	d->packet_num = 0;
	d->bitstream_size = VIDEO_DEC_MAX_WIDTH*VIDEO_DEC_MAX_HEIGHT*3/2;
	d->bitstream = (uint8_t*)ms_malloc0(d->bitstream_size);
	d->IsFirstImageDec = FALSE;
	d->IsRecivedFirstIframe = FALSE;
	d->IsRecivedSps = FALSE;
	d->IsRecivedPps = FALSE;
	f->data = d;
}


static void dec_uninit(MSFilter *f){
	DecData *d = (DecData*)f->data;
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	
	(*jni_env)->DeleteGlobalRef(jni_env, d->input_data);
	(*jni_env)->DeleteGlobalRef(jni_env, d->output_data);

	(*jvm)->DetachCurrentThread(jvm);	
	
	rfc3984_uninit(&d->unpacker);
	if (d->yuv_msg) freemsg(d->yuv_msg);
	if (d->sps) freemsg(d->sps);
	if (d->pps) freemsg(d->pps);
	ms_free(d->bitstream);
	ms_free(d);	
}

static void dec_preprocess(MSFilter* f) { 
	DecData *d = (DecData*)f->data;
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	
	d->h264_decode_obj = (*jni_env)->NewObject(jni_env, h264_codec_class, d->h264_construct_id, 
							     CODEC_DECODE, VIDEO_DEC_MAX_WIDTH, VIDEO_DEC_MAX_HEIGHT, 0, 0);
	d->h264_decode_obj = (*jni_env)->NewGlobalRef(jni_env, d->h264_decode_obj);

	(*jvm)->DetachCurrentThread(jvm);
}

static void dec_postprocess(MSFilter* f) { 
	DecData *d = (DecData*)f->data;
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	
	(*jni_env)->CallIntMethod(jni_env, d->h264_decode_obj, d->h264_close_id);
	(*jni_env)->DeleteGlobalRef(jni_env, d->h264_decode_obj);
	d->h264_decode_obj = 0;
	
	(*jvm)->DetachCurrentThread(jvm);
}

static mblk_t *get_as_yuvmsg(MSFilter *f, DecData *s, unsigned char* inBuf, 
									unsigned int inSize) {
	const int padding = 16;
	mblk_t *msg = allocb(inSize + padding, 0);
	
	if(NULL == msg) {
		return NULL;
	}
	memcpy(msg->b_wptr, inBuf, inSize);
	msg->b_wptr += inSize;
	
	return msg;
}

static void dec_process(MSFilter *f) {
	DecData *d = (DecData*)f->data;
	mblk_t *im = NULL;
	MSQueue nalus;
	mblk_t *decodeM = NULL;	
	int in_size = 0;
	jint out_size = 0;
	mblk_t *oneNalu = NULL;
	JNIEnv *jni_env = NULL;
	JavaVM *jvm = ms_get_jvm();
	static char start_code[4] = {0, 0, 0, 1};
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);	
	
	ms_queue_init(&nalus);	
	while((im = ms_queue_get(f->inputs[0])) != NULL) {
		/*push the sps/pps given in sprop-parameter-sets if any*/
		if (d->packet_num == 0 && d->sps && d->pps) {
			mblk_set_timestamp_info(d->sps,mblk_get_timestamp_info(im));
			mblk_set_timestamp_info(d->pps,mblk_get_timestamp_info(im));
			rfc3984_unpack(&d->unpacker,d->sps,&nalus);
			rfc3984_unpack(&d->unpacker,d->pps,&nalus);
			d->sps = NULL;
			d->pps = NULL;
		}
		rfc3984_unpack(&d->unpacker,im,&nalus);
		
		while((oneNalu = ms_queue_get(&nalus)) != NULL) {
			in_size = oneNalu->b_wptr - oneNalu->b_rptr;
			
			if (GET_TYPE(oneNalu->b_rptr[0]) == SPS_TYPE) {
				if (d->sps == NULL) {
					FMS_WARN("dec_process get sps\n");
					d->sps = allocb(in_size + START_CODE_LEN, 0);
					if (d->sps) {
						memcpy(d->sps->b_rptr, start_code, START_CODE_LEN);
						memcpy(d->sps->b_rptr + START_CODE_LEN, oneNalu->b_rptr, in_size);
						d->sps->b_wptr += in_size + START_CODE_LEN;
					}
				} else {
					freemsg(oneNalu);
					continue;						
				}
			} else if (GET_TYPE(oneNalu->b_rptr[0]) == PPS_TYPE) {
				if (d->pps == NULL && d->sps != NULL) {
					FMS_WARN("dec_process get pps\n");
					d->pps = allocb(in_size + START_CODE_LEN, 0);
					if (d->pps) {
						memcpy(d->pps->b_rptr, start_code, START_CODE_LEN);
						memcpy(d->pps->b_rptr + START_CODE_LEN, oneNalu->b_rptr, in_size);
						d->pps->b_wptr += in_size + START_CODE_LEN;
					}
				} else {
					freemsg(oneNalu);
					continue;	
				}
			}
			
			if (d->sps == NULL || (GET_TYPE(oneNalu->b_rptr[0]) != SPS_TYPE && d->pps == NULL)) {
				FMS_WARN("skip frame without no sps and pps\n");
				freemsg(oneNalu);
				continue;			
			}
			if (!d->IsRecivedFirstIframe && GET_TYPE(oneNalu->b_rptr[0]) != SPS_TYPE 
				&& GET_TYPE(oneNalu->b_rptr[0]) != PPS_TYPE) {
				if (GET_TYPE(oneNalu->b_rptr[0]) == IDR_TYPE) {
					d->IsRecivedFirstIframe = TRUE;
				} else {
					FMS_WARN("skip frame without the first IDR\n");
					freemsg(oneNalu);
					continue;
				}
			}

			(*jni_env)->SetByteArrayRegion(jni_env, d->input_data, 0, START_CODE_LEN, (jbyte*)start_code);
			(*jni_env)->SetByteArrayRegion(jni_env, d->input_data, START_CODE_LEN, in_size, (jbyte*)oneNalu->b_rptr);
	 
			out_size = (*jni_env)->CallIntMethod(jni_env, d->h264_decode_obj, d->h264_decode_id,
				                                     d->input_data, in_size + START_CODE_LEN, d->output_data);
			if (out_size <= 0) {
				freemsg(oneNalu);
				continue;
			}
				
			(*jni_env)->GetByteArrayRegion(jni_env, d->output_data, 0, out_size, 
				                              (jbyte*)d->bitstream);
		
			if (FALSE == d->IsFirstImageDec) {
				d->IsFirstImageDec = TRUE;
				//ms_filter_notify_no_arg(f, MS_VIDEO_DECODER_GET_FIRST_VIDEO_FRAME);
			}
			
			decodeM = get_as_yuvmsg(f, d, d->bitstream, out_size);
			if (decodeM) {
				ms_queue_put(f->outputs[0], decodeM);
			}
	
			freemsg(oneNalu);
		}
		d->packet_num++;
	}
	(*jvm)->DetachCurrentThread(jvm);
	
}
static int support_rendering(MSFilter *f, void *arg) {
	MSVideoDisplayDecodingSupport *decoding_support = (MSVideoDisplayDecodingSupport *)arg;
	
	decoding_support->supported = FALSE;

	return 0;
}

static MSFilterMethod  dec_methods[] = {
	{	MS_VIDEO_DECODER_SUPPORT_RENDERING	,	support_rendering	},
	{	0			                        ,	NULL	            }
};

MSFilterDesc ms_h264_dec_desc={
	MS_H264_DEC_ID,
	"MSH264Dec",
	N_("A H264 decoder based on MediaCodec project."),
	MS_FILTER_DECODER,
	"H264",
	1,
	1,
	dec_init,
	dec_preprocess,
	dec_process,
	dec_postprocess,
	dec_uninit,
	dec_methods
};

MS_FILTER_DESC_EXPORT(ms_h264_dec_desc)

