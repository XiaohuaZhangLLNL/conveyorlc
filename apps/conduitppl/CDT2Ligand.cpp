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
#include <unordered_set>

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
#include "Common/Chomp.hpp"
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

void getLigNames(std::string& ligNameFile, std::unordered_set<std::string>& ligNameSet){

    std::ifstream inFile;
    inFile.open(ligNameFile.c_str());

    const std::string comment="#";
    std::string fileLine;
    while(inFile){
        std::getline(inFile, fileLine);
        if(fileLine.compare(0, 1, comment)==0) continue;
        chomp(fileLine);
        ligNameSet.insert(fileLine);
    }
}

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

        std::vector<std::string> filenames={"LIG.prmtop", "LIG.lib", "LIG.inpcrd", "LIG_min.pdbqt", "LIG_min.pdb", "ligand.mol2",
                                  "LIG_min.rst", "LIG_minGB.out", "ligand.frcmod"};
        std::cout << "CONDUIT: " << jobOut.ligPath << std::endl;

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

/*
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
 */

void getCalcHDF5(std::string& fileName, std::vector<bool>& calcList, std::vector<int>& skipList){
    Node n;

    hid_t lig_hid=relay::io::hdf5_open_file_for_read(fileName);

    std::vector<std::string> lig_names;
    try {
        relay::io::hdf5_group_list_child_names(lig_hid, "/lig/", lig_names);
    }catch (...){
        std::cout << "Warning some error in ligand.hdf5" << std::endl;
    }

    std::cout << "Previous complete ligands " <<  lig_names.size() << std::endl;

    for(int i=0; i<lig_names.size(); i++){
        int ligID=std::atoi(lig_names[i].c_str());
        calcList[ligID]=true;
    }

    std::cout << "Skip ligands " <<  skipList.size() << std::endl;
    for(int i=0; i<skipList.size(); i++){
        int ligID=skipList[i];
        calcList[ligID]=true;
    }

    /*
    NodeIterator itrLig = n["lig"].children();

    while(itrLig.has_next())
    {
        Node &nLig=itrLig.next();
        std::string ligStr=nLig.name();
        int ligID=std::atoi(ligStr.c_str());
        calcList[ligID]=true;
    }

     */

    relay::io::hdf5_close_file(lig_hid);
}

int getNumLigand(POdata& podata){
    std::ifstream inFile;
    inFile.open(podata.sdfFile);

    int count =0;
    std::string fileLine;
    while (inFile) {
        std::getline(inFile, fileLine);

        if (fileLine[0]=='$') count++;

    }
    std::cout << "There are total " << count << " ligands in SDF file" << std::endl;
    return count;
}

void getSkipList(POdata& podata, std::vector<int>& skipList){
    if(fileExist(podata.skipFile)){
        std::ifstream inFile;
        inFile.open(podata.skipFile);
        std::string fileLine;
        while (inFile) {
            std::getline(inFile, fileLine);
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens, "\n");
            if(tokens.size()==1) {
                skipList.push_back(std::stoi(tokens[0].c_str()));
            }
        }
    }
}

void getCalcList(std::vector<bool>& calcList, std::string& fileName, POdata& podata){
    int numLigand=getNumLigand(podata);
    calcList.resize(numLigand+1, false);
    std::vector<int> skipList;
    getSkipList(podata, skipList);
    getCalcHDF5(fileName, calcList, skipList);
}

void rmLigDir(JobOutData& jobOut)
{
    std::cout << "rmLigDir: " << jobOut.ligPath << std::endl;
    std::string cmd="rm -rf " + jobOut.ligPath;
    std::string errMesg="Remove fails for "+jobOut.ligPath;
    command(cmd, errMesg);
}

void backupHDF5File(std::string& hdf5file)
{
    std::string cmd="cp " + hdf5file + " " + hdf5file+"-backup";
    std::string errMesg="Backup fails for "+hdf5file;
    command(cmd, errMesg);
}

