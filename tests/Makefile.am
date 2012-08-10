if USE_MPI
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include -I/@mpi_path@/include 
else
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include 
endif

if USE_MPI
    bin_PROGRAMS = AmberSplTest # AutoDockSplTest DrugBankSplTest GromacsSplTest MainSplTest NamdSplTest ProteinSplTest ProteinSplTest SstrmSplTest1
else
endif

if USE_MPI
    AmberSplTest_SOURCES = AmberSplTest.cpp
    AmberSplTest_LDADD = -L$(top_srcdir)/src -lLBind -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

#    AutoDockSplTest_SOURCES = AmberSplTest.cpp
#    AutoDockSplTest_LDADD = -L$(top_srcdir)/src -lLbind -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_mpi -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx



else
endif
