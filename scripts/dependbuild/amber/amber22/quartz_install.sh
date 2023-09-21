#!/bin/bash

set -e

module purge

# Runtime modules - Must be loaded at runtime unless dependencies are
# RPATH'd into the build (See below. Recommended to load them and not mess with
# RPATHs unless absolutely necessary).
module load gcc/12.1.1 \
  cmake/3.23.1 netcdf-c-serial/4.9.0 

# Buildtime modules - Probably only needed at buildtime.
#module load flex git
module load git

SRCDIR="${SRCDIR:-/g/g92/zhang30/W/kras/AMBER22/standard/amber22_src}"
BUILDDIR="${BUILDDIR:-/g/g92/zhang30/W/kras/AMBER22/standard/amber22_src/build_quartz}"
MYCONDAENV="${MYCONDAENV:-/usr/gapps/bbs/TOSS-4/amber22/miniconda3/}"
INSTALLDIR="${INSTALLDIR:-/usr/gapps/bbs/TOSS-4/amber22/}"

#rm -fr "${INSTALLDIR}"
#rm -fr "${BUILDDIR}"
mkdir -p "${BUILDDIR}"

# Optional: Use own conda env for python dependency
if false; then
  # Setup conda env
  if [ ! -f "${MYCONDAENV}/bin/python" ]; then
    conda create -p "${MYCONDAENV}" python=3.10.9 numpy scipy matplotlib setuptools
    . activate "${MYCONDAENV}"
    pip install mpi4py
  fi

  # Activate conda env
  if [ -z "${CONDA_PREFIX:-}" ]; then
    . activate "${MYCONDAENV}"
  elif [ "${CONDA_PREFIX}" != "${MYCONDAENV}" ]; then
    conda deactivate
    . activate "${MYCONDAENV}"
  fi
fi

PYTHON_BIN="$(which python3)"
echo "Using python from '${PYTHON_BIN}'"

cd "${BUILDDIR}"

# To add RPATHs to dependencies, you may use either of the following approaches
# to get CMake to insert them.
#   ```
#   -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE \
#   -DCMAKE_INSTALL_RPATH="$OLCF_<PKG>_ROOT/lib[64][:$OLCF_<PKG>_ROOT/lib[64]...]"
#   ```
# where the lib dir for each dependency (including the correct dirname lib vs
# lib64) is included in the list. This approach doesn't always work, however.
# Depending on how the Amber source was packaged for CMake, it could be
# over-ridden internally by the CMake build system. Alternatively, one can add
# the RPATHs directly via link flags, but this is not advised because it is
# indiscriminate among the build targets with respect to their actual
# dependencies and the order packages are listed may matter.
#   ```
#   -DCMAKE_<LANG>_LINK_FLAGS="-Wl,-rpath,$OLCF_<PKG>_ROOT/lib[64] -L$OLCF_<PKG>_ROOT/lib[64]"
#   ```

# EXTRA_CMAKE_OPTS="-DCMAKE_VERBOSE_MAKEFILE=True"
#export CC=/usr/tce/packages/gcc/gcc-8.3.1/bin/gcc 
#export CXX=/usr/tce/packages/gcc/gcc-8.3.1/bin/g++ 
#export FC=/usr/tce/packages/gcc/gcc-8.3.1/bin/gfortran 
export OLCF_GCC_ROOT=/usr/tce/packages/gcc/gcc-12.1.1/
ccmake ".. ${EXTRA_CMAKE_OPTS:-}" \
  -DBUILD_HOST_TOOLS=False \
  -DCMAKE_INSTALL_PREFIX="${INSTALLDIR}" \
  -DCMAKE_C_COMPILER="${OLCF_GCC_ROOT}/bin/gcc" \
  -DCMAKE_CXX_COMPILER="${OLCF_GCC_ROOT}/bin/g++" \
  -DCMAKE_Fortran_COMPILER="${OLCF_GCC_ROOT}/bin/gfortran" \
  -DCOMPILER=MANUAL \
  -DCUDA=False \
  -DOPENMP=True \
  -DMPI=False \
  -DUSE_FFT=True \
  -DBUILD_DEPRECATED=False \
  -DBUILD_INDEV=False \
  -DBUILD_GUI=False \
  -DBUILD_PERL=True \
  -DBUILD_PYTHON=True \
  -DFORCE_EXTERNAL_LIBS="boost;netcdf" \
  -DPYTHON_EXECUTABLE="${PYTHON_BIN}" \
  -DOPTIMIZE=True \
  -DBUILD_TESTING=True \
  "${SRCDIR}" | tee build.log

#make -j8 | tee -a build.log
#make install | tee -a build.log



