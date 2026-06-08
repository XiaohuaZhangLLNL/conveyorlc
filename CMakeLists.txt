# stop if cmake version below 3.0
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
cmake_policy(SET CMP0074 NEW)
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()


# project name
project(ConveyorLC)

# enable fortran, c, and c++ language
#enable_language(C CXX)

# project version
set(VERSION_MAJOR 1)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)

set (CMAKE_CXX_STANDARD 14)

# location of additional cmake modules
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# detect operating system
message(STATUS "Building on a ${CMAKE_SYSTEM_NAME} system")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DSYSTEM_LINUX)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_definitions(-DSYSTEM_DARWIN)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-DSYSTEM_WINDOWS)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(CMAKE_BUILD_TYPE Debug)
    set(MPI_C_COMPILER "/Users/zhang30/local/Cellar/mpich/4.0.3/bin/mpicc")
    set(MPI_CXX_COMPILER "/Users/zhang30/local/Cellar/mpich/4.0.3/bin/mpicxx")
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
endif()

# detect host processor
message(STATUS "The host processor is ${CMAKE_HOST_SYSTEM_PROCESSOR}")

# find MPI library
find_package(MPI REQUIRED)


# 3. Explicitly pass MPI hints if FindBoost is still struggling
# Boost 1.60 often needs these to locate the specific MPI-linked binaries.
#set(Boost_MPI_LIBRARIES MPI::MPI_CXX)
#set(Boost_MPI_INCLUDE_DIRS ${MPI_CXX_INCLUDE_DIRS})

if(NOT DEFINED Boost_ROOT AND NOT DEFINED ENV{Boost_ROOT} AND DEFINED ENV{BOOST_ROOT})
    set(Boost_ROOT $ENV{BOOST_ROOT})
endif()

# find boost library
if (NOT DEFINED     BOOST_ROOT        AND
    NOT DEFINED ENV{BOOST_ROOT}       AND
    NOT DEFINED     BOOST_INCLUDEDIR  AND
    NOT DEFINED ENV{BOOST_INCLUDEDIR} AND
    NOT DEFINED     BOOST_LIBRARYDIR  AND
    NOT DEFINED ENV{BOOST_LIBRARYDIR})
    if (APPLE)
        #set(BOOST_ROOT "../../../boost/boost_1_54_0/mac")
        set(BOOST_ROOT "/Users/zhang30/inst/conveyorlc_contrib/boost-1.72.0")
    elseif (WIN32)
        set(BOOST_INCLUDEDIR "C:/local/boost_1_55_0")
        set(BOOST_LIBRARYDIR "C:/local/boost_1_55_0/lib32-msvc-10.0")
    endif()
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30")
    if(POLICY CMP0167)
        cmake_policy(SET CMP0167 NEW)
    endif()

    find_package(Boost 1.60 REQUIRED CONFIG COMPONENTS program_options system filesystem regex serialization mpi thread)
else()
    find_package(Boost 1.60 REQUIRED COMPONENTS program_options system filesystem regex serialization mpi thread)
endif()

message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(OpenBabel3 REQUIRED)
if (NOT OPENBABEL_INCLUDE_DIRS AND NOT OPENBABEL3_INCLUDE_DIRS AND NOT OpenBabel3_INCLUDE_DIRS)
    if(OpenBabel3_LIBRARIES)
        set(RAW_LIB_LIST "${OpenBabel3_LIBRARIES}")
        set(OPENBABEL3_LIBRARIES ${OpenBabel3_LIBRARIES})
    elseif(OPENBABEL3_LIBRARIES)
        set(RAW_LIB_LIST "${OPENBABEL3_LIBRARIES}")
    endif()

    if(RAW_LIB_LIST)
        list(GET RAW_LIB_LIST 0 FIRST_LIB_PATH)
        get_filename_component(OB3_LIB_DIR "${FIRST_LIB_PATH}" DIRECTORY)
        get_filename_component(OB3_PREFIX_DIR "${OB3_LIB_DIR}" DIRECTORY)
        set(OB3_BASE_INCLUDE "${OB3_PREFIX_DIR}/include")
        include_directories(${OB3_BASE_INCLUDE} "${OB3_BASE_INCLUDE}/openbabel3")
    endif()
else()
    if (OPENBABEL_INCLUDE_DIRS)
        include_directories(${OPENBABEL_INCLUDE_DIRS})
    elseif (OPENBABEL3_INCLUDE_DIRS)
        include_directories(${OPENBABEL3_INCLUDE_DIRS}/openbabel3)
    elseif (OpenBabel3_INCLUDE_DIRS)
        include_directories(${OpenBabel3_INCLUDE_DIRS})
    endif()
endif()

# find sqlite3
find_package (sqlite3 QUIET)
IF(SQLITE3_FOUND)
    SET(SQLITE3_LIBRARIES ${SQLITE3_LIBRARY})
    SET(SQLITE3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR})
ELSE(SQLITE3_FOUND)
    SET(SQLITE3_LIBRARIES)
    SET(SQLITE3_INCLUDE_DIRS)
ENDIF(SQLITE3_FOUND)
include_directories(${SQLITE3_INCLUDE_DIRS})

# find zlib
find_package( ZLIB REQUIRED )
include_directories( ${ZLIB_INCLUDE_DIRS} )

# find HDF5

find_package(HDF5 REQUIRED COMPONENTS CXX HL)
include_directories( ${HDF5_INCLUDE_DIRS} )

# find Conduit for handling hierarchical scientific data
if (NOT CONDUIT_DIR)
    if (DEFINED ENV{CONDUIT_DIR})
        set(CONDUIT_DIR $ENV{CONDUIT_DIR})
    elseif (DEFINED ENV{Conduit_DIR})
        set(CONDUIT_DIR $ENV{Conduit_DIR})
    endif ()
endif()

find_package( Conduit REQUIRED )
include_directories(${CONDUIT_INCLUDE_DIRS})

# example how to set c++ compiler flags for GNU
message(STATUS "Using C++ compiler: ${CMAKE_CXX_COMPILER_ID}")
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -DUSE_MPI")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src)
add_subdirectory(apps)

INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION  ${CMAKE_INSTALL_PREFIX} )
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts DESTINATION  ${CMAKE_INSTALL_PREFIX} )
