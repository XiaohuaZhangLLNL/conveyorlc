if USE_MPI
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include -I/@mpi_path@/include 
else
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include 
endif

if USE_MPI
    bin_PROGRAMS = AmberSplTest PPL1ReceptorTest StdResContainerSplTest JobTrackSplTest# AutoDockSplTest DrugBankSplTest GromacsSplTest MainSplTest NamdSplTest ProteinSplTest ProteinSplTest SstrmSplTest1
else
endif

if USE_MPI
    AmberSplTest_SOURCES = AmberSplTest.cpp
    AmberSplTest_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx -lz

    PPL1ReceptorTest_SOURCES = PPL1ReceptorTest.cpp
    PPL1ReceptorTest_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx -lz

    StdResContainerSplTest_SOURCES = StdResContainerSplTest.cpp
    StdResContainerSplTest_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -lz

    JobTrackSplTest_SOURCES = JobTrackSplTest.cpp
    JobTrackSplTest_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -lz

#    AutoDockSplTest_SOURCES = AmberSplTest.cpp
#    AutoDockSplTest_LDADD = -L$(top_builddir)/src -lLbind -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_mpi -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx



else
endif