void preLigands(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir, std::string& targetDir, bool useLocalDir,
        std::unordered_set<std::string>& ligNameSet, POdata& podata) {

    try{
        jobOut.ligID=jobInput.dirBuffer;
        jobOut.message="Finished!";

        std::string subDir=workDir+"/scratch/lig/"+jobOut.ligID;
        std::string tgtDir=targetDir+"/scratch/lig/"+jobOut.ligID;
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

        boost::scoped_ptr<Sdf> pSdf(new Sdf());
        if(jobInput.cmpName=="NoName"){
            jobOut.ligName=pSdf->getTitle(sdfFile);
        }else{
            jobOut.ligName=pSdf->getInfo(sdfFile, jobInput.cmpName);
        }

        if(ligNameSet.size()>0) {
            auto pos = ligNameSet.find(jobOut.ligName);
            if (pos == ligNameSet.end()) {
                jobOut.message = "Not in ligName.list";
                jobOut.error = false;
                return;
            }
        }

        if(jobInput.minimizeFlg) {
            //! Get ligand charge from SDF file.
            std::string keyword="TOTAL_CHARGE";
            std::string keyword2="FCharge";

            std::string info=pSdf->getInfo(sdfFile, keyword);
            if(info==""){
                info=pSdf->getInfo(sdfFile, keyword2);
            }

            if(info==""){
                std::string message="Total charge is not in SDF file";
                throw LBindException(message);
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

            if (jobInput.ambVersion == 16) {
                pAmber->parmchk2(output);
            }else {
                pAmber->parmchk(output); // parmchk is deprecated from AMBER16
            }

            //! leap to obtain forcefield for ligand
            std::string ligName="LIG";
            std::string tleapFile="leap.in";

            pAmber->tleapInput(output,ligName,tleapFile, subDir);
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

            if(jobInput.score_only){
                cmd = "ambpdb -p LIG.prmtop < LIG.inpcrd > LIG_minTmp.pdb ";
            }else {
                //! GB energy minimization
                std::string minFName = "LIG_minGB.in";
                {
                    std::ofstream minFile;
                    try {
                        minFile.open(minFName.c_str());
                    }
                    catch (...) {
                        std::string mesg = "mmpbsa::receptor()\n\t Cannot open min file: " + minFName;
                        throw LBindException(mesg);
                    }

                    minFile << "title..\n"
                            << "&cntrl\n"
                            << "  imin   = 1,\n"
                            << "  ntmin   = 1,\n"
                            << "  maxcyc = 2000,\n"
                            << "  ncyc   = 1000,\n"
                            << "  ntpr   = 200,\n"
                            << "  ntb    = 0,\n"
                            << "  igb    = 5,\n"
                            << "  gbsa   = 1,\n"
                            << "  intdiel= " << jobInput.intDiel << ",\n"
                            << "  cut    = 50,\n"
                            << " /\n" << std::endl;

                    minFile.close();
                }

                if (jobInput.ambVersion == 13) {
                    cmd = "sander13  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
                } else {
                    cmd = "sander  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
                }
                //std::cout <<cmd <<std::endl;
                errMesg = "sander ligand minimization fails";
                command(cmd, errMesg);
                boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
                std::string sanderOut = "LIG_minGB.out";
                double ligGBen = 0;
                bool success = pSanderOutput->getEnergy(sanderOut, ligGBen);
                jobOut.gbEn = ligGBen;

                if (!success) {
                    std::string message = "Ligand GB minimization fails.";
                    throw LBindException(message);
                }

                //! Use ambpdb generated PDB file for PDBQT.
                if (jobInput.ambVersion == 16) {
                    cmd = "ambpdb -p LIG.prmtop -c LIG_min.rst > LIG_minTmp.pdb ";
                } else {
                    cmd = "ambpdb -p LIG.prmtop < LIG_min.rst > LIG_minTmp.pdb ";
                }
            }

        }else{

            //cmd="ambpdb -p LIG.prmtop < LIG.inpcrd > LIG_minTmp.pdb";
            cmd="ln -s ligstrp.pdb LIG_minTmp.pdb";

        }

        //std::cout <<cmd <<std::endl;
        errMesg = "ambpdb converting rst/inp to pdb fails";
        command(cmd, errMesg);

        std::string checkFName = "LIG_minTmp.pdb";
        if (!fileExist(checkFName)) {
            std::string message = "LIG_min.pdb minimization PDB file does not exist.";
            throw LBindException(message);
        }

        pPdb->fixElement("LIG_minTmp.pdb", "LIG_min.pdb");

        //! Get DPBQT file for ligand from minimized structure.
        //cmd="prepare_ligand4.py -l  LIG_min.pdb >> log";
        cmd="obabel -ipdb LIG_min.pdb -omol2 > LIG_min.mol2 && obabel -imol2 LIG_min.mol2  -xn -opdbqt > LIG_min.pdbqt";
        //std::cout << cmd << std::endl;   
        //errMesg="prepare_ligand4.py fails";
        errMesg="obabel LIG_min.pdbqt fails";
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
        if(useLocalDir) {
            cmd = "mkdir -p " + tgtDir;
            errMesg = "mkdir ligand target directory fails";
            command(cmd, errMesg);

            cmd = "cp LIG.prmtop LIG.lib LIG.inpcrd LIG_min.pdbqt LIG_min.pdb ligand.mol2 LIG_min.rst LIG_minGB.out ligand.frcmod " + tgtDir;
            errMesg = "copying saved ligand files fails";
            command(cmd, errMesg);
            std::cout << "BEFORE: " << jobOut.ligPath << std::endl;
            jobOut.ligPath=tgtDir;
	    std::cout << "AFTER : " << jobOut.ligPath << std::endl;
        }
        chdir(workDir.c_str());

        if(useLocalDir) {
            std::string cmd="rm -rf " + subDir;
            std::string errMesg="Remove fails for "+subDir;
            command(cmd, errMesg);
        }

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
    std::string localDir;

    if(!initConveyorlcEnv(workDir, localDir, inputDir, dataPath)){
        world.abort(1);
    }

    bool useLocalDir=(localDir!=workDir);
       
    POdata podata;

    bool success=CDT2LigandPO(argc, argv, podata);
    if(!success){
        world.abort(1);
    }

    std::unordered_set<std::string> ligNameSet;
    if(podata.useLigName){
        getLigNames(podata.ligNameFile, ligNameSet);
    }

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        world.abort(1);
    }

    JobInputData jobInput;
    JobOutData jobOut;

    if (world.rank() == 0) {
        // Check if these is ligand.hdf5
        bool isNew=true;
        std::string ligOutfile=workDir+"/scratch/ligand.hdf5";
        std::vector<bool> calcList;
        if(fileExist(ligOutfile)){
            isNew=false;
            getCalcList(calcList, ligOutfile, podata);
        }

        //! Open a Conduit file to track the calculation
        std::string cmd = "mkdir -p " + workDir + "/scratch";
        std::string errMesg = "mkdir scratch directory fails";
        LBIND::command(cmd, errMesg);

        //! Open a Conduit file to track the calculation
        Node n;
        std::string ligCdtFile=workDir+"/scratch/ligand.hdf5:/";
        std::string ligHDF5Backup=workDir+"/scratch/ligand.hdf5";
        n["date"]="Create By CDT2Ligand at "+timeStamp();
        relay::io::hdf5_append(n, ligCdtFile);

        if(podata.saveSDF=="on") {
            hid_t lig_hid = relay::io::hdf5_open_file_for_read_write(workDir + "/scratch/ligand.hdf5");
            //std::string ligSdfFile = ligCdtFile + "sdf/";
            bool hasSaveSDF=conduit::relay::io::hdf5_has_path(lig_hid, "sdf");
            relay::io::hdf5_close_file(lig_hid);
            if (!hasSaveSDF) {
                std::ifstream infile(podata.sdfFile);
                if (infile.good()) {
                    std::string buffer((std::istreambuf_iterator<char>(infile)),
                                       std::istreambuf_iterator<char>());
                    infile.close();
                    Node nSDF;
                    nSDF["sdf/path"] = podata.sdfFile;
                    nSDF["sdf/file"] = buffer;
                    relay::io::hdf5_append(nSDF, ligCdtFile);
                } else {
                    std::cout << "SDF input file - " << podata.sdfFile << " is not there." << std::endl;
                }
            }
        }

        if(podata.minimizeFlg=="on"){
            jobInput.minimizeFlg=true;
        }else{
            jobInput.minimizeFlg=false;
        }

        bool backupHDF5=false;
        if(podata.backup=="on"){
            backupHDF5=true;
        }

        jobInput.ligCdtFile=ligCdtFile;
        
        // Pass the ligand name option
        jobInput.cmpName=podata.cmpName;
        jobInput.ambVersion=podata.version;
        jobInput.score_only=podata.score_only;
        jobInput.intDiel=podata.intDiel;

        // Start to read in the SDF file
        //std::string sdfFileName=inputDir+"/"+podata.sdfFile;
        std::ifstream inFile;
        try {
            inFile.open(podata.sdfFile.c_str());
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
                //For restart
                if(!isNew) {
                    if (calcList[dirCnt]) {
                        contents = ""; //! clean up the contents for the next structure.
                        continue;
                    }
                }
                jobInput.dirBuffer=std::to_string(dirCnt);

                count++;

                if(count > world.size()-1){
                    world.recv(mpi::any_source, outTag, jobOut);

                    toConduit(jobOut, ligCdtFile);
                    if(backupHDF5 && count%1000==0){
                        backupHDF5File(ligHDF5Backup);
                    }
                    if(jobOut.error && !podata.keep) {
                        rmLigDir(jobOut);
                    }
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
            if(backupHDF5 && i%1000==0){
                backupHDF5File(ligHDF5Backup);
            }
            if(jobOut.error  && !podata.keep) {
            	rmLigDir(jobOut);
            }
        } 

        
        for(int i=1; i < world.size(); ++i){
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;;
            world.send(freeProc, jobTag, jobFlag);
        }
        
    }else {
        //if(useLocalDir){
        //    std::string cmd = "rm -rf " + localDir+"/scratch";
        //    std::string errMesg = "Clean up local disk fails before calculation";
        //    LBIND::command(cmd, errMesg);
        //}
        while (1) {
            world.send(0, rankTag, world.rank());
            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }

            world.recv(0, inpTag, jobInput);

            preLigands(jobInput, jobOut, localDir, workDir, useLocalDir, ligNameSet, podata);

            world.send(0, outTag, jobOut);

            //if(useLocalDir){
            //    std::string cmd = "rm -rf " + localDir+"/scratch";
            //    std::string errMesg = "Clean up local disk fails before calculation";
            //    LBIND::command(cmd, errMesg);
            //}
        }
    }

    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;
    
    return 0;
}


