# FindOpenBabel3.cmake
#
#  OPENBABEL3_FOUND - system has Open Babel
#  OPENBABEL3_INCLUDE_DIRS - the Open Babel include directories
#  OPENBABEL3_LIBRARIES - Link these to use Open Babel

if(OPENBABEL3_INCLUDE_DIRS AND OPENBABEL3_LIBRARIES)
  # in cache already or user-specified
  set(OPENBABEL3_FOUND TRUE)

else()

  if(NOT OPENBABEL3_INCLUDE_DIRS)
    if(WIN32)
      find_path(OPENBABEL3_INCLUDE_DIR openbabel/obconversion.h
              PATHS
              ${OPENBABEL3_ROOT}\\include
              $ENV{OPENBABEL3_INCLUDE_DIR}
              $ENV{OPENBABEL3_INCLUDE_DIR}\\openbabel3
              $ENV{OPENBABEL3_INCLUDE_PATH}
              $ENV{OPENBABEL3_INCLUDE_PATH}\\openbabel3
              $ENV{OPENBABEL3_ROOT}\\include
              C:\\OpenBabel\\include
              )
      find_path(OPENBABEL3_INCLUDE_DIR2 openbabel/babelconfig.h
              PATHS
              ${OPENBABEL3_ROOT}/build/include
              ${OPENBABEL3_ROOT}/windows-vc2008/build/include
              $ENV{OPENBABEL3_ROOT}/windows-vc2008/build/include
              )
      set(OPENBABEL3_INCLUDE_DIRS ${OPENBABEL3_INCLUDE_DIR} ${OPENBABEL3_INCLUDE_DIR2})
    else()
      find_path(OPENBABEL3_INCLUDE_DIRS openbabel/obconversion.h
              PATHS
              ${OPENBABEL3_ROOT}/include/openbabel3
              ${OPENBABEL3_ROOT}/include
              $ENV{OPENBABEL3_INCLUDE_DIR}/openbabel3
              $ENV{OPENBABEL3_INCLUDE_DIR}
              $ENV{OPENBABEL3_INCLUDE_PATH}/openbabel3
              $ENV{OPENBABEL3_INCLUDE_PATH}
              $ENV{OPENBABEL3_ROOT}/include/openbabel3
              $ENV{OPENBABEL3_ROOT}/include
              $ENV{OPENBABEL3_PATH}/include/openbabel3
              $ENV{OPENBABEL3_PATH}/include
              $ENV{OPENBABEL3_BASE}/include/openbabel3
              $ENV{OPENBABEL3_BASE}/include
              /usr/include
              /usr/local/include
              /usr/local/openbabel/include/openbabel3
              /usr/local/openbabel/include
              )
    endif()
    if(OPENBABEL3_INCLUDE_DIRS)
      message(STATUS "Found Open Babel include files at ${OPENBABEL3_INCLUDE_DIRS}")
    endif()
  endif()

  if(NOT OPENBABEL3_LIBRARY_DIR)
    find_library(OPENBABEL3_LIBRARIES NAMES openbabel openbabel3
            PATHS
            ${OPENBABEL3_ROOT}/lib
            ${OPENBABEL3_ROOT}/windows-vc2008/build/src/Release
            ${OPENBABEL3_ROOT}/build/src/Release
            $ENV{OPENBABEL3_LIBRARIES}
            $ENV{OPENBABEL3_ROOT}/lib
            $ENV{OPENBABEL3_ROOT}/windows-vc2008/build/src/Release
            $ENV{OPENBABEL3_PATH}/lib
            $ENV{OPENBABEL3_BASE}/lib
            /usr/lib
            /usr/local/lib
            ~/lib
            $ENV{LD_LIBRARY_PATH}
            )
    if(OPENBABEL3_LIBRARIES)
      message(STATUS "Found Open Babel library at ${OPENBABEL3_LIBRARIES}")
    endif()
  endif()

  if(OPENBABEL3_INCLUDE_DIRS AND OPENBABEL3_LIBRARIES)
    set(OPENBABEL3_FOUND TRUE)
  endif()

  mark_as_advanced(OPENBABEL3_INCLUDE_DIRS OPENBABEL3_LIBRARIES
          OPENBABEL3_INCLUDE_DIR OPENBABEL3_INCLUDE_DIR2)
endif()