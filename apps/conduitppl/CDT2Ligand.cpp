/* 
 * File:   CDT2Ligand.cpp
 * Author: zhang
 *
 * Created on March 24, 2014, 11:16 AM
 */

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <boost/scoped_ptr.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

#include "Parser/Sdf.h"
#include "Parser/Pdb.h"
#include "MM/Amber.h"
#include "Parser/SanderOutput.h" 
#include "Structure/Sstrm.hpp"
#include "Structure/Constants.h"
#include "Structure/Molecule.h"
#include "Parser/Mol2.h"
#include "Common/File.hpp"
#include "Common/Utils.h"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "Common/Command.hpp"
#include "XML/XMLHeader.hpp"

#include "CDT2Ligand.h"
#include "CDT2LigandPO.h"
#include "InitEnv.h"

namespace mpi = boost::mpi;
using namespace LBIND;
using namespace conduit;

/*!
 * \breif preReceptors calculation receptor grid dimension from CSA sitemap output
 * \param argc
 * \param argv argv[1] takes the input file name
 * \return success 
 * \defgroup preReceptors_Commands preReceptors Commands
 *
 * 
 * Usage on HPC slurm
 * 
 * \verbatim
 
    export AMBERHOME=/usr/gapps/medchem/amber/amber12
    export PATH=$AMBERHOME/bin/:$PATH
    export LIGDIR=`pwd`

    srun -N4 -n48 -ppdebug /g/g92/zhang30/medchem/NetBeansProjects/MedCM/apps/mmpbsa/preLignds  <input-file.sdf>

    <input-file.sdf>: multi-structure SDF file contains all ligands information. 
 *                  Each ligand must have "TOTAL_CHARGE" field value for ligand total charge.  

    Requires: define 
   \endverbatim
 */


void toConduit(JobOutData& jobOut, std::string& ligCdtFile){

    try {

        Node n;

        n["lig/"+jobOut.ligID + "/status"]=jobOut.error;

        std::string ligIDMeta ="lig/"+jobOut.ligID + "/meta";
        n[ligIDMeta] = jobOut.ligID;

        n[ligIDMeta + "/name"] = jobOut.ligName;
        n[ligIDMeta + "/LigPath"] = jobOut.ligPath;
        n[ligIDMeta + "/GBEN"] = jobOut.gbEn;
        n[ligIDMeta + "/Mesg"] = jobOut.message;
        std::string ligIDFile ="lig/"+jobOut.ligID+ "/file/";

        std::vector<std::string> filenames={"LIG.prmtop", "LIG.lib", "LIG.inpcrd", "LIG_min.pdbqt",
                                  "LIG_min.rst", "LIG_minGB.out", "ligand.frcmod"};

        for(std::string& name : filenames)
        {
            std::string filename=jobOut.ligPath+"/"+name;
            std::ifstream infile(filename);
            if(infile.good())
            {
                std::string buffer((std::istreambuf_iterator<char>(infile)),
                                   std::istreambuf_iterator<char>());
                infile.close();
                n[ligIDFile+name] = buffer;
            }
            else
            {
                std::cout << "File - " << filename << " is not there." << std::endl;
            }
        }

        relay::io::hdf5_append(n, ligCdtFile);

    }catch(conduit::Error &error){
        jobOut.message= error.message();
    }

}

bool isRun(JobInputData& jobInput){

    Node n;
    relay::io::hdf5_read(jobInput.ligCdtFile, n);
    std::string path="lig/"+jobInput.dirBuffer;
    if(n.has_path(path))
    {
        std::cout << "ligand - " << jobInput.dirBuffer << " has already completed!" << std::endl;
        return true;
    }
    return false;
}

void rmLigDir(JobOutData& jobOut)
{
    std::string cmd="rm -rf " + jobOut.ligPath;
    std::string errMesg="Remove fails for "+jobOut.ligPath;
    command(cmd, errMesg);
}

