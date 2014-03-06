if USE_MPI
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include -I/@mpi_path@/include 
else
    INCLUDES = -I. -I$(top_srcdir)/src -I/@boost_dir@/include 
endif

if USE_MPI
    bin_PROGRAMS = amberTools preLigands mmpbsa mmgbsa preReceptors processVinaLC glideGBSA
else
endif

if USE_MPI
    amberTools_SOURCES = amberTools.cpp amberToolsPO.cpp
    amberTools_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    preLigands_SOURCES = preLigands.cpp preLigandsPO.cpp
    preLigands_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    mmpbsa_SOURCES = mmpbsa.cpp mmpbsaPO.cpp
    mmpbsa_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    mmgbsa_SOURCES = mmgbsa.cpp mmgbsaPO.cpp
    mmgbsa_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx


    preReceptors_SOURCES = preReceptors.cpp
    preReceptors_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx 

    processVinaLC_SOURCES = processVinaLC.cpp
    processVinaLC_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_program_options -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx  -lz

    glideGBSA_SOURCES = glideGBSA.cpp glideGBSAPO.cpp
    glideGBSA_LDADD = $(top_builddir)/src/.libs/libLBind.a -L/@boost_dir@/lib/ -lboost_system -lboost_program_options -lboost_filesystem -lboost_regex -lboost_serialization -L/@mpi_path@/lib/ -lmpi -lmpi_cxx


else
endif
