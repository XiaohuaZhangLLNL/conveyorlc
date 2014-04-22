if USE_MPI
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include -I/@mpi_path@/include 
else
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include 
endif

if USE_MPI
    bin_PROGRAMS = PPL1Receptor PPL2Ligand PPL3Docking
else
endif

if USE_MPI
    PPL1Receptor_SOURCES = PPL1Receptor.cpp PPL1ReceptorPO.cpp
    PPL1Receptor_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -lboost_mpi -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    PPL2Ligand_SOURCES = PPL2Ligand.cpp PPL2LigandPO.cpp
    PPL2Ligand_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -lboost_mpi -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    PPL3Docking_SOURCES = PPL3Docking.cpp dock.cpp mpiparser.cpp mainProcedure.cpp
    PPL3Docking_LDFLAGS = -Wl,-rpath,@mpi_path@/lib/:@boost_dir@/lib/
    PPL3Docking_LDADD = $(top_builddir)/src/VinaLC/.libs/libVinaLC.a $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_regex -lboost_thread -lboost_serialization -lboost_filesystem -lboost_program_options -lboost_mpi -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

else
endif
