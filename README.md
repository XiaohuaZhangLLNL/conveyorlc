# ConveyorLC: A Parallel Virtual Screening Pipeline for Docking and MM/GSBA

## 1. Build and use ConveyorLC with Spack.

### 1.1 Build ConveyorLC with Spack

Download spack program, create a environment for ConveyorLC
```asm
git clone git@github.com:spack/spack.git
. spack/share/spack/setup-env.sh
spack env create conveyorlc
spack env activate conveyorlc
```

Setting up for spack.
```asm
spack mirror add develop https://binaries.spack.io/releases/develop
spack buildcache keys --install --trust
```

specify the dependence for the conveyorlc
```asm
git clone https://github.com/XiaohuaZhangLLNL/spack4atom spack4atom
spack repo add spack4atom
spack repo list
spack add boost@1.72.0+mpi+program_options+system+filesystem+regex+serialization+thread
spack add hdf5+cxx
spack add conveyorlc@master
```
build the all porgrams. It might take a while.
```asm
spack install
``

Some useful command for munally fixing the configuration
```asm
spack cd -e conveyorlc 
spack concretize –force
```

### 1.2 Use ConveyorLC within Spack

The following is the environment setting for running ConveyorLC
```asm
. <install_directory>/spack/share/spack/setup-env.sh
spack env activate conveyorlc
export LBindData="$(dirname `which CDT3Docking`)/../data/"
export PATH=<Amber_installation_directory>/bin/:$PATH
```


## 2. Another way - compile the program and dependent libraries one by one

### 2.1 MPI and Boost libraries are required for ConveyorLC.

ConveyorLC depends on two external libraries to complete the compilation: MPI (https://www.open-mpi.org or https://www.mpich.org) and boost (https://www.boost.org).

Please install the MPI first. Users can use open-mpi or mpich. Either one is OK. Then install the boost. You need to enable the boost-mpi build. In the project-config.jam add one line to the end of the file:
```asm
using mpi : <path_to_your_MPI_installation_directory>/bin/mpicxx ;
```

to install Boost library please follow the step in the Boost document.

```asm
./bootstrap.sh --prefix=path/to/installation/prefix
./b2 install
```

On the LC machines, the MPI and Boost are installed. Users can source the environment and then compile the code.

On quartz:
```asm
module load boost/1.62.0
```

### 2.2 Obtain the code

The code can be download from:

https://github.com/XiaohuaZhangLLNL/conveyorlc

by git:
```asm
git clone git@github.com:XiaohuaZhangLLNL/conveyorlc.git
```


### 2.3 Installation

Installing it by using cmake is straight forward:

```asm
cd conveyorlc
cmake . -DCMAKE_INSTALL_PREFIX:PATH=/usr/gapps/bbs/conveyorlc
make
make install
```

### 2.4 Installation with Spack configuration

Use git clone to download spack4atom

```asm
git clone ssh://git@cz-bitbucket.llnl.gov:7999/xzr/spack4atom.git
```
Add the spack4atom repo to spack

```asm
spack repo add <path to spack4atom repo root>
```

Double check if the repo has been added correctly.
```asm
[zhang30@quartz2306 spack4atom]$ spack repo list
==> 2 package repositories.
atom       /g/g92/zhang30/H/test/spack4atom
builtin    /usr/gapps/bbs/TOSS-3/spack/var/spack/repos/builtin
```

Install the ConveyorLC by spack
```asm
spack install conveyorlc
```

To clean up the installation and re-install
```asm
spack uninstall conveyorlc
spack clean --all conveyorlc
spack install conveyorlc
```

### 2.5 Compile with Conduit and HDF5

Conduit is an open source project from Lawrence Livermore National Laboratory
that provides an intuitive model for describing hierarchical scientific data.
Conduit has I/O interfacces to HDF5.
The purpose to use Conduit in the program is to reduce the number of files.

To install Conduit:

```asm
git clone --recursive https://github.com/llnl/conduit.git
cd conduit
mkdir build
cd build
cmake ../src/ -DCMAKE_INSTALL_PREFIX:PATH=<conduit_installation_dir> -DHDF5_DIR=<Path_to_HDF5_library>
```

Install pipeline with Conduit and HDF5 support:

```asm
cmake ../ -DCMAKE_INSTALL_PREFIX=<conveyorlc_installation_dir> -DBOOST_ROOT=<Path_to_Boost_library> \
-DHDF5_ROOT=<path_to_HDF5_library> -DCONDUIT_DIR=<path_to_Conduit_library>
```

### 2.6 Executables

The executables with Conduit and HDF5 support are
```asm
CDT1Receptor CDT2Ligand   CDT3Docking  CDT4mmgbsa
```

### 2.7 environment setting:

```asm
export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH
```


## 3 Running the code

### 3.1 Submitting scripts

If use Spack with ConveyorLC, please swapp the environment setting with section 1.2

#### 3.1.1 To run the receptor preparation with CDT1Receptor

```asm
#MSUB -A bbs
#MSUB -l nodes=1:ppn=16
#MSUB -l walltime=16:00:00
#MSUB -l partition=syrah
#MSUB -m be

