#!/bin/bash
SOURCE_PATH=$(pwd)

cd $SOURCE_PATH/x264
./configure --host=$1 --disable-asm --enable-shared --cross-prefix=$1-
make

cd $SOURCE_PATH/ffmpeg
./configure --cross-prefix=$1- --enable-cross-compile --target-os=linux --arch=arm --enable-shared --disable-static
make

cd $SOURCE_PATH
./configure --host=$1 --with-python=./python --enable-gtk_ui=no --disable-speex --disable-xv --disable-glx --disable-zlib BELLESIP_CFLAGS="-I$(pwd)/belle-sip-1.4.1/include" BELLESIP_LIBS="$(pwd)/belle-sip-1.4.1/src/libbellesip.la" LIBXML2_CFLAGS="-I$(pwd)/libxml2-2.9.1/include" LIBXML2_LIBS="$(pwd)/libxml2-2.9.1/libxml2.la" ANTLR_CFLAGS="-I$(pwd)/libantlr3c-3.4/include -I$(pwd)/libantlr3c-3.4" ANTLR_LIBS="$(pwd)/libantlr3c-3.4/libantlr3c.la" CFLAGS="-Wno-unused-function -Wno-unused-variable -DIN_LINPHONE" --disable-libv4l2 --enable-video=yes --enable-alsa=no --enable-android=yes --disable-python FMS_UTILS_CFLAGS="-I$(pwd)/fremaks_utils/include -I$(pwd)/fremaks_utils/fms_common/include" FMS_UTILS_LIBS="$(pwd)/fremaks_utils/fms_common/src/libfms_common.la" LINPHONE_CFLAGS="-I$(pwd)/linphone-3.8.5/coreapi -I$(pwd)/linphone-3.8.5/oRTP/include -I$(pwd)/linphone-3.8.5/mediastreamer2/include -I$(pwd)/linphone-3.8.5/mediastreamer2/include/mediastreamer2" LINPHONE_LIBS="$(pwd)/linphone-3.8.5/coreapi/liblinphone.la $(pwd)/linphone-3.8.5/oRTP/src/libortp.la $(pwd)/linphone-3.8.5/mediastreamer2/src/libmediastreamer_base.la $(pwd)/linphone-3.8.5/mediastreamer2/src/libmediastreamer_voip.la" CPPFLAGS="-Wno-strict-aliasing"   LDFLAGS="-lstdc++" --disable-tests --disable-pulseaudio  --enable-msg-storage=no  FFMPEG_CFLAGS="-I$(pwd)/ffmpeg -I$(pwd)/x264" FFMPEG_LIBS="-L$(pwd)/ffmpeg/libavcodec -lavcodec -L$(pwd)/ffmpeg/libavutil -lavutil -L$(pwd)/x264 -lx264" SWSCALE_CFLAGS="-I$(pwd)/ffmpeg -Wno-unused-variable" SWSCALE_LIBS="-L$(pwd)/ffmpeg/libswscale -lswscale"
make
