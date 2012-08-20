if USE_MPI
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include -I/@mpi_path@/include 
else
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include 
endif

if USE_MPI
    bin_PROGRAMS = amberTools premmpbsa mmpbsa preVinaLC
else
endif

if USE_MPI
    amberTools_SOURCES = amberTools.cpp
    amberTools_LDADD = -L$(top_srcdir)/src -lLBind -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    premmpbsa_SOURCES = premmpbsa.cpp
    premmpbsa_LDADD = -L$(top_srcdir)/src -lLBind -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    mmpbsa_SOURCES = mmpbsa.cpp
    mmpbsa_LDADD = -L$(top_srcdir)/src -lLBind -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    preVinaLC_SOURCES = preVinaLC.cpp
    preVinaLC_LDADD = -L$(top_srcdir)/src -lLBind -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 


else
endif
