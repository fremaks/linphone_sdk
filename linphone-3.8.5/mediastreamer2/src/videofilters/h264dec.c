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

typedef struct _DecData{
	mblk_t *yuv_msg;
	mblk_t *sps,*pps;
	Rfc3984Context unpacker;
	MSPicture outbuf;
	unsigned int packet_num;
	uint8_t *bitstream;
	int bitstream_size;
	bool_t IsFirstImageDec;
	bool_t IsRecivedFirstIframe;
	bool_t IsRecivedSps;
	bool_t IsRecivedPps;
	int reduceIframe;//0正常，大于0跳出
}DecData;



unsigned char decode_output[800*480*3/2];

extern jclass g_h264_codec_class;
static jobject g_h264_decode_obj;
static jmethodID g_h264_codec_decode_id;
static jmethodID g_h264_codec_close_id;


static void dec_init(MSFilter *f){
	DecData *d=(DecData*)ms_new(DecData,1);
	JNIEnv *jni_env;
	JavaVM *jvm = ms_get_jvm();
	jmethodID h264_codec_init_id;
	jobject h264_decode_obj;
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);

	d->yuv_msg=NULL;
	d->sps=NULL;
	d->pps=NULL;
	rfc3984_init(&d->unpacker);
	d->packet_num=0;
	d->outbuf.w=0;
	d->outbuf.h=0;
	d->bitstream_size=65536;
	d->bitstream=(uint8_t*)ms_malloc0(d->bitstream_size);
	d->IsFirstImageDec = FALSE;
	d->IsRecivedFirstIframe=FALSE;
	d->IsRecivedSps=FALSE;
	d->IsRecivedPps=FALSE;
	d->reduceIframe = 0;
	f->data=d;


	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);
	h264_codec_init_id = (*jni_env)->GetMethodID(jni_env, g_h264_codec_class, "<init>", "(IIIII)V");
	h264_decode_obj = (*jni_env)->NewObject(jni_env, g_h264_codec_class, h264_codec_init_id, 1, 
								             800, 480, 0, 0);
	
	g_h264_decode_obj = (*jni_env)->NewGlobalRef(jni_env, h264_decode_obj);
	(*jni_env)->DeleteLocalRef(jni_env, h264_decode_obj);

	g_h264_codec_decode_id = (*jni_env)->GetMethodID(jni_env, g_h264_codec_class, "decode", "([B[B)I");
	g_h264_codec_close_id = (*jni_env)->GetMethodID(jni_env, g_h264_codec_class, "close", "()V");

	(*jvm)->DetachCurrentThread(jvm);	

}


static void dec_uninit(MSFilter *f){
	DecData *d=(DecData*)f->data;
	rfc3984_uninit(&d->unpacker);
	if (d->yuv_msg) freemsg(d->yuv_msg);
	if (d->sps) freemsg(d->sps);
	if (d->pps) freemsg(d->pps);
	ms_free(d->bitstream);
	ms_free(d);
}

static mblk_t *get_as_yuvmsg(MSFilter *f, DecData *s, unsigned char* inBuf, unsigned int inSize){
	const int padding=16;
	mblk_t *msg=allocb(inSize+padding,0);
	if(msg == NULL)
		return NULL;
	memcpy(msg->b_wptr, inBuf, inSize);
	msg->b_wptr += inSize;
	return msg;
}



