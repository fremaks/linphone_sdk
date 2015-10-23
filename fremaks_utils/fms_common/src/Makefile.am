lib_LTLIBRARIES=libfms_common.la
libfms_common_la_SOURCES = fms_log.cpp fms_mem.cpp fms_spin_lock.cpp FmsSpinLock.cpp fms_file.c fms_dir.c
libfms_common_la_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/fms_common/include 
libfms_common_la_LDFLAGS = -avoid-version
if BUILD_ANDROID
libfms_common_la_LIBADD = -llog
endif

