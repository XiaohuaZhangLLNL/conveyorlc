CFLAGS = -Wall 

if USE_MPI
INCLUDES = -I$(top_srcdir)/src -I@boost_dir@/include -I/@mpi_path@/include
else
INCLUDES = -I$(top_srcdir)/src -I@boost_dir@/include 
endif

BOOST_LIB = -L/@boost_dir@/lib/ -lboost_regex  -lz 

#MM_LIB= -L/@mm_dir@/lib/ -lpcm

if USE_MPI
MPI_LIB= -L/@mpi_path@/lib/ -lmpi -lmpi_cxx
endif

lib_LTLIBRARIES = libLBind.la

libLBind_la_SOURCES = \
BackBone/ADR.cpp \
BackBone/Base.cpp \
BackBone/Grid.cpp \
BackBone/Ligand.cpp \
BackBone/Pathway.cpp \
BackBone/Protein.cpp \
BackBone/Surface.cpp \
Common/Control.cpp \
Common/LBindException.cpp \
MM/Amber.cpp \
MM/AutoDock.cpp \
MM/Gromacs.cpp \
MM/Namd.cpp \
MM/SpMMGBSA.cpp  \
MM/SpMMPBSA.cpp  \
MM/VinaLC.cpp  \
Parser/BaseParser.cpp  \
Parser/ElementParser.cpp  \
Parser/GZstream.cpp   \
Parser/Mol2.cpp  \
Parser/Pdb.cpp  \
Parser/SanderOutput.cpp \
Parser/Sdf.cpp  \
Structure/Atom.cpp \
Structure/BaseStruct.cpp \
Structure/BondContainer.cpp \
Structure/Bond.cpp \
Structure/Complex.cpp \
Structure/Connectivity.cpp \
Structure/Coor3d.cpp \
Structure/Coordinates.cpp \
Structure/ElementContainer.cpp \
Structure/Element.cpp \
Structure/Fragment.cpp \
Structure/Logger.cpp \
Structure/Molecule.cpp \
Structure/ParmContainer.cpp \
Structure/PMolException.cpp \
Structure/RingContainer.cpp \
Structure/Ring.cpp \
Structure/StdResContainer.cpp   \
XML/XMLAttribute.cpp \
XML/XMLAttributeSet.cpp \
XML/XMLBase.cpp \
XML/XMLComment.cpp \
XML/XMLDeclaration.cpp \
XML/XMLDocument.cpp \
XML/XMLElement.cpp \
XML/XMLHandle.cpp \
XML/XMLNode.cpp \
XML/XMLParsingData.cpp \
XML/XMLPrinter.cpp \
XML/XMLText.cpp \
XML/XMLUnknown.cpp 

libLBindincludedir = $(includedir)/

libLBindinclude_HEADERS = \
BackBone/ADR.h \
BackBone/Base.h \
BackBone/Grid.h \
BackBone/Ligand.h \
BackBone/Pathway.h \
BackBone/Protein.h \
BackBone/Surface.h \
Common/Chomp.hpp \
Common/Control.h \
Common/File.hpp  \
Common/LBindException.h \
Common/Tokenize.hpp \
MM/Amber.h \
MM/AutoDock.h \
MM/Gromacs.h \
MM/Namd.h \
MM/SpMMGBSA.h  \
MM/SpMMPBSA.h  \
MM/VinaLC.h  \
Parser/BaseParser.h  \
Parser/ElementParser.h  \
Parser/GZstream.h   \
Parser/Mol2.h  \
Parser/Pdb.h  \
Parser/SanderOutput.h \
Parser/Sdf.h  \
Structure/Atom.h \
Structure/BaseStruct.h \
Structure/BondContainer.h \
Structure/Bond.h \
Structure/Complex.h \
Structure/Connectivity.h \
Structure/Constants.h \
Structure/Coor3d.h \
Structure/Coordinates.h \
Structure/DataStruct.h \
Structure/ElementContainer.h \
Structure/Element.h \
Structure/Fragment.h \
Structure/Logger.h \
Structure/Molecule.h \
Structure/ParmContainer.h \
Structure/PMolException.h \
Structure/RingContainer.h \
Structure/Ring.h \
Structure/StdResContainer.h   \
XML/XMLAttribute.h \
XML/XMLAttributeSet.h \
XML/XMLBase.h \
XML/XMLComment.h \
XML/XMLDeclaration.h \
XML/XMLDocument.h \
XML/XMLElement.h \
XML/XMLHandle.h \
XML/XMLNode.h \
XML/XMLParsingData.h \
XML/XMLPrinter.h \
XML/XMLText.h \
XML/XMLUnknown.h 