void preLigands(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir) {

    try{
        jobOut.ligID=jobInput.dirBuffer;
        jobOut.message="Finished!";

        std::string subDir=workDir+"/scratch/lig/"+jobOut.ligID;
        jobOut.ligPath=subDir;

        jobOut.gbEn=0.0;        
        std::string sdfPath=subDir+"/ligand.sdf";

        std::string cmd="mkdir -p "+subDir;
        std::string errMesg="mkdir ligDir fails";
        command(cmd, errMesg);
        
        std::ofstream outFile;
        try {
            outFile.open(sdfPath.c_str());
        }
        catch(...){
            std::cout << "preLigands >> Cannot open file" << sdfPath << std::endl;
        }

        outFile <<jobInput.sdfBuffer;
        outFile.close();     

        if(!fileExist(sdfPath)){
            std::string message=sdfPath+" does not exist.";
            throw LBindException(message);
        }

        chdir(subDir.c_str());

        std::string sdfFile="ligand.sdf";
        std::string pdb1File="ligand.pdb";

        cmd="obabel -isdf " + sdfFile + " -opdb -O " +pdb1File +" >> log";
        //std::cout << cmd << std::endl;
        errMesg="obabel converting SDF to PDB fails";
        command(cmd, errMesg);

        std::string pdbFile="ligrn.pdb";
        std::string tmpFile="ligstrp.pdb";

        boost::scoped_ptr<Pdb> pPdb(new Pdb());   

        //! Rename the atom name.
        pPdb->renameAtom(pdb1File, pdbFile);

        pPdb->strip(pdbFile, tmpFile);

        //! Get ligand charge from SDF file.
        std::string keyword="TOTAL_CHARGE";

        boost::scoped_ptr<Sdf> pSdf(new Sdf());
        std::string info=pSdf->getInfo(sdfFile, keyword);
        if(jobInput.cmpName=="NoName"){
            jobOut.ligName=pSdf->getTitle(sdfFile);
        }else{
            jobOut.ligName=pSdf->getInfo(sdfFile, jobInput.cmpName);
        }

        std::cout << "Charge:" << info << std::endl;
        int charge=Sstrm<int, std::string>(info);
        std::string chargeStr=Sstrm<std::string,int>(charge);

        //! Start antechamber calculation
        std::string output="ligand.mol2";
        std::string options=" -c bcc -nc "+ chargeStr;

        boost::scoped_ptr<Amber> pAmber(new Amber(jobInput.ambVersion));
        pAmber->antechamber(tmpFile, output, options);

        {
            if(!fileExist(output)){
                std::string message="ligand.mol2 does not exist.";
                throw LBindException(message);        
            }

            if(fileEmpty(output)){
                std::string message="ligand.mol2 is empty.";
                throw LBindException(message);              
            }
        }        
        
        pAmber->parmchk(output);

        //! leap to obtain forcefield for ligand
        std::string ligName="LIG";
        std::string tleapFile="leap.in";

        pAmber->tleapInput(output,ligName,tleapFile);
        pAmber->tleap(tleapFile); 

        std::string checkFName="LIG.prmtop";
        {
            if(!fileExist(checkFName)){
                std::string message="LIG.prmtop does not exist.";
                throw LBindException(message);        
            }

            if(fileEmpty(checkFName)){
                std::string message="LIG.prmtop is empty.";
                throw LBindException(message);              
            }
        }

        //! GB energy minimization
        std::string minFName="LIG_minGB.in";
        {
            std::ofstream minFile;
            try {
                minFile.open(minFName.c_str());
            }
            catch(...){
                std::string mesg="mmpbsa::receptor()\n\t Cannot open min file: "+minFName;
                throw LBindException(mesg);
            }   

            minFile << "title..\n" 
                    << "&cntrl\n" 
                    << "  imin   = 1,\n" 
                    << "  ntmin   = 3,\n" 
                    << "  maxcyc = 2000,\n" 
                    << "  ncyc   = 1000,\n" 
                    << "  ntpr   = 200,\n" 
                    << "  ntb    = 0,\n" 
                    << "  igb    = 5,\n" 
                    << "  gbsa   = 1,\n" 
                    << "  cut    = 15,\n"       
                    << " /\n" << std::endl;

            minFile.close();    
        }          

        if(jobInput.ambVersion==13){
            cmd="sander13  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
        }else{
            cmd="sander  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
        }
        //std::cout <<cmd <<std::endl;
        errMesg="sander ligand minimization fails";
        command(cmd, errMesg);
        boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
        std::string sanderOut="LIG_minGB.out";
        double ligGBen=0;
        bool success=pSanderOutput->getEnergy(sanderOut,ligGBen);
        jobOut.gbEn=ligGBen;

        if(!success){
            std::string message="Ligand GB minimization fails.";
            throw LBindException(message);          
        }

        //! Use ambpdb generated PDB file for PDBQT.
        if(jobInput.ambVersion==16){
            cmd="ambpdb -p LIG.prmtop -c LIG_min.rst > LIG_minTmp.pdb "; 
        }else{
            cmd="ambpdb -p LIG.prmtop < LIG_min.rst > LIG_minTmp.pdb ";
        } 

        //std::cout <<cmd <<std::endl;
        errMesg="ambpdb converting rst to pdb fails";
        command(cmd, errMesg);   

        checkFName="LIG_minTmp.pdb";
        if(!fileExist(checkFName)){
            std::string message="LIG_min.pdb minimization PDB file does not exist.";
            throw LBindException(message);         
        }

        pPdb->fixElement("LIG_minTmp.pdb", "LIG_min.pdb"); 



        //! Get DPBQT file for ligand from minimized structure.
        cmd="prepare_ligand4.py -l LIG_min.pdb  >> log";
        //std::cout << cmd << std::endl;   
        errMesg="prepare_ligand4.py fails";
        command(cmd, errMesg);

        checkFName="LIG_min.pdbqt";
        {
            if(!fileExist(checkFName)){
                std::string message="LIG_min.pdbqt PDBQT file does not exist.";
                throw LBindException(message);        
            }

            if(fileEmpty(checkFName)){
                std::string message="LIG_min.pdbqt is empty.";
                throw LBindException(message);              
            }
        }
        
        //! fix the Br element type
        cmd="sed -i '/Br.* LIG/{s! B ! Br!}' LIG_min.pdbqt";
        //std::cout << cmd << std::endl;
        errMesg="sed to fix Br fails";
        command(cmd, errMesg);

        chdir(workDir.c_str());

    } catch (LBindException& e){
        jobOut.message= e.what();
        jobOut.error=false;
        return;
    }

    jobOut.error=true;
    return;
}