static void dec_process(MSFilter *f){
	DecData *d=(DecData*)f->data;
	mblk_t *im;
	MSQueue nalus;
	mblk_t *oneNalu;
	JNIEnv *jni_env;
	JavaVM *jvm= ms_get_jvm();
	char start_code[4] = {0, 0, 0, 1};
	
	(*jvm)->AttachCurrentThread(jvm, &jni_env, NULL);	
	ms_queue_init(&nalus);	

	while((im=ms_queue_get(f->inputs[0]))!=NULL){
		/*push the sps/pps given in sprop-parameter-sets if any*/
		if (d->packet_num==0 && d->sps && d->pps){
			mblk_set_timestamp_info(d->sps,mblk_get_timestamp_info(im));
			mblk_set_timestamp_info(d->pps,mblk_get_timestamp_info(im));
			rfc3984_unpack(&d->unpacker,d->sps,&nalus);
			rfc3984_unpack(&d->unpacker,d->pps,&nalus);
			d->sps=NULL;
			d->pps=NULL;
		}
		rfc3984_unpack(&d->unpacker,im,&nalus);
		
		while((oneNalu=ms_queue_get(&nalus))!=NULL){
			int in_size = oneNalu->b_wptr-oneNalu->b_rptr;
			mblk_t *decodeM;	
			jbyteArray input_data;
			jbyteArray output_data;
			jint out_sizes;
			
			if (oneNalu->b_rptr[0] == 0x67) {
				if (d->sps == NULL) {
					FMS_WARN("dec_process get sps\n");
					d->sps = allocb(in_size + 4, 0);
					if (d->sps) {
						memcpy(d->sps->b_rptr, start_code, 4);
						memcpy(d->sps->b_rptr + 4, oneNalu->b_rptr, in_size);
						d->sps->b_wptr += in_size + 4;
					}
				} else {
					freemsg(oneNalu);
					continue;						
				}
			} else if (oneNalu->b_rptr[0] == 0x68) {
				if (d->pps == NULL && d->sps != NULL) {
					FMS_WARN("dec_process get pps\n");
					d->pps = allocb(in_size + 4, 0);
					if (d->pps) {
						memcpy(d->pps->b_rptr, start_code, 4);
						memcpy(d->pps->b_rptr + 4, oneNalu->b_rptr, in_size);
						d->pps->b_wptr += in_size + 4;
					}
				} else {
					freemsg(oneNalu);
					continue;	
				}
			}
			
			if (d->sps == NULL || (oneNalu->b_rptr[0] != 0x67 && d->pps == NULL)) {
				FMS_WARN("skip frame without no sps and pps\n");
				freemsg(oneNalu);
				continue;			
			}
			if (!d->IsRecivedFirstIframe && oneNalu->b_rptr[0] != 0x67 && oneNalu->b_rptr[0] != 0x68) {
				if (oneNalu->b_rptr[0] == 0x65) {
					d->IsRecivedFirstIframe = TRUE;
				} else {
					FMS_WARN("skip frame without the first IDR\n");
					continue;
				}
			}

			input_data = (*jni_env)->NewByteArray(jni_env, in_size+4); 
			(*jni_env)->SetByteArrayRegion(jni_env, input_data, 0, 4, (jbyte*)start_code);
			(*jni_env)->SetByteArrayRegion(jni_env, input_data, 4, in_size, (jbyte*)oneNalu->b_rptr);
	
			
			output_data = (*jni_env)->NewByteArray(jni_env, 800*480*3/2); 
			out_sizes = (*jni_env)->CallIntMethod(jni_env, g_h264_decode_obj, g_h264_codec_decode_id, input_data, output_data);
			if (out_sizes != 800*480*3/2) {
				(*jni_env)->DeleteLocalRef(jni_env, input_data);
				(*jni_env)->DeleteLocalRef(jni_env, output_data);
				break;
			}
				
			(*jni_env)->GetByteArrayRegion(jni_env, output_data, 0, out_sizes, (jbyte*)decode_output);
			(*jni_env)->DeleteLocalRef(jni_env, input_data);
			(*jni_env)->DeleteLocalRef(jni_env, output_data);

				
				
			if(d->IsFirstImageDec == FALSE) {
				d->IsFirstImageDec = TRUE;
				//ms_filter_notify_no_arg(f, MS_VIDEO_DECODER_GET_FIRST_VIDEO_FRAME);
			}
			decodeM = get_as_yuvmsg(f, d, decode_output, out_sizes);
			if(decodeM)
			{
				FMS_WARN("@@@@@@@@@@@@@@@@@dec_process SUCCESS\n");
				ms_queue_put(f->outputs[0],decodeM);
			}
			
			freemsg(oneNalu);

		}
		d->packet_num++;
	}
	(*jvm)->DetachCurrentThread(jvm);
	
}

/*static MSFilterMethod  h264_dec_methods[]={
	{	MS_FILTER_ADD_FMTP	,	dec_add_fmtp	},
	{	0			,	NULL	}
};*/

MSFilterDesc ms_h264_dec_desc={
	MS_H264_DEC_ID,
	"MSH264Dec",
	N_("A H264 decoder based on MediaCodec project."),
	MS_FILTER_DECODER,
	"H264",
	1,
	1,
	dec_init,
	NULL,
	dec_process,
	NULL,
	dec_uninit,
	NULL //h264_dec_methods
};

MS_FILTER_DESC_EXPORT(ms_h264_dec_desc)

