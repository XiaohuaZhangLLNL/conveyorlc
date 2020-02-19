#!/bin/bash

export LBindData=/usr/gapps/aha/quartz/conveyorlc_10/data
export PATH=/usr/gapps/aha/quartz/conveyorlc_10/bin:/usr/gapps/aha/quartz/bin:$PATH
export AMBERHOME=/usr/gapps/aha/quartz/amber16
export AMBERHOME10=/usr/gapps/aha/quartz/amber10
export PATH=$AMBERHOME/bin/:$AMBERHOME10/bin/:$PATH

# 1. Convert you ligand coordinates into SDF file
# 2. Add total charge fields for each ligand in SDF file

#> <i_user_TOTAL_CHARGE>
#-2


#srun -N 1 -n 3  -ppdebug PPL2Ligand --sdf pur2.sdf
srun -N 1 -n 10  -ppdebug CDT2Ligand --sdf pur2.sdf --version 13 
#srun -N 1 -n 10  -ppdebug PPL2Ligand --sdf test.sdf


# 3. You can find ligand gaff force field:

#scratch/lig/1/LIG.lib, LIG.prmtop,  LIG.inpcrd
#scratch/lig/2/LIG.lib, LIG.prmtop,  LIG.inpcrd
#...

# 1, 2, .. correspond to the sequence of ligand in SDF file