int main(int argc, char** argv) {

    int jobFlag=1; // 1: doing job,  0: done job

    int rankTag=1;
    int jobTag=2;

    int inpTag=3;
    int outTag=4;

    mpi::environment env(argc, argv);
    mpi::communicator world; 
    
    mpi::timer runingTime;

    std::string workDir;
    std::string inputDir;
    std::string dataPath;

    if(!initConveyorlcEnv(workDir, inputDir, dataPath)){
        world.abort(1);
    }
       
    POdata podata;
    
    if (world.rank() == 0) {        
        bool success=CDT2LigandPO(argc, argv, podata);
        if(!success){
            world.abort(1);
        }        
    }

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        world.abort(1);
    }

    JobInputData jobInput;
    JobOutData jobOut;

    if (world.rank() == 0) {
        //! Open a Conduit file to track the calculation
        std::string cmd = "mkdir -p " + workDir + "/scratch";
        std::string errMesg = "mkdir scratch directory fails";
        LBIND::command(cmd, errMesg);

        //! Open a Conduit file to track the calculation
        Node n;
        std::string ligCdtFile=workDir+"/scratch/ligand.hdf5:/";
        n["date"]="Create By CDT2Ligand at "+timeStamp();
        relay::io::hdf5_append(n, ligCdtFile);
        jobInput.ligCdtFile=ligCdtFile;
        
        // Pass the ligand name option
        jobInput.cmpName=podata.cmpName;
        jobInput.ambVersion=podata.version;

        // Start to read in the SDF file
        std::string sdfFileName=inputDir+"/"+podata.sdfFile;
        std::ifstream inFile;
        try {
            inFile.open(sdfFileName.c_str());
        } catch (...) {
            std::cout << "CDT2Ligand >> Cannot open file" << podata.sdfFile << std::endl;
            world.abort(1);
        }

        const std::string delimter = "$$$$";
        std::string fileLine = "";
        std::string contents = "";

        int count = 0;
        int dirCnt= 0;

        while (inFile) {
            std::getline(inFile, fileLine);
            contents = contents + fileLine + "\n";
            if (fileLine.size() >= 4 && fileLine.compare(0, 4, delimter) == 0) {
                dirCnt++;
                jobInput.dirBuffer=std::to_string(dirCnt);
                //For restart
                if(isRun(jobInput)){
                    continue;
                }

                count++;

                if(count > world.size()-1){
                    world.recv(mpi::any_source, outTag, jobOut);

                    toConduit(jobOut, ligCdtFile);
                    rmLigDir(jobOut);
                }

                int freeProc;
                world.recv(mpi::any_source, rankTag, freeProc);
                std::cout << "At Process: " << freeProc << " working on: " << jobInput.dirBuffer << std::endl;
                world.send(freeProc, jobTag, jobFlag);

                jobInput.sdfBuffer=contents;

                world.send(freeProc, inpTag, jobInput);
                
                contents = ""; //! clean up the contents for the next structure.

            }
                             
        }

        int nJobs=count;
        int nWorkers=world.size()-1;
        int ndata=(nJobs<nWorkers)? nJobs: nWorkers;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(int i=0; i < ndata; ++i){
            world.recv(mpi::any_source, outTag, jobOut);

            toConduit(jobOut, ligCdtFile);
            rmLigDir(jobOut);
        } 

        
        for(int i=1; i < world.size(); ++i){
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;;
            world.send(freeProc, jobTag, jobFlag);
        }
        
    }else {
        while (1) {
            world.send(0, rankTag, world.rank());
            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }

            world.recv(0, inpTag, jobInput);

            preLigands(jobInput, jobOut, workDir);

            world.send(0, outTag, jobOut);
        }
    }

    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;
    
    return 0;
}


