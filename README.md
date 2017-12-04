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

## 2. Running the code

### 2.1 Executables in the pipeline
There will be executables in the ${conveyorlc_install_dir}/bin

```
PPL1Receptor  PPL2Ligand  PPL3Docking  PPL4mmgbsa  PPL4parseXML
```

For the help information of executables 

```
PPL1Receptor -h
```

### 2.2 Submitting scripts for running pipeline

#### 2.2.1 running the receptor preparation

```
#!/bin/bash 
#msub -A bmc
#msub -l nodes=2:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export LBindData=/usr/gapps/bbs/zhang30/data/
export PATH=/usr/gapps/bbs/zhang30/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/zhang30/amber/amber10_all
export PATH=$AMBERHOME/bin/:$PATH

srun -N 2 -n 24 PPL1Receptor --input pdb.list --output out

```

#### 2.2.2 running the ligand preparation

```
#!/bin/bash 
#msub -A bmc
#msub -l nodes=10:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export LBindData=/usr/gapps/bbs/zhang30/data/
export PATH=/usr/gapps/bbs/zhang30/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/zhang30/amber/amber10_all
export PATH=$AMBERHOME/bin/:$PATH

srun -N 10 -n 160 PPL1Receptor --sdf ligand.sdf

```

#### 2.2.3 running the docking calculation

```
#!/bin/bash 
#msub -A bmc
#msub -l nodes=64:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export LBindData=/usr/gapps/bbs/zhang30/data/
export PATH=/usr/gapps/bbs/zhang30/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/zhang30/amber/amber10_all
export PATH=$AMBERHOME/bin/:$PATH

srun -N 64 -n 64 -c 16 PPL3Docking --recXML PPL1Track.xml --ligXML PPL2Track.xml --exhaustiveness 32

```

#### 2.2.4 runing the single-point MM/GBSA calculation
```
#!/bin/bash 
#msub -A bmc
#msub -l nodes=32:ppn=16
#msub -l walltime=16:00:00
#msub -l partition=cab
#msub -m be

export LBindData=/usr/gapps/bbs/zhang30/data/
export PATH=/usr/gapps/bbs/zhang30/bin:$PATH
export AMBERHOME=/usr/gapps/bbs/zhang30/amber/amber10_all
export PATH=$AMBERHOME/bin/:$PATH

srun -N 32 -n 512  PPL4mmgbsa --comXML PPL3Track.xml

```

### 2.3 Output

Summray of outputs are in the XML format.


The protein target preparation output.
```
<?xml version="1.0" ?>
<Receptors>
    <!-- Tracking calculation error using XML file -->
    <Receptor>
        <RecID>1ald</RecID>
        <PDBPath>pdb/1ald.pdb</PDBPath>
        <PDBQTPath>scratch/com/1ald/rec/1ald.pdbqt</PDBQTPath>
        <GBEN>-1.0395E+04</GBEN>
        <Site>
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
```
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
```
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

```
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
```
PPL4parseXML --inXML PPL4Track.xml --outXML PPL4Parse.xml
```

The "pretty" version the MM/GBSA output

```
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