export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH

srun -N 1 -n 16  CDT1Receptor --input  pdb.list --output out
```
The program will detect the cavities on the protein surface. It will need reference point to help identify which cavity is the active site. In the program, there are multiple ways to generate such reference point

* Use a ligand in the active site. The ligand doesn’t need to be big to cover the whole space.
* Use a few key residues in the active site to help identify the active site.
* Use user defined box information.

These input information can be specify in the pdb.list. Please use anyone of the three options in the following pdb.list:
```asm
# Comments start with '#'
# The first column of the input always is the path/pdb_file
# The second column and third one are optional
# To specify non-standard residues in receptor start with "NonRes:"
# nonstandard residues also separated by '|'
#############################################
# 1. Use a ligand in the active site
# srun -N $nnodes -n $procs CDT1Receptor --input  pdb.list --output out --sitebylig on
# in your submitting script use “--sitebylig on”
pdb/protease.pdb SubRes:pdb/ligand.pdb
#############################################
# 2. Use a few key residues
# To specify key residues in the active site to help identify the binding site start with "KeyRes:"
# residues separated by '|' and are in <three-letter-residue-name>.<residue-id>.[<chain-id>] format
# in your submitting scriptt use “--sitebylig on”
# srun -N $nnodes -n $procs CDT1Receptor --input  pdb.list --output out --sitebylig on
pdb/corona.pdb KeyRes:TYR.489
#############################################
# 3. Use user defined box information
#-35.5312,24.1875,4.5 is the coordinates of box center
#16,18,16 is the box dimension.
# in your submitting script don’t use “--sitebylig on”
# srun -N $nnodes -n $procs CDT1Receptor --input  pdb.list --output out
pdb/corona.pdb DockBX:-35.5312,24.1875,4.5|16,18,16
```
Here is the description of general format in pdb.list
```asm
# 1. Comments start with '#'
#
# 2. The first column of the input always is the path/pdb_file
#    The second column and third one are optional
#
pdb/2y2vA_A.pdb
#
# 3. To specify key residues in the active site to help identify the binding site start with "KeyRes:"
#    residues separated by '|' and are in <three-letter-residue-name>.<residue-id>[.<chain-id>] format
#
pdb/2y2vA_A.pdb KeyRes:SGB.203|GLU.202.A
#
# 4. To specify non-standard residues in receptor start with "NonRes:"
#    nonstandard residues also separated by '|'
#    by default it takes off lib files for example SGB (or SBG.O) for SGB.off
#    If mol2 file is used, Please specify it. For example SGB.M for SBG.mol2
#
pdb/2y2vA_A.pdb NonRes:SGB
pdb/2y2vA_A.pdb KeyRes:SGB.203|GLU.202.A NonRes:SGB
pdb/2y2vA_A.pdb KeyRes:SGB.203|GLU.202.A NonRes:SGB.O
pdb/2y2vA_A.pdb KeyRes:SGB.203|GLU.202.A NonRes:SGB.O|NAD.M|GTP.M|FMN.O
#
# 5. To specify substrate ligand for site identification start with "SubRes:" and follow by the relative
#    path to ligand file
#
pdb/sarinXtalnAChE.pdb SubRes:pdb/ligand.pdb  NonRes:SGB
pdb/1a50_protein.pdb SubRes:pdb/1a50_ligand.mol2
#

