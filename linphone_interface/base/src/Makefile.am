lib_LTLIBRARIES=liblinphone_base.la
liblinphone_base_la_SOURCES = \
	linphone_event.c \
	linphone_base.c

liblinphone_base_la_CFLAGS = \
	-I$(top_srcdir)/base/include \
	$(LINPHONE_CFLAGS) \
	$(FMS_UTILS_CFLAGS) \
	-I$(top_srcdir)/../mswebrtc/webrtc/common_audio/signal_processing/include \
	-I$(top_srcdir)/../linphone-3.8.5/include \
	$(BELLESIP_CFLAGS) \
	$(LIBXML2_CFLAGS)
	
liblinphone_base_la_LDFLAGS = -avoid-version

liblinphone_base_la_LIBADD = \
	$(LINPHONE_LIBS) \
	$(FMS_UTILS_LIBS) \
	$(BELLESIP_LIBS) \
	$(LIBXML2_LIBS)\
	$(top_srcdir)/../mswebrtc/libmswebrtc.la
