#!/bin/bash

set -e

module purge

# Runtime modules - Must be loaded at runtime unless dependencies are
# RPATH'd into the build (See below. Recommended to load them and not mess with
# RPATHs unless absolutely necessary).
module load gcc/8.3.1 spectrum-mpi \
  cmake netcdf/4.7.0  fftw

# Buildtime modules - Probably only needed at buildtime.
module load flex git

SRCDIR="${SRCDIR:-/g/g92/zhang30/W/kras/AMBER20/amber20_src}"
BUILDDIR="${BUILDDIR:-/g/g92/zhang30/W/kras/AMBER20/amber20_src/build}"
MYCONDAENV="${MYCONDAENV:-/usr/gapps/kras/sierra/amber20_cpu/miniconda}"
INSTALLDIR="${INSTALLDIR:-/usr/gapps/kras/sierra/amber20_cpu}"

#rm -fr "${INSTALLDIR}"
#rm -fr "${BUILDDIR}"
mkdir -p "${BUILDDIR}"

# Optional: Use own conda env for python dependency
if false; then
  # Setup conda env
  if [ ! -f "${MYCONDAENV}/bin/python" ]; then
    conda create -p "${MYCONDAENV}" python=3.6 numpy scipy matplotlib setuptools
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
ccmake ".. ${EXTRA_CMAKE_OPTS:-}" \
  -DBUILD_HOST_TOOLS=False \
  -DCMAKE_INSTALL_PREFIX="${INSTALLDIR}" \
  -DCMAKE_C_COMPILER="${OLCF_GCC_ROOT}/bin/gcc" \
  -DCMAKE_CXX_COMPILER="${OLCF_GCC_ROOT}/bin/g++" \
  -DCMAKE_Fortran_COMPILER="${OLCF_GCC_ROOT}/bin/gfortran" \
  -DCOMPILER=MANUAL \
  -DCUDA=False \
  -DOPENMP=True \
  -DMPI=True \
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