```

If you have known the docking site and has ligand in active site you can run CDT1Receptor this way:
(for ligand, it can take mol2, pdb, sdf format)
```asm
srun -N 4 -n 64 CDT1Receptor --input  pdb.list --output out --sitebylig on
```
in pdb.list:
```asm
pdb/5os3_protein.pdb SubRes:pdb/5os3_ligand.mol2
pdb/5os0_protein.pdb SubRes:pdb/5os0_ligand.mol2
pdb/5os2_protein.pdb SubRes:pdb/5os2_ligand.mol2
```

You also can use "--minimize off  " if you don't do the MM/GBSA rescoring
```asm
srun -N 4 -n 64 CDT1Receptor --input  pdb.list --output out --minimize off  --sitebylig on
```

Sometimes docking box is not accurate or not you want.
For example, the Aurora kinase binding site is a wide groove but we may want to docking ligand just part of binding site. 
There are two ways to change the docking box.

Use ‘--boxExtend’ to extend the box
```asm
srun -N 1 -n 4 -ppdebug CDT1Receptor --input  pdb.list. --boxExtend 5
```

by default, program adds 2 angstroms to six directions of bounding box of cavity. 
The command above adds 5 angstroms (it has effect on all receptors in pdb.list)

You can define the docking box for individual protein in pdb.list by using “DockBx” keyword
In pdb.list
```asm
pdb/2y2vA_A.pdb KeyRes:SGB.203|GLU.202.A NonRes:SGB
pdb/sarinXtalnAChE_A.pdb KeyRes:SGB.203  NonRes:SGB  DockBX:1.034,-2.453,-3.123|22,25,36
```
“DockBX:1.034,-2.453,-3.123|22,25,36” 
1.034,-2.453,-3.123 is docking box centroid coordinates separated by comma
22,25,36  is docking box dimension separated by comma

If DockBX is defined for a receptor, the grid calculation will be skipped for that receptor and use the user defined docking box.


#### 3.1.2 To run the ligand preparation

```asm
#MSUB -A bbs
#MSUB -l nodes=1:ppn=16
#MSUB -l walltime=16:00:00
#MSUB -l partition=syrah
#MSUB -m be

export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH

srun -N 1 -n 16 CDT2Ligand --sdf pur2.sdf
```
Requirement for the SDF file:
```asm
1. Convert you ligand coordinates into SDF file
2. Add total charge fields for each ligand in SDF file, for example:

> <i_user_TOTAL_CHARGE>
-2

3. After calculation, you can find the ligand parameters in:

scratch/lig/1/LIG.lib, LIG.prmtop,  LIG.inpcrd
scratch/lig/2/LIG.lib, LIG.prmtop,  LIG.inpcrd
...

1, 2, .. correspond to the sequence of ligands appearing in the SDF file

```


#### 3.1.3 To run the docking

```asm
#MSUB -A bbs
#MSUB -l nodes=4:ppn=36
#MSUB -l walltime=16:00:00
#MSUB -l partition=syrah
#MSUB -m be

export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH

srun -N 4 -n 4 -c 36 CDT3Docking --exhaustiveness 36 --num_modes 10
```

#### 2.1.4 To run the MM/GBSA

```asm
#MSUB -A bbs
#MSUB -l nodes=1:ppn=16
#MSUB -l walltime=16:00:00
#MSUB -l partition=syrah
#MSUB -m be

export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH

srun -N 1 -n 16 CDT4mmgbsa 
```


#### 3.1.4 To use the local disk on Quartz to avoid I/O impact on file system

Both docking and rescoring support LOCALDIR on quartz. The only thing you need to do is add the following environment variable to you submitting script:
```asm
export LOCALDIR=/dev/shm/<Your_OUN>/
```

### 3.2 Output data structures

The output data structure is different from XML version.
The calculated data are all in the scratch

```asm
scratch/

    rec/

    receptor.hdf5

    lig/

    ligand.hdf5

    dock/

    dockHDF5/
        dock_proc1.hdf5
        dock_proc2.hdf5
        dock_proc3.hdf5
        ...
        dock_proc<N-1>.hdf5

    gbsa/

    gbsaHDF5/
        gbsa_proc1.hdf5
        gbsa_proc2.hdf5
        gbsa_proc3.hdf5
        ...
        gbsa_proc<N-1>.hdf5

```

rec, lig, dock, gbsa are the sub-directories for temporarily storing
the individual calculation. All the files will be deleted after individual
calculation is completed.

All the necessary data/files for different stage calculations are stored in
HDF5 such as receptor.hdf5, ligand.hdf5, dock_procN.hdf5, and gbsa_procN.hdf5
Receptor preparation will only produce one HDF5 - receptor.hdf5 and so does
ligand preparation - ligand.hdf5.
dock and MM/GBSA will have maximum of N-1 number of the HDF files,
where N is the number of MPI tasks used in the calculation.
Note that normally number of MPI tasks used in MM/GBSA is much larger than
that in the docking calculation.

The HDF5 files can be opened by HDF viewer downloaded from:
https://www.hdfgroup.org/downloads/hdfview/

To visualize the HDF5 file:
```asm
hdfview <your_hdf5_file>
```
Note: this is only a viewer. You cannot modify the files. Please use the following python scripts to do that.

### 3.3 Python scripts

The conveyorLC has been installed:
```asm
/usr/gapps/aha/quartz/conveyorlc_10/
```
The python scripts are in:
```asm
/usr/gapps/aha/quartz/conveyorlc_10/scripts
```

The python command to manipulate the HDF5 files:
```asm
export spack=/usr/gapps/bbs/TOSS-3/spack/bin/
export cdtpy=/usr/gapps/aha/quartz/conveyorlc_10/scripts/
```

CDT1Receptor.py has following options
```asm
$spack/cdtPython.sh $cdtpy/CDT1Receptor.py -h
python /usr/gapps/aha/quartz/conveyorlc_10/scripts//CDT1Receptor.py -h
usage: CDT1Receptor.py [-h] [-i INFILE] [-o OUTFILE] [-d] [-n RECNAME]
                       [-dn DELNAME] [-sn SAVENAME] [-c CHECKDATA CHECKDATA]
                       [-m META]

