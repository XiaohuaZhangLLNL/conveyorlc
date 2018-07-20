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

### 1.4 Installation with Spack configuration

Use git clone to download spack4atom

```
git clone ssh://git@cz-bitbucket.llnl.gov:7999/xzr/spack4atom.git
```
Add the spack4atom repo to spack
 
```
spack repo add <path to spack4atom repo root>
```

Double check if the repo has been added correctly.
```
[zhang30@quartz2306 spack4atom]$ spack repo list
==> 2 package repositories.
atom       /g/g17/fdinatal/repos/spack4atom
builtin    /g/g17/fdinatal/repos/spack/var/spack/repos/builtin
```

Install the ConveyorLC by spack
```
spack install conveyorlc
```

To clean up the installation and re-install
```
spack uninstall conveyorlc
spack clean --all conveyorlc
spack install conveyorlc
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

export conveyorlc=/usr/gapps/kras/quartz/pipeline/conveyorlc
export LBindData=$conveyorlc/data
export PATH=$conveyorlc/bin:/usr/gapps/kras/quartz/pipeline/bin:$PATH
export AMBERHOME=/usr/gapps/kras/quartz/amber16
export PATH=$AMBERHOME/bin/:$PATH

test -f /usr/gapps/kras/quartz/amber16/amber.sh && source /usr/gapps/kras/quartz/amber16/amber.sh


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

export conveyorlc=/usr/gapps/kras/quartz/pipeline/conveyorlc
export LBindData=$conveyorlc/data
export PATH=$conveyorlc/bin:/usr/gapps/kras/quartz/pipeline/bin:$PATH
export AMBERHOME=/usr/gapps/kras/quartz/amber16
export PATH=$AMBERHOME/bin/:$PATH

test -f /usr/gapps/kras/quartz/amber16/amber.sh && source /usr/gapps/kras/quartz/amber16/amber.sh


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

export conveyorlc=/usr/gapps/kras/quartz/pipeline/conveyorlc
export LBindData=$conveyorlc/data
export PATH=$conveyorlc/bin:/usr/gapps/kras/quartz/pipeline/bin:$PATH
export AMBERHOME=/usr/gapps/kras/quartz/amber16
export PATH=$AMBERHOME/bin/:$PATH

test -f /usr/gapps/kras/quartz/amber16/amber.sh && source /usr/gapps/kras/quartz/amber16/amber.sh


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

export conveyorlc=/usr/gapps/kras/quartz/pipeline/conveyorlc
export LBindData=$conveyorlc/data
export PATH=$conveyorlc/bin:/usr/gapps/kras/quartz/pipeline/bin:$PATH
export AMBERHOME=/usr/gapps/kras/quartz/amber16
export PATH=$AMBERHOME/bin/:$PATH

test -f /usr/gapps/kras/quartz/amber16/amber.sh && source /usr/gapps/kras/quartz/amber16/amber.sh


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

The calculated data are stored under the scratch directory. The directory structure is:

```
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

## 3 Work with Maestro workflow

The following set up works on Cab/Syrah. For other machines, change the commands accordingly.

### 3.1 Set up python virtual environment

```
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

```
  export WORKON_HOME=~/.venvs
  source /g/g92/zhang30/.local/bin/virtualenvwrapper.sh
```

### 3.2 Create a virtual environment for conveyorLC

```
  mkvirtualenv conveyorlc
```

### 3.3 Install maestro workflow

```
  git clone git@github.com:LLNL/maestrowf.git
  cd maestrowf/
  git fetch --all
  git checkout -t origin/bugfix/dependency_ordering
  pip install -e .
```

### 3.4 Run the workflow to launch the conveyorlc pipeline 

```
  workon conveyorlc
  mv run_converyorlc.yaml ~/W/ConveyorLC/workflow
  cd ~/W/ConveyorLC/workflow
  maestro run -h
  maestro run ./run_converyorlc.yaml

```

The "maestro run" will crearte a subdirectory using time stamp (i.e. run_converyorlc_20180504-155630)
To check the job status, run "maestro status"

```
(conveyorlc)[zhang30@syrah256 workflow]$  maestro status run_converyorlc_20180504-155630

Step Name     Workspace                        State              Run Time        Elapsed Time    Start Time                  Submit Time                 End Time                      Number Restarts
------------  -------------------------------  -----------------  --------------  --------------  --------------------------  --------------------------  --------------------------  -----------------
PPL4parseXML  run_converyorlc_20180504-155630  State.INITIALIZED  --:--:--        --:--:--        --                          --                          --                                          0
PPL1Receptor  run_converyorlc_20180504-155630  State.FINISHED     0:12:01.079873  0:13:01.157428  2018-05-04 15:57:36.256237  2018-05-04 15:56:36.178682  2018-05-04 16:09:37.336110                  0
PPL4mmgbsa    run_converyorlc_20180504-155630  State.RUNNING      --:--:--        0:30:03.091956  2018-05-04 16:12:37.699076  2018-05-04 16:11:37.603008  --                                          0
PPL2Ligand    run_converyorlc_20180504-155630  State.FINISHED     0:01:00.053917  0:02:00.187536  2018-05-04 15:57:36.256309  2018-05-04 15:56:36.122690  2018-05-04 15:58:36.310226                  0
PPL3Docking   run_converyorlc_20180504-155630  State.FINISHED     0:01:00.096606  0:02:00.202646  2018-05-04 16:10:37.473674  2018-05-04 16:09:37.367634  2018-05-04 16:11:37.570280                  0
```

All the results are under the run_converyorlc_20180504-155630 subdirectory.


### 3.4 run_converyorlc.yaml sample file

```
description:
    name: run_converyorlc
    description: |
        Run the conveyorlc code.

env:
    variables:
        OUTPUT_PATH:    ./

    labels:
        SETUP_MAIN: |
            export LBindData=$(CONVERYOR_BIN)/data
            export PATH=$(CONVERYOR_BIN)/bin:$(BIN_HOME)/bin:$PATH
            export AMBERHOME=$(BIN_HOME)/amber10
            export PATH=$AMBERHOME/bin/:$PATH
            export INPUTDIR=$(INPUT_DIR)
            export WORKDIR=$(OUTPUT_DIR)

    dependencies:
        paths:
            - name: BIN_HOME
              path: /usr/gapps/aha/cab

            - name: CONVERYOR_BIN
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

