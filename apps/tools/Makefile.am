
INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include 

bin_PROGRAMS = jobTracking mol2Info

jobTracking_SOURCES = jobTracking.cpp jobTrackingPO.cpp
jobTracking_LDADD = $(top_srcdir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lz

mol2Info_SOURCES = mol2Info.cpp mol2InfoPO.cpp
mol2Info_LDADD = $(top_srcdir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lz