optional arguments:
  -h, --help            show this help message and exit
  -i INFILE, --in INFILE
                        receptor HDF5 input file
  -o OUTFILE, --out OUTFILE
                        receptor HDF5 output file
  -d, --del             delete all paths for failed calculations
  -n RECNAME, --name RECNAME
                        extract meta data and files by receptor name
  -dn DELNAME, --deletename DELNAME
                        delete path by receptor name
  -sn SAVENAME, --savename SAVENAME
                        save data to HDF5 output file by receptor name
  -c CHECKDATA CHECKDATA, --checkdata CHECKDATA CHECKDATA
                        update meta data by protein name and checkpoint file
                        name (e.g. sarinXtalnAChE checkpoint.txt)
  -m META, --meta META  extract meta data from HDF5 file to CSV file
```

CDT2Ligand.py has following options
```asm
$spack/cdtPython.sh $cdtpy/CDT2Ligand.py -h
python /usr/gapps/aha/quartz/conveyorlc_10/scripts//CDT2Ligand.py -h
usage: CDT2Ligand.py [-h] [-i INFILE] [-o OUTFILE] [-d] [-l LIGID]
                     [-n LIGNAME] [-c CLIST CLIST]

optional arguments:
  -h, --help            show this help message and exit
  -i INFILE, --in INFILE
                        Ligand HDF5 input file
  -o OUTFILE, --out OUTFILE
                        Ligand HDF5 output file
  -d, --del             delete all paths for failed calculations
  -l LIGID, --ligid LIGID
                        extract data and files by ligand ID
  -n LIGNAME, --name LIGNAME
                        extract data and files by ligand name
  -c CLIST CLIST, --clist CLIST CLIST
                        output ligand name and id map into a file and choose
                        only successful one or not (e.g. idnameList.txt T[A])
```

CDT3Docking.py has following options
```asm
$spack/cdtPython.sh $cdtpy/CDT3Docking.py -h
python /usr/gapps/aha/quartz/conveyorlc_10/scripts//CDT3Docking.py -h
usage: CDT3Docking.py [-h] [-i INDIR] [-o OUTDIR] [-r RECNAME] [-l LIGID]
                      [-p PERCENT] [-s SCOREONLY]

optional arguments:
  -h, --help            show this help message and exit
  -i INDIR, --in INDIR  receptor HDF5 input file
  -o OUTDIR, --out OUTDIR
                        receptor HDF5 output file
  -r RECNAME, --recname RECNAME
                        receptor name for extracting data and files (also need
                        ligand id)
  -l LIGID, --ligid LIGID
                        ligand id for extracting data and files (also need
                        receptor name)
  -p PERCENT, --percent PERCENT
                        select top percent ligands into the HDF5 output file
  -s SCOREONLY, --scoreonly SCOREONLY
                        extract score-only data from HDF5 file to CSV file
```


CDT4mmgbsa.py has following options
```asm
$spack/cdtPython.sh $cdtpy/CDT4mmgbsa.py -h
python /usr/gapps/aha/quartz/conveyorlc_10/scripts//CDT4mmgbsa.py -h
usage: CDT4mmgbsa.py [-h] [-i INDIR] [-o OUTDIR] [-r RECNAME] [-l LIGID]
                     [-m META] [-d]

optional arguments:
  -h, --help            show this help message and exit
  -i INDIR, --in INDIR  receptor HDF5 input file
  -o OUTDIR, --out OUTDIR
                        receptor HDF5 output file
  -r RECNAME, --recname RECNAME
                        receptor name for extracting data and files (also need
                        ligand id)
  -l LIGID, --ligid LIGID
                        ligand id for extracting data and files (also need
                        receptor name)
  -m META, --meta META  extract meta data from HDF5 file to CSV file
  -d, --del             delete all paths for failed calculations
