lib_LTLIBRARIES=liblinphone_base.la
liblinphone_base_la_SOURCES = \
	linphone_event.c \
	linphone_base.c

liblinphone_base_la_CFLAGS = \
	-I$(top_srcdir)/base/include \
	$(LINPHONE_CFLAGS) \
	$(FMS_UTILS_CFLAGS) \
	-I$(top_srcdir)/../mswebrtc/webrtc/common_audio/signal_processing/include 
	
liblinphone_base_la_LDFLAGS = -avoid-version

liblinphone_base_la_LIBADD = \
	$(LINPHONE_LIBS) \
	$(FMS_UTILS_LIBS) \
	$(BELLESIP_LIBS) \
	$(LIBXML2_LIBS)\
	$(top_srcdir)/../mswebrtc/libmswebrtc.la
