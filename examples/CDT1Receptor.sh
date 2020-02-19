#!/bin/bash

export LBindData=/usr/gapps/aha/quartz/conveyorlc_10/data
export PATH=/usr/gapps/aha/quartz/conveyorlc_10/bin:/usr/gapps/aha/quartz/bin:$PATH
export AMBERHOME=/usr/gapps/aha/quartz/amber16
export AMBERHOME10=/usr/gapps/aha/quartz/amber10
export PATH=$AMBERHOME/bin/:$AMBERHOME10/bin/:$PATH


#srun -N 1 -n 4 -ppdebug $LBindData/../bin/PPL1Receptor --input  pdb.list --output out
#srun -N 2 -n 24 -ppdebug PPL1Receptor --input  pdb.list --output out

#srun -N 1 -n 4 -ppdebug PPL1Receptor --input  pdb.list --output out --version 13  --minimize on --forceRedo on

#srun -N 1 -n 4 -ppdebug PPL1Receptor --input  pdb.list --output out --version 13 --spacing 2.0 --cutProt on 
#srun -N 1 -n 4 -ppdebug PPL1Receptor --input  pdb.list --output out --version 13 --spacing 2.0 --minimize off --forceRedo on --cutRadius 10 --cutProt on 

srun -N 1 -n 2 -ppdebug CDT1Receptor --input  pdb.list --output out --version 13 --spacing 1.4 --minimize on --forceRedo on
