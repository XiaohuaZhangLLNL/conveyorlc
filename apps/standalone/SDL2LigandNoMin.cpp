//
// Created by Zhang, Xiaohua on 5/24/20.
//

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <boost/scoped_ptr.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

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

#include "SDL2LigandNoMin.h"
#include "SDL2LigandPO.h"
#include "InitEnv.h"

using namespace LBIND;
using namespace conduit;
using namespace OpenBabel;

void toConduit(JobOutData& jobOut, std::string& ligCdtFile){

    try {

        Node n;

        std::string ligIDpath="lig/"+jobOut.ligID;

        n[ligIDpath + "/status"]=jobOut.error;

        std::string ligIDMeta =ligIDpath + "/meta";
        n[ligIDMeta] = jobOut.ligID;

        n[ligIDMeta + "/name"] = jobOut.ligName;
        n[ligIDMeta + "/LigPath"] = "";
        n[ligIDMeta + "/GBEN"] = 0;
        n[ligIDMeta + "/Mesg"] = jobOut.message;

        std::string ligIDFile =ligIDpath+ "/file/";
        n[ligIDFile+"LIG_min.pdbqt"] = jobOut.pdbqtStr;
        n[ligIDFile+"LIG_min.pdb"] = jobOut.pdbStr;

        relay::io::hdf5_append(n, ligCdtFile);

    }catch(conduit::Error &error){
        jobOut.message= error.message();
    }

}


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

void backupHDF5File(std::string& hdf5file)
{
    std::string cmd="cp " + hdf5file + " " + hdf5file+"-backup";
    std::string errMesg="Backup fails for "+hdf5file;
    command(cmd, errMesg);
}

void sdf2pdb(std::string& inputStr, std::string& outputStr)
{
    OBConversion conv;
    OBMol mol;
    conv.SetInFormat("sdf");
    conv.SetOutFormat("pdb");
    conv.SetOutputIndex(1);

    conv.ReadString(&mol, inputStr);
    outputStr = conv.WriteString(&mol, true);

}

void pdb2pdbqt(std::string& inputStr, std::string& outputStr)
{
    std::string mol2str;
    {
        OBConversion conv;
        OBMol mol;
        conv.SetInFormat("pdb");
        conv.SetOutFormat("mol2");
        conv.SetOutputIndex(1);
        conv.SetOptions("O", conv.OUTOPTIONS);

        conv.ReadString(&mol, inputStr);
        mol2str = conv.WriteString(&mol, true);
    }

    {
        OBConversion conv;
        OBMol mol;
        conv.SetInFormat("mol2");
        conv.SetOutFormat("pdbqt");
        conv.SetOutputIndex(1);
        conv.SetOptions("n", conv.OUTOPTIONS);

        conv.ReadString(&mol, mol2str);
        outputStr = conv.WriteString(&mol, true);
    }
}

void preLigands(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir, std::string& targetDir, bool useLocalDir) {

    try{
        jobOut.ligID=jobInput.dirBuffer;
        jobOut.message="Finished!";

        std::string sdfStr=jobInput.sdfBuffer;
        std::string pdbStr="";
        sdf2pdb(sdfStr, pdbStr);
        if(pdbStr.size()==0){
            throw LBindException("sdf2pdb failed");
        }

        boost::scoped_ptr<Pdb> pPdb(new Pdb());
        std::string pdbReNameStr="";
        pPdb->renameAtomStr(pdbStr, pdbReNameStr);
        if(pdbReNameStr.size()==0){
            throw LBindException("pPdb->renameAtomStr fail");
        }

        std::string pdbStripStr="";
        pPdb->stripStr(pdbReNameStr, pdbStripStr);
        if(pdbStripStr.size()==0){
            throw LBindException("pPdb->stripStr fail");
        }

        boost::scoped_ptr<Sdf> pSdf(new Sdf());
        if(jobInput.cmpName=="NoName"){
            jobOut.ligName=pSdf->getTitleStr(sdfStr);
        }else{
            jobOut.ligName=pSdf->getInfoStr(sdfStr, jobInput.cmpName);
        }

        std::string pdbEleStr="";
        pPdb->fixElementStr(pdbStripStr, pdbEleStr);
        if(pdbEleStr.size()==0){
            throw LBindException("pPdb->fixElementStr fail");
        }
        jobOut.pdbStr=pdbEleStr;

        std::string pdbqtStr="";
        pdb2pdbqt(pdbEleStr, pdbqtStr);
        if(pdbqtStr.size()==0){
            throw LBindException("pdb2pdbqt fail");
        }
        jobOut.pdbqtStr=pdbqtStr;

    } catch (LBindException& e){
        jobOut.message= e.what();
        jobOut.error=false;
        return;
    }

    jobOut.error=true;
    return;
}

int main(int argc, char** argv) {

    std::string workDir;
    std::string inputDir;
    std::string dataPath;
    std::string localDir;

    if(!initConveyorlcEnv(workDir, localDir, inputDir, dataPath)){
        exit(-1);
    }

    bool useLocalDir=(localDir!=workDir);

    POdata podata;

    bool success=SDL2LigandPO(argc, argv, podata);
    if(!success){
        exit(-1);
    }

    JobInputData jobInput;
    JobOutData jobOut;


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
    n["date"]="Create By SDL2Ligand at "+timeStamp();
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
        std::cout << "SDL2Ligand >> Cannot open file" << podata.sdfFile << std::endl;
        exit(-1);
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

            std::cout <<"Working on: " << jobInput.dirBuffer << std::endl;

            jobInput.sdfBuffer=contents;

            preLigands(jobInput, jobOut, localDir, workDir, useLocalDir);

            contents = ""; //! clean up the contents for the next structure.

            toConduit(jobOut, ligCdtFile);
            if(backupHDF5 && count%1000==0){
                backupHDF5File(ligHDF5Backup);
            }
        }

    }

    return 0;
}