```


A tool named hdfx is installed in /usr/gapps/bbs/TOSS-3/hdf_utils/hdfx which can extract information from HDF5 files much faster than python script:
You can use hdfx -h to see the options or ask Dan for detailed information

1. Get scores
```asm
/usr/gapps/bbs/TOSS-3/hdf_utils/hdfx -m gbsaHDF5  > meta_data.txt
```
2. Parse the scores
3. Get complex files and ligand files
```asm
/usr/gapps/bbs/TOSS-3/hdf_utils/hdfx -k gbsa/lv14/286774/p6 -w temp gbsaHDF5
/usr/gapps/bbs/TOSS-3/hdf_utils/hdfx -k lig/286774 -w temp ligand.hdf5
```
132542 is lig id, compound id can be retrieved from the meta_data


## 4 Deprecated executables

### 4.1 Deprecated executables in the pipeline
There will be executables in the ${conveyorlc_install_dir}/bin

```asm
PPL1Receptor  PPL2Ligand  PPL3Docking  PPL4mmgbsa  PPL4PostProcess  PPL4parseXML
```

For the help information of executables

```asm
PPL1Receptor -h
```

### 3.2 Submitting scripts for running pipeline

#### 3.2.1 running the receptor preparation

```asm
#!/bin/bash 
#msub -A bmc
#msub -l nodes=2:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export conveyorlc=/usr/gapps/kras/quartz/pipeline/conveyorlc
export LBindData=$conveyorlc/data
export PATH=$conveyorlc/bin:/usr/gapps/kras/quartz/pipeline/bin:$PATH
export AMBERHOME=/usr/gapps/kras/quartz/amber16
export PATH=$AMBERHOME/bin/:$PATH

test -f /usr/gapps/kras/quartz/amber16/amber.sh && source /usr/gapps/kras/quartz/amber16/amber.sh


srun -N 2 -n 24 PPL1Receptor --input pdb.list --output out

```


#### 4.2.2 running the ligand preparation

```asm
#!/bin/bash 
#msub -A bmc
#msub -l nodes=10:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH

test -f /usr/gapps/kras/quartz/amber16/amber.sh && source /usr/gapps/kras/quartz/amber16/amber.sh


srun -N 10 -n 160 PPL1Receptor --sdf ligand.sdf

```

Requirement for the SDF file:
```asm
1. Convert you ligand coordinates into SDF file
2. Add total charge fields for each ligand in SDF file, for example:

> <i_user_TOTAL_CHARGE>
-2

3. After calculation, you can find the ligand parameters in:

scratch/lig/1/LIG.lib, LIG.prmtop,  LIG.inpcrd
scratch/lig/2/LIG.lib, LIG.prmtop,  LIG.inpcrd
...

1, 2, .. correspond to the sequence of ligands appearing in the SDF file

```

#### 4.2.3 running the docking calculation

```asm
#!/bin/bash 
#msub -A bmc
#msub -l nodes=64:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH

test -f /usr/gapps/kras/quartz/amber16/amber.sh && source /usr/gapps/kras/quartz/amber16/amber.sh


srun -N 64 -n 64 -c 16 PPL3Docking --recXML PPL1Track.xml --ligXML PPL2Track.xml --exhaustiveness 32

```

#### 4.2.4 running the single-point MM/GBSA calculation
```asm
#!/bin/bash 
#msub -A bmc
#msub -l nodes=32:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH

test -f /usr/gapps/kras/quartz/amber16/amber.sh && source /usr/gapps/kras/quartz/amber16/amber.sh


srun -N 32 -n 512  PPL4mmgbsa --comXML PPL3Track.xml

```

#### 4.2.5 zip up the MM/GBSA results by ligand to save space and reduce number of files
The pipeline will generate a lot of files and can cause problem for the file system. PPL4PostProcess tar-zip all mmgbsa poses for the same ligand into one file.
```asm
#!/bin/bash 
#msub -A bmc
#msub -l nodes=32:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export LBindData=/usr/gapps/bbs/TOSS-3/conveyorlc_10/data
export PATH=/usr/gapps/bbs/TOSS-3/conveyorlc_10/bin:/usr/gapps/bbs/TOSS-3/openbabel.3.1.1/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/TOSS-3/amber18/
export PATH=$AMBERHOME/bin/:$PATH

srun -N 4 -n 64  PPL4PostProcess --comXML PPL3Track.xml

