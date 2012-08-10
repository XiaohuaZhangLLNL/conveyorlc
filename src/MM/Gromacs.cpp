/* 
 * File:   Gromacs.cpp
 * Author: zhang30
 * 
 * Created on December 9, 2011, 3:32 PM
 */

#include <fstream>
#include <iostream>

#include "Gromacs.h"
#include "BackBone/Protein.h"
#include "Common/LBindException.h"

namespace LBIND {

Gromacs::Gromacs() : pProtein(NULL) {
    GromacsPath=getenv("GromacsDir");
    GromacsEXEPath=GromacsPath+"/bin/";
    GromacsTopPath=GromacsPath+"/share/gromacs/top/";   
}

Gromacs::Gromacs(Protein* pProt) : pProtein(pProt) {   
    GromacsPath=getenv("GromacsDir");
    GromacsEXEPath=GromacsPath+"/bin/";
    GromacsTopPath=GromacsPath+"/share/gromacs/top/";   
}

Gromacs::Gromacs(const Gromacs& orig) {
}

Gromacs::~Gromacs() {
}

void Gromacs::run(){
    std::string pdbid=pProtein->getPDBID();
    this->prepare(pdbid);
    this->minimization(pdbid);
//    this->prmd(pdbid);
//    this->md(pdbid);
}

void Gromacs::prepare(std::string pdbid){    
    std::string pdbFName=pdbid+".pdb";
    std::ifstream pdbFile;
    try {
        pdbFile.open(pdbFName.c_str());
    }
    catch(...){
        std::string mesg="Gromacs::prepare()\n\t Cannot open PDB file: "+pdbFName;
        throw LBindException(mesg);
    }   
    pdbFile.close();
    
    this->pdb2gmx(pdbid);
    this->editconf(pdbid);
    this->genbox(pdbid);
    
    
}
 
void Gromacs::pdb2gmx(std::string pdbid){
    //! Process the pdb file with pdb2gmx:
    // The pdb2gmx (to view the command line options for this
    //command just type pdb2gmx -h; In fact, to get help for any command in Gromacs just use the -h
    //flag) command converts your pdb file to a gromacs file and writes the topology for you. This file is
    //derived from an NMR structure which contains hydrogen atoms. Therefore, we use the -ignh flag
    //to ignore hydrogen atoms in the pdb file. The -ff flag is used to select the forcefield (G43a1 is the
    //Gromos 96 FF, a united atom FF). The -f flag reads the pdb file. The -o flag outputs a new pdb
    //file (various file formats supported) with the name you have given it in the command line. The -p
    //flag is used for output and naming of the topology file. The topology file is very important as it
    //contains all of the forcefield parameters (based upon the forcefield that you select in the initial
    //prompt) for your model. Studies have revealed that the SPC/E water model [3] performs best in
    //water box simulations. [4] Use the spce water model as it is better for use with the long-range
    //electrostatics methods. So, we use the -water flag to specify this model.
    //
    // pdb2gmx -ignh -ff gromos43a1 -f 1OMB.pdb -o fws.pdb -p fws.top -water spce
    std::string cmd=GromacsEXEPath+"/pdb2gmx -ignh -ff gromos43a1 -f " + 
                   pdbid+".pdb -o "+ pdbid + "-fws.pdb -p "+pdbid+"-fws.top -water spce";
    system(cmd.c_str());
}

void Gromacs::editconf(std::string pdbid){
    //!  Setup the Box for your simulations
    //What you have done in this command is specify that you want to use a truncated octahedron box.
    //The -d 1.0 flag sets the dimensions of the box based upon setting the box edge approx 1.0 nm (i.e.
    //10.0 angstroms) from the molecule(s) periphery. Ideally you should set -d at no less than 1.0 nm
    //for most systems. [5]
    //[Special Note: editconf may also be used to convert gromacs files (*.gro) to pdb files (*.pdb) and
    //vice versa. For example: editconf -f file.gro -o file.pdb converts file.gro to the pdb file file.pdb]
    //You may use the files generated from the this step to begin your in vacuo simulation. For the in
    //vacuo work just move ahead to the energy minimization step followed by the molecular dynamics
    //step (No position restrained dynamics necessary for in vacuo work. Why?).  
    //
    // editconf -bt octahedron -f fws.pdb -o fws-b4sol.pdb -d 1.0    
    std::string cmd=GromacsEXEPath+"/editconf -bt octahedron -f "+ pdbid +"-fws.pdb -o "
            +pdbid+"-fws-b4sol.pdb -d 1.0";
    system(cmd.c_str());
}

void Gromacs::genbox(std::string pdbid){
    this->genbox(pdbid, "spc216.gro");
}

void Gromacs::genbox(std::string pdbid, std::string groFile="spc216.gro"){   
    // ! Solvate the Box
    //The genbox command generates the water box based upon the dimensions/box type that you had
    //specified using editconf. In the command above, we specify the spc water box. The genbox
    //program will add the correct number of water molecules needed to solvate your box of given
    //dimensions.    
    //    
    //genbox -cp fws-b4sol.pdb -cs spc216.gro -o fws-b4ion.pdb -p fws.top
    // spc216.gro is in share/gromacs/top/.
    std::string cmd=GromacsEXEPath+"/genbox -cp "+pdbid+"-fws-b4sol.pdb -cs "
            +GromacsTopPath + "/" + groFile + " -o "+pdbid+"-fws-b4ion.pdb -p "+pdbid+"-fws.top";
    std::cout << cmd << std::endl;
    system(cmd.c_str());
}

void Gromacs::minimization(std::string pdbid){
    //Use the em.mdp file. Gromacs uses special *.mdp files to setup the parameters for every type of
    //calculation that it performs. Look into the contents of this file. It specifies a steepest descents
    //minimization to remove bad van der Waals contacts. Edit the file and change nsteps to 400. If the
    //minimization fails to converge, re-submit with nsteps = 500. (The minimization should converge in
    //less than 400 steps; however, different platforms respond differently.) To re-submit the job, you
    //will need to re-run grompp.    
    
    this->minimizationInput(pdbid);
    this->grompp(pdbid, 1);
    this->genion(pdbid);
    this->grompp(pdbid, 2);

    //Run the energy minimization and watch its progress.
    //mdrun -v -deffnm em
    std::string cmd=GromacsEXEPath+"/mdrun -v -deffnm "+pdbid+"-em";
    system(cmd.c_str());
}

void Gromacs::grompp(std::string pdbid, int nTimes){
    // grompp is the pre-processor program (the gromacs preprocessor
    //“grompp” Get it! Sigh!). grompp will setup your run for input into mdrun.
    std::string cmd;
    switch(nTimes){    
        //The -f flag in grompp is used to input the parameter file (*.mdp). The -c flag is used to input the
        //coordinate file (the pdb file, *.pdb); -p inputs the topology and -o outputs the input file (*.tpr)
        //needed for mdrun.

        //grompp -f em.mdp -c fws-b4ion.pdb -p fws.top -o ion.tpr -maxwarn 5
        case 1:
            cmd=GromacsEXEPath+"/grompp -f "+pdbid+"-em.mdp -c "
                +pdbid+"-fws-b4ion.pdb -p "+pdbid+"-fws.top -o "+pdbid+"-ion.tpr -maxwarn 5";
            break;
       
        //If you added the chloride ions, you will need to run the grompp step again. First
        //remove the old fws_em.tpr file, then run the next grompp command below. We added chloride ions
        //in our model to neutralize the overall net charge of the model.
        case 2:
            cmd="rm -f "+pdbid+"-fws_em.tpr";
            system(cmd.c_str());     
            //grompp -f em.mdp -c fws-b4em.pdb -p fws.top -o em.tpr -maxwarn 5    
            cmd=GromacsEXEPath+"/grompp -f "+pdbid+"-em.mdp -c "
                    +pdbid+"-fws-b4em.pdb -p "+pdbid+"-fws.top -o "+pdbid+"-em.tpr -maxwarn 5";
            break;
        case 3:
            //grompp -f pr.mdp -c em.gro -p fws.top -o pr.tpr -maxwarn 5
            cmd=GromacsEXEPath+"/grompp -f "+pdbid+"-pr.mdp -c "
                    +pdbid+"-em.gro -p "+pdbid+"-fws.top -o "+pdbid+"-pr.tpr -maxwarn 5";
            break;
        case 4:
            //grompp -f md.mdp -c pr.gro -p fws.top -o md.tpr -maxwarn 5
            cmd=GromacsEXEPath+"/grompp -f "+pdbid+"-md.mdp -c "
                    +pdbid+"-pr.gro -p "+pdbid+"-fws.top -o "+pdbid+"-md.tpr -maxwarn 5";
            break;
        default:
            break;
    }
    system(cmd.c_str());
}

void Gromacs::genion(std::string pdbid){
    //Using genion and the tpr file to add ions. You may use the tpr file generated here to add
    //counterions to your model to neutralize any net charge. In order for the Ewald equation we are
    //using to describe long range electrostatics in our model to be valid, the net system charge must be
    //neutral. Our model has a net charge of +2.00. Therefore, we want to add two chloride ions. Copy
    //the fws_em.tpr file that you used for your explicit solvated model to your “ionwet” subdirectory. In
    //addition, copy your fws.top and posre.itp files from your explicit solvation model to your ionwet
    //subdirectory. Use the genion command to add the chloride ions.  
    //
    std::string cmd=GromacsEXEPath+"/genion -s "+pdbid+"-ion.tpr -o "+pdbid+
            "-fws-b4em.pdb -nname CL -nn 2 -p "+pdbid+"-fws.top -g ion.log  < 13 ";
    //
    //Where -nname is the negative ion name (CL for the Gromos G43a1 force field; see the ions.itp file
    //for specifics wrt force field), -nn is the number of negative ions to add. Use -pname to add
    //positively charged ions and -np to specify the number of positively charged ions to add. The -g
    //flag gives a name to the output log for genion. An even easier method which creates a neutral
    //system using a specified concentration of NaCl uses the -neutral and -conc flags as in the
    //following …
    // We will use the 0.15 M NaCl model via the command above. The -norandom flag places ions
    //based on electrostatic potential as opposed to random placement (default). However, we will use
    //the default (random placement). When you process this command, you will be prompted to provide
    //a continuous group of solvent molecules, which should be Group 13 (SOL). Type 13 then <enter>.
    //You will notice that a number of solvent molecules have been replaced by Na and Cl ions. You
    //should also notice that the fws.top file has been updated to include the NA and CL ions in your
    //topology accounting for the replaced water molecules.
    //Caution: The molecules listed here must appear in the exact same order as in your coordinate file!
    
    //genion -s ion.tpr -o fws-b4em.pdb -neutral -conc 0.15 -p fws.top -g ion.log    
//    std::string cmd=GromacsEXEPath+"/genion -s "+pdbid+"-ion.tpr -o "+pdbid
//            +"-fws-b4em.pdb -neutral -conc 0.15 -p "+pdbid+"-fws.top -g ion.log";
    system(cmd.c_str());
}

void Gromacs::minimizationInput(std::string pdbid){
    //Use the em.mdp file. Gromacs uses special *.mdp files to setup the parameters for every type of
    //calculation that it performs. Look into the contents of this file. It specifies a steepest descents
    //minimization to remove bad van der Waals contacts. Edit the file and change nsteps to 400. If the
    //minimization fails to converge, re-submit with nsteps = 500. (The minimization should converge in
    //less than 400 steps; however, different platforms respond differently.) To re-submit the job, you
    //will need to re-run grompp.    
    std::ofstream mInput;
    std::string mInputName=pdbid+"-em.mdp";
    try {
        mInput.open(mInputName.c_str());
    }
    catch(...){
        std::string mesg="Gromacs::minimizationInput(std::string pdbid)\n\t "
                "Cannot open minimization input file: "+mInputName;
        throw LBindException(mesg);
    }
    
    //Important aspects of the em.mdp file:
    //title - The title can be any given text description (limit 64 characters; keep it short and simple!)
    //Deprecated and no longer used in Gromacs 4.0.
    //cpp - location of the pre-processor. Deprecated: Gromacs 4.0 uses the default cpp for your system.
    //define - defines to pass to the pre-processor. -DFLEXIBLE will tell grompp to include the flexible
    //SPC water model instead of the rigid SPC into your topology. This allows steepest descents to
    //minimize further.
    //constraints - sets any constraints used in the model.
    //integrator - steep tells grompp that this run is a steepest descents minimization. Use cg for
    //conjugate gradient.
    //dt - not necessary for minimization. Only needed for dynamics integrators (like md).
    //nsteps - In minimization runs, this is just the maximum number of iterations.
    //nstlist - frequency to update the neighbor list. nstlist = 10 (updates the list every 10 steps).
    //rlist - cut-off distance for short-range neighbor list.
    //coulombtype - tells gromacs how to model electrostatics. PME is particle mesh ewald method
    //(please see the Gromacs user manual for more information).
    //rcoulomb - distance for the coulomb cut-off
    //vdwtype - tells Gromacs how to treat van der Waals interactions (cut-off, Shift, etc.)
    //rvdw - distance for the LJ or Buckingham potential cut-off
    //fourierspacing - Used to automate setup of the grid dimensions (fourier_nx , …) for pme.
    //EM Stuff
    //emtol - the minimization converges when the max force is smaller than this value (in units of kJ
    //mol-1 nm-1)
    //emstep - initial step size (in nm).    
    
    mInput << "define = -DFLEXIBLE \n";
    mInput << "constraints = none \n";
    mInput << "integrator = steep \n";
    mInput << "dt = 0.002 ; ps ! \n";
    mInput << "nsteps = 400 \n";
    mInput << "nstlist = 10 \n";
    mInput << "ns_type = grid \n";
    mInput << "rlist = 1.0 \n";
    mInput << "coulombtype = PME \n";
    mInput << "rcoulomb = 1.0 \n";
    mInput << "vdwtype = cut-off \n";
    mInput << "rvdw = 1.4 \n";
    mInput << "optimize_fft = yes \n";
    mInput << "; \n";
    mInput << "; Energy minimizing stuff \n";
    mInput << "; \n";
    mInput << "emtol = 1000. \n";  
    
    mInput.close();
}

void Gromacs::prmd(std::string pdbid){
    this->grompp(pdbid,3);
    //mdrun -deffnm pr &
    std::string cmd=GromacsEXEPath+"/mdrun -v -deffnm "+pdbid+"-pr";
    system(cmd.c_str());    
}

void Gromacs::prmdInput(std::string pdbid){
    std::ofstream mInput;
    std::string mInputName=pdbid+"-pr.mdp";
    try {
        mInput.open(mInputName.c_str());
    }
    catch(...){
        std::string mesg="Gromacs::prmdInput(std::string pdbid)\n\t "
                "Cannot open position restrainted MD input file: "+mInputName;
        throw LBindException(mesg);
    }
    //The -DPOSRES in the define statement tells Gromacs to perform position restrained dynamics.
    //The constraints statement is as previously discussed. all-bonds sets the LINCS constraint for all
    //bonds. [8]
    //The integrator tells Gromacs what type of dynamics algorithm to use (another option is “sd” for
    //stochastic dynamics).
    //dt is the time step (we have selected 2 fs; however, this must be entered in units of ps!).
    //nsteps is the number of steps to run (just multiply nsteps X dt to compute the length of the
    //simulation).
    //nstxout tells gromacs how often to collect a “snapshot” for the trajectory. (e.g. nstxout = 250 with
    //dt = 0.002 would collect a snapshot every 0.5 ps)
    //coulombtype selects the manner in which Gromacs computes electrostatic interactions between
    //atoms. (PME is particle mesh ewald; there are other options like cut-off).
    //rcoulomb and rvdw are the cutoffs (in nm; 1.0 nm = 10.0 angstroms) for the electrostatic and van
    //der Waals terms.
    //The temperature coupling section is very important and must be filled out correctly.
    //Tcoupl = v-rescale [9, 10] (type of temperature coupling where velocity is rescaled using a
    //stochastic term.)
    //tau_t = Time constant for temperature coupling (units = ps). You must list one per tc_grp in the
    //order in which tc_grps appear.
    //tc_grps = groups to couple to the thermostat (every atom or residue in your model must be
    //represented here by appropriate index groups).
    //ref_t = reference temperature for coupling (i.e. the temperature of your MD simulation in degrees
    //K). You must list one per tc_grp in the order in which tc_grps appear.
    //When you alter the temperature, don’t forget to change the gen_temp variable for velocity
    //generation.
    //pcoupl - No pressure coupling is selected for this run.
    //pcoupltype - isotropic means that the “box” will expand or contract evenly in all directions (x, y,
    //and z) in order to maintain the proper pressure. Note: Use semiisotropic for membrane simulations.
    //tau_p - time constant for coupling (in ps).
    //compressibility - this is the compressibility of the solvent used in the simulation in bar-1 (the
    //setting above is for water at 300 K and 1 atm).
    //ref_p - the reference pressure for the coupling (in bar) (1 atm ~ 0.983 bar).
    //gen_seed = -1 Use random number seed based on process ID #. More important for Langevin
    //dynamics.[11]    
    
    mInput << "define = -DPOSRES \n";
    mInput << "constraints = all-bonds \n";
    mInput << "integrator = md \n";
    mInput << "dt = 0.002 ; ps ! \n";
    mInput << "nsteps = 50000 ; total 100.0 ps. \n";
    mInput << "nstcomm = 10 \n";
    mInput << "nstxout = 500 ; collect data every 1.0 ps \n";
    mInput << "nstxtcout = 500 \n";
    mInput << "nstvout = 5000 \n";
    mInput << "nstfout = 0 \n";
    mInput << "nstlog = 10 \n";
    mInput << "nstenergy = 50 \n";
    mInput << "nstlist = 10 \n";
    mInput << "ns_type = grid \n";
    mInput << "rlist = 1.0 \n";
    mInput << "coulombtype = PME \n";
    mInput << "rcoulomb = 1.0 \n";
    mInput << "vdwtype = cut-off \n";
    mInput << "rvdw = 1.4 \n";
    mInput << "pme_order = 4 \n";
    mInput << "ewald_rtol = 1e-5 \n";
    mInput << "optimize_fft = yes \n";
    mInput << "DispCorr = no \n";
    mInput << "; Berendsen temperature coupling is on \n";
    mInput << "Tcoupl = v-rescale \n";
    mInput << "tau_t = 0.1 0.1 \n";
    mInput << "tc-grps = protein non-protein \n";
    mInput << "ref_t = 300 300 \n";
    mInput << "; Pressure coupling is off \n";
    mInput << "Pcoupl = no \n";
    mInput << "Pcoupltype = isotropic \n";
    mInput << "tau_p = 1.0 \n";
    mInput << "compressibility = 4.5e-5  \n";   
    mInput << "ref_p = 1.0 \n";
    mInput << "; Generate velocites is on at 300 K. \n";
    mInput << "gen_vel = yes \n";
    mInput << "gen_temp = 300.0 \n";
    mInput << "gen_seed = -1  \n";   
    
    mInput.close();
}

void Gromacs::md(std::string pdbid){
    this->grompp(pdbid,4);
    //mdrun -deffnm pr &
    std::string cmd=GromacsEXEPath+"/mdrun -v -deffnm "+pdbid+"-md";
    system(cmd.c_str());       
}

void Gromacs::mdInput(std::string pdbid){
    std::ofstream mInput;
    std::string mInputName=pdbid+"-md.mdp";
    try {
        mInput.open(mInputName.c_str());
    }
    catch(...){
        std::string mesg="Gromacs::mdInput(std::string pdbid)\n\t "
                "Cannot open MD input file: "+mInputName;
        throw LBindException(mesg);
    } 
    mInput << "constraints = all-bonds \n";
    mInput << "integrator = md \n";
    mInput << "dt = 0.002 ; ps ! \n";
    mInput << "nsteps = 500000 ; total 1000 ps. \n";
    mInput << "nstcomm = 10 \n";
    mInput << "nstxout = 500 ; collect data every 1 ps \n";
    mInput << "nstxtcout = 0 \n";
    mInput << "nstenergy = 100 \n";
    mInput << "nstvout = 0 \n";
    mInput << "nstfout = 0 \n";
    mInput << "nstlist = 10 \n";
    mInput << "ns_type = grid \n";
    mInput << "rlist = 1.0 \n";
    mInput << "coulombtype = PME \n";
    mInput << "rcoulomb = 1.0 \n";
    mInput << "vdwtype = cut-off \n";
    mInput << "rvdw = 1.4 \n";
    mInput << "pme_order = 4 \n";
    mInput << "ewald_rtol = 1e-5 \n";
    mInput << "optimize_fft = yes \n";
    mInput << "DispCorr = no \n";
    mInput << "; Berendsen temperature coupling is on \n";
    mInput << "Tcoupl = v-rescale \n";
    mInput << "tau_t = 0.1 0.1 \n";
    mInput << "tc-grps = protein non-protein \n";
    mInput << "ref_t = 300 300 \n";
    mInput << "; Pressure coupling is on  \n";   
    mInput << "Pcoupl = parrinello-rahman \n";
    mInput << "Pcoupltype = isotropic \n";
    mInput << "tau_p = 1.0 \n";
    mInput << "compressibility = 4.5e-5 \n";
    mInput << "ref_p = 1.0 \n";
    mInput << "; Generate velocites is on at 300 K. \n";
    mInput << "gen_vel = yes \n";
    mInput << "gen_temp = 300.0 \n";
    mInput << "gen_seed = -1 \n";
        
    mInput.close();    
}

}//namespace LBIND 
