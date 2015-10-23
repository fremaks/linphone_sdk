bin_PROGRAMS = linphone_base_test
linphone_base_test_SOURCES = \
	linphone_base_test.c

linphone_base_test_CFLAGS = \
        -I$(top_srcdir)/base/include \
	$(FMS_UTILS_CFLAGS) 
#	-Wl,-rpath -Wl,/home/fremaks/lorent/linphone/lib \
#	-Wl,-rpath -Wl,/home/fremaks/lorent/belle-sip/lib \
#	-Wl,-rpath -Wl,/home/fremaks/lorent/libantlr3c/lib \
#	-Wl,-rpath -Wl,/home/fremaks/lorent/libxml2/lib \
#	-Wl,-rpath -Wl,/home/fremaks/lorent/alsa-lib/lib \
#	-Wl,-rpath -Wl,/home/fremaks/lorent/fremaks_utils/lib  
#       -I/home/fremaks/lorent/linphone/include/linphone \
#        -I/home/fremaks/lorent/linphone/include/mediastreamer2 \
#        -I/home/fremaks/lorent/linphone/include

#linphone_base_test_LDFLAGS= \
#	 -Wl,rpath -Wl,/home/fremaks/lorent/linphone/lib

linphone_base_test_LDADD = \
        $(top_srcdir)/base/src/liblinphone_base.la \
	$(ANTLR_LIBS) \
	$(FMS_UTILS_LIBS) \
	$(LINPHONE_LIBS) \
	$(BELLESIP_LIBS) \
	$(LIBXML2_LIBS)