```



### 4.3 Output

Summray of outputs are in the XML format.


The protein target preparation output.
```asm
<?xml version="1.0" ?>
<Receptors>
    <!-- Tracking calculation error using XML file -->
    <Receptor>
        <RecID>1ald</RecID>
        <PDBPath>pdb/1ald.pdb</PDBPath>
        <PDBQTPath>scratch/com/1ald/rec/1ald.pdbqt</PDBQTPath>
        <GBEN>-1.0395E+04</GBEN>
        <Site>
            <Cluster>3</Cluster> 
            <Volume>154</Volume>
            <Centroid>
                <X>-48.2348</X>
                <Y>50.1281</Y>
                <Z>47.2788</Z>
            </Centroid>
            <Dimension>
                <X>23.1982</X>
                <Y>21.3827</Y>
                <Z>19.4385</Z>
            </Dimension>
        </Site>
        <Mesg>Finished!</Mesg>
    </Receptor>
    ...
</Receptors>
```

The ligand preparation output.
```asm
<?xml version="1.0" ?>
<Ligands>
    <!-- Tracking calculation error using XML file -->
    <Ligand>
        <LigID>3</LigID>
        <LigName>Sorafenib</LigName>
        <PDBPath>scratch/lig/3/LIG_min.pdb</PDBPath>
        <PDBQTPath>scratch/lig/3/LIG_min.pdbqt</PDBQTPath>
        <GBEN>-100.76</GBEN>
        <Mesg>Finished!</Mesg>
    </Ligand>
    ...
</Ligands>
```

The docking calculation output
```asm
<?xml version="1.0" ?>
<Complexes>
    <!-- Tracking calculation error using XML file -->
    <Complex>
        <RecID>2y2vA_A</RecID>
        <NonStdAAList>
            <NonStdAA id="1">SGB</NonStdAA>
        </NonStdAAList>
        <LigID>2</LigID>
        <LogPath>scratch/com/2y2vA_A/dock/2/scores.log</LogPath>
        <PosePath>scratch/com/2y2vA_A/dock/2/poses.pdbqt</PosePath>
        <Scores>
            <Pose id="1">-9.1</Pose>
            <Pose id="2">-8.7</Pose>
            <Pose id="3">-8.4</Pose>
            <Pose id="4">-8.2</Pose>
            <Pose id="5">-8.1</Pose>
        </Scores>
        <Mesg>Finished!</Mesg>
    </Complex>
    ...
</Complexes>
```

The MM/GSBA calcualtion output

```asm
<?xml version="1.0" ?>
<Complexes>
    <!-- Tracking calculation error using XML file -->
    <Complex>
        <RecID>2y2vA_A</RecID>
        <LigID>3</LigID>
        <PoseID>2</PoseID>
        <Vina>-8.7</Vina>
        <GBSA>29.9987</GBSA>
        <Mesg>Finished!</Mesg>
    </Complex>
...
</Complexes>
```

To convert the MM/GBSA output to a better format:
```asm
PPL4parseXML --inXML PPL4Track.xml --outXML PPL4Parse.xml
```

The "pretty" version the MM/GBSA output

```asm
<?xml version="1.0" ?>
<Complex>
    <!-- Tracking calculation error using XML file -->
    <Receptor RecID="2y2vA_A">
        <Ligand LigID="1" MinVinaPoseID="1" MinVina="-10.2" MinGBPoseID="2" MinGB="5.2934">
            <Pose PoseID="1" Vina="-10.2" GBSA="30.0588">scratch/com/2y2vA_A/gbsa/lig_1/pose_1</Pose>
            <Pose PoseID="2" Vina="-10.2" GBSA="5.2934">scratch/com/2y2vA_A/gbsa/lig_1/pose_2</Pose>
            <Pose PoseID="3" Vina="-10" GBSA="57.1238">scratch/com/2y2vA_A/gbsa/lig_1/pose_3</Pose>
            <Pose PoseID="4" Vina="-9.1" GBSA="17.3861">scratch/com/2y2vA_A/gbsa/lig_1/pose_4</Pose>
            <Pose PoseID="5" Vina="-8.9" GBSA="12.672">scratch/com/2y2vA_A/gbsa/lig_1/pose_5</Pose>
        </Ligand>
        ...
    </Receptor>
</Complex>
```

The calculated data are stored under the scratch directory. The directory structure is:
```asm
scratch/     

    com/                        --- directory for all complexes
        receptor/               --- directory for complex named “receptor” (receptor.pdb)
            dock/               --- directory for docking calculation
                1/              --- docking ligand 1
                2/              --- docking ligand 2
                ...
            gbsa/               --- directory for gbsa calculation
                lig_1/          --- ligand 1
                    pose_1/     --- rescoring pose 1
                    pose_2/     --- rescoring pose 2
                    ...
                lig_2/          --- ligand 2
                    pose_1/     --- rescoring pose 1
                    pose_2/     --- rescoring pose 2
                    ...
            rec/                --- directory for receptor preparation                         
        ...

    lig/                        --- directory for ligand preparation  
        1/                      --- ligand 1 preparation
        2/                      --- ligand 2 preparation
        ...


