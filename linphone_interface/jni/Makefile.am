lib_LTLIBRARIES=liblinphone_jni.la
liblinphone_jni_la_SOURCES = \
	linphone_jni.c

liblinphone_jni_la_CFLAGS = \
	-I./ \
	-I$(top_srcdir)/base/include \
	$(FMS_UTILS_CFLAGS) \
	$(LINPHONE_CFLAGS)	

liblinphone_jni_la_LDFLAGS = -avoid-version

liblinphone_jni_la_LIBADD = \
	$(FMS_UTILS_LIBS) \
	$(LINPHONE_LIBS) \
	$(BELLESIP_LIBS) \
        $(ANTLR_LIBS) \
        $(LIBXML2_LIBS) \
	$(top_srcdir)/base/src/liblinphone_base.la -lstdc++
