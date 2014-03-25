if USE_MPI
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include -I/@mpi_path@/include 
else
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include 
endif

if USE_MPI
    bin_PROGRAMS = PPL1Receptor PPL2Ligand
else
endif

if USE_MPI
    PPL1Receptor_SOURCES = PPL1Receptor.cpp PPL1ReceptorPO.cpp
    PPL1Receptor_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    PPL2Ligand_SOURCES = PPL2Ligand.cpp PPL2LigandPO.cpp
    PPL2Ligand_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 


else
endif