For the docking complex: the receptor is in scratch/com/receptor/rec/receptor.pdbqt
                                           The ligand  is in  scratch/com/receptor/dock/1/poses.pdbqt  (multiple poses)

For the rescoring comlex : scratch/com/receptor/gbsa/lig_1/pose_1/mmgbsa_results.tar.gz
                                    Unzip the mmgbsa_results.tar.gz. You will find  Com_min.pdb

```

PostProcess MM/GBSA results
```asm
<?xml version="1.0" ?>
<Complexes>
    <!-- Tracking calculation error using XML file -->
    <Complex>
        <RecID>1a6q_A_784_minimized_w_metal5</RecID>
        <LigID>2</LigID>
        <Mesg>Pose 2 has not been completed</Mesg>
    </Complex>
    <Complex>
        <RecID>1a6q_A_784_minimized_w_metal5</RecID>
        <LigID>1</LigID>
        <Mesg>Finished!</Mesg>
    </Complex>
    <Complex>
        <RecID>1a6q_A_784_minimized_w_metal5</RecID>
        <LigID>3</LigID>
        <Mesg>Finished!</Mesg>
    </Complex>
</Complexes>
```

Under the scratch/com/<receptor_name>/gbsa, you will find:
```asm
lig_1.tar.gz  lig_1_checkpoint.txt  lig_2  lig_3.tar.gz  lig_3_checkpoint.txt
```
lig_1 and lig_3 are completed so they are tar-zip. The lig_1 and lig_3 are deleted. Lig_2 has not finished so the folder is unchanged.

```asm
Note:
  1. try to use small number of nodes so that the system is not overwhelming by I/O.
  2. Original files under the lig_1, etc will be deleted.
  3. PPL4PostProcess can be run while PPL4mmgbsa is running. PPL4PostProcess has restart capability.
```


## 5 Deprecated Work with Maestro workflow

The following set up works on Cab/Syrah. For other machines, change the commands accordingly.

### 5.1 Set up python virtual environment

```asm
  use -l python
  use python-2.7.10
  which pip
  pip install --user virtualenv
  pip install --user -U virtualenv
  pip install --user virtualenvwrapper
  export WORKON_HOME=~/.venvs
  pip list
  pip show virtualenvwrapper
  source /g/g92/zhang30/.local/bin/virtualenvwrapper.sh
  echo $WORKON_HOME
```

put the following two command into your .bashrc or .cshrc

```asm
  export WORKON_HOME=~/.venvs
  source /g/g92/zhang30/.local/bin/virtualenvwrapper.sh
```

### 5.2 Create a virtual environment for conveyorLC

```asm
  mkvirtualenv conveyorlc
```

### 5.3 Install maestro workflow

```asm
  git clone git@github.com:LLNL/maestrowf.git
  cd maestrowf/
  git fetch --all
  git checkout -t origin/bugfix/dependency_ordering
  pip install -e .
```

### 5.4 Run the workflow to launch the conveyorlc pipeline

```asm
  workon conveyorlc
  mv run_conveyorlc.yaml ~/W/ConveyorLC/workflow
  cd ~/W/ConveyorLC/workflow
  maestro run -h
  maestro run ./run_conveyorlc.yaml

```

The "maestro run" will crearte a subdirectory using time stamp (i.e. run_conveyorlc_20180504-155630)
To check the job status, run "maestro status"

```asm
(conveyorlc)[zhang30@syrah256 workflow]$  maestro status run_conveyorlc_20180504-155630

