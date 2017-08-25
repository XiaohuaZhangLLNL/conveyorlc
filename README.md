# ConveyorLC: A Parallel Virtual Screening Pipeline for Docking and MM/GSBA


## 1. Compile the program.

This source code is configured to run on LLNL LC machines. 

### 1.1 Boost library (www.boost.org) is require for ConveyorLC.
to install Boost library please follow the step in the Boost document. 

```
./bootstrap.sh --prefix=path/to/installation/prefix
./b2 install
```

Beside the standard installation, Boost MPI binding also need to be turn on.
copy tools/build/v2/user-config.jam to your home directory. In the file 
specify the mpi compiler you want to use:

```
using mpi : /usr/local/tools/mvapich-gnu/bin/mpicxx ;
```

On quartz, you can use LC-precompiled boost library by:
```
module load boost/1.62.0
```
### 1.2 Obtain the code 

The code can be download from:

https://lc.llnl.gov/bitbucket/projects/XZR/repos/conveyorlc/browse

by git:
```
git clone ssh://git@cz-bitbucket.llnl.gov:7999/xzr/conveyorlc.git
```


### 1.3 Installation

Installing it by using cmake is straight forward:

```
cd conveyorlc
cmake . -DCMAKE_INSTALL_PREFIX:PATH=/usr/gapps/bbs/conveyorlc
make
make install
```



