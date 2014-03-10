if USE_MPI
    INCLUDES = -I. -I$(top_srcdir)/src -I$(top_srcdir)/src/VinaLC -I/@boost_dir@/include -I/@mpi_path@/include
else
    INCLUDES = -I. -I$(top_srcdir)/src -I$(top_srcdir)/src/VinaLC -I/@boost_dir@/include
endif

if USE_MPI
    bin_PROGRAMS = vinaLC
else
    bin_PROGRAMS = vina
endif

if USE_MPI

    vinaLC_SOURCES = vinaLC.cpp dock.cpp mpiparser.cpp mainProcedure.cpp
    vinaLC_LDFLAGS = -Wl,-rpath,@mpi_path@/lib/:@boost_dir@/lib/
    vinaLC_LDADD = $(top_builddir)/src/VinaLC/.libs/libVinaLC.a -L/@boost_dir@/lib/ -lboost_system -lboost_thread -lboost_serialization -lboost_filesystem -lboost_program_options -lboost_mpi -L/@mpi_path@/lib/ -lmpi -lmpi_cxx -lz

else
    vina_SOURCES = vina.cpp mainProcedure.cpp
    vina_LDFLAGS = -Wl,-rpath,@boost_dir@/lib/
    vina_LDADD = $(top_builddir)/src/VinaLC/.libs/libVinaLC.a -L/@boost_dir@/lib/ -lboost_system -lboost_thread -lboost_serialization -lboost_filesystem -lboost_program_options -lz

endif