Step Name     Workspace                        State              Run Time        Elapsed Time    Start Time                  Submit Time                 End Time                      Number Restarts
------------  -------------------------------  -----------------  --------------  --------------  --------------------------  --------------------------  --------------------------  -----------------
PPL4parseXML  run_conveyorlc_20180504-155630  State.INITIALIZED  --:--:--        --:--:--        --                          --                          --                                          0
PPL1Receptor  run_conveyorlc_20180504-155630  State.FINISHED     0:12:01.079873  0:13:01.157428  2018-05-04 15:57:36.256237  2018-05-04 15:56:36.178682  2018-05-04 16:09:37.336110                  0
PPL4mmgbsa    run_conveyorlc_20180504-155630  State.RUNNING      --:--:--        0:30:03.091956  2018-05-04 16:12:37.699076  2018-05-04 16:11:37.603008  --                                          0
PPL2Ligand    run_conveyorlc_20180504-155630  State.FINISHED     0:01:00.053917  0:02:00.187536  2018-05-04 15:57:36.256309  2018-05-04 15:56:36.122690  2018-05-04 15:58:36.310226                  0
PPL3Docking   run_conveyorlc_20180504-155630  State.FINISHED     0:01:00.096606  0:02:00.202646  2018-05-04 16:10:37.473674  2018-05-04 16:09:37.367634  2018-05-04 16:11:37.570280                  0
```

All the results are under the run_conveyorlc_20180504-155630 subdirectory.


### 5.5 run_conveyorlc.yaml sample file

```asm
description:
    name: run_conveyorlc
    description: |
        Run the conveyorlc code.

env:
    variables:
        OUTPUT_PATH:    ./

    labels:
        SETUP_MAIN: |
            export LBindData=$(CONVEYOR_BIN)/data
            export PATH=$(CONVEYOR_BIN)/bin:$(BIN_HOME)/bin:$PATH
            export AMBERHOME=$(BIN_HOME)/amber18
            export PATH=$AMBERHOME/bin/:$PATH
            export INPUTDIR=$(INPUT_DIR)
            export WORKDIR=$(OUTPUT_DIR)

    dependencies:
        paths:
            - name: BIN_HOME
              path: /usr/gapps/aha/cab

            - name: CONVEYOR_BIN
              path: /usr/gapps/aha/cab/conveyorlc_test

            - name: INPUT_DIR
              path: /g/g92/zhang30/W/ConveyorLC/workflow

batch:
    type: slurm
    host: syrah
    bank: bbs
    queue: pdebug

study:
    - name: PPL1Receptor
      description: Run Lingand preparation step of the pipeline.
      run:
          cmd: |
              $(SETUP_MAIN)
              $(LAUNCHER)[1n, 3p] PPL1Receptor --input $(INPUT_DIR)/pdb.list --output out
          restart: |
              $(SETUP_MAIN)
              $(LAUNCHER)[1n, 3p] PPL1Receptor --input $(INPUT_DIR)/pdb.list --output out
          walltime: "00:30:00"
          nodes: 1
          procs: 3

    - name: PPL2Ligand
      description: Run Lingand preparation step of the pipeline.
      run:
          cmd: |
              $(SETUP_MAIN)
              $(LAUNCHER)[1n, 16p] PPL2Ligand --sdf $(INPUT_DIR)/pur2.sdf
          restart: |
              $(SETUP_MAIN)
              $(LAUNCHER)[1n, 16p] PPL2Ligand --sdf $(INPUT_DIR)/pur2.sdf
          walltime: "00:30:00"
          nodes: 1
          procs: 16

    - name: PPL3Docking
      description: Run docking calculations.
      run:
          cmd: |
              $(SETUP_MAIN)
              $(LAUNCHER)[1n, 4p] PPL3Docking --recXML $(PPL1Receptor.workspace)/PPL1Track.xml --ligXML $(PPL2Ligand.workspace)/PPL2Track.xml --exhaustiveness 2 --num_modes 1
          restart: |
              $(SETUP_MAIN)
              $(LAUNCHER)[1n, 4p] PPL3Docking --recXML $(PPL1Receptor.workspace)/PPL1Track.xml --ligXML $(PPL2Ligand.workspace)/PPL2Track.xml --exhaustiveness 2 --num_modes 1
          depends: [PPL1Receptor, PPL2Ligand]
          walltime: "00:30:00"
          nodes: 1
          procs: 4
          cores per task: 4

    - name: PPL4mmgbsa
      description: Run docking calculations.
      run:
          cmd: |
              $(SETUP_MAIN)
              $(LAUNCHER)[1n, 16p] PPL4mmgbsa --comXML $(PPL3Docking.workspace)/PPL3Track.xml
          restart: |
              $(SETUP_MAIN)
              $(LAUNCHER)[1n, 16p] PPL4mmgbsa --comXML $(PPL3Docking.workspace)/PPL3Track.xml
          depends: [PPL3Docking]
          walltime: "00:30:00"
          nodes: 1
          procs: 16

    - name: PPL4parseXML
      description: Convert PPL4Track.xml to readable XML file.
      run:
          cmd: |
              $(SETUP_MAIN)
              PPL4parseXML --inXML $(PPL4mmgbsa.workspace)/PPL4TrackTemp.xml --outXML PPL4Parse.xml
          depends: [PPL4mmgbsa]

```
!!!
