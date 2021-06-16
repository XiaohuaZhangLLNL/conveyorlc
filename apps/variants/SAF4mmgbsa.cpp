/* 
 * File:   mmgbsa.cpp
 * Author: zhang30
 *
 * Created on April 16, 2014, 10:52 AM
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <mpi.h>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/atom.h>

#include "MM/Amber.h"
#include "MM/CDTgbsa.h"
#include "Common/Tokenize.hpp"
#include "Structure/Constants.h"
#include "Common/File.hpp"
#include "Common/LBindException.h"
#include "Common/Command.hpp"
#include "Common/Chomp.hpp"
#include "Parser/Pdb.h"
#include "Parser/SanderOutput.h"

#include "../conduitppl/InitEnv.h"
#include "SAF4mmgbsa.h"
#include "SAF4mmgbsaPO.h"

using namespace conduit;
namespace mpi = boost::mpi;
using namespace boost::filesystem;
using namespace LBIND;

using namespace OpenBabel;

/*!
 * \breif mmgbsa MM-PB(GB)SA calculations on HPC using amber forcefield
 * \param argc
 * \param argv
 * \return success 
 * \defgroup mmgbsa_Commands mmgbsa Commands
 * 
 * Usage: mmgbsa <input-file>
 */

struct SAFmeta{
    int version;
    std::string dockInDir;
    std::string recFile;
    std::string ligFile;
    std::string ligName;

    std::string workDir;
    std::string localDir;
    std::string inputDir;
    std::string dataPath;

    std::string poseDir;

    std::string key;
    int procID;

    bool error;
    bool score_only;
    bool newapp;
    bool minimize;
    bool useScoreCF; //switch to turn on score cutoff
    double scoreCF;  // value for score cutoff
    double intDiel;
    double dockscore;
    double gbbind;
    std::string recID;
    std::string ligID;
    std::string poseID;
    std::string message;

    double ligGB;
    double recGB;
    double comGB;

    std::vector<std::string> nonRes;

};

void getRecData(SAFmeta &cdtMeta)
{
    Node nRec;

    std::string recFilePath=cdtMeta.workDir+"/"+cdtMeta.recFile+":rec/" + cdtMeta.recID;
    relay::io::load(recFilePath, nRec);

    int status = nRec["status"].as_int();
    if (status != 1) {
        throw LBindException("Receptor " + cdtMeta.recID + " preparation failed");
    }

    if(nRec.has_path("meta/NonStdAA"))
    {
        Node nNonRes = nRec["meta/NonStdAA"];
        NodeIterator itrNres = nNonRes.children();

        while (itrNres.has_next()) {
            Node &nonRes = itrNres.next();
            cdtMeta.nonRes.push_back(nonRes.as_string());
        }

    }


    std::vector<std::string> filenames = {"rec_min.pdb", "std4pdbqt.pdb"};

    for (std::string &name : filenames) {

        if(nRec.has_path("file/" + name)) {
            std::ofstream outfile(name);
            std::string outLines = nRec["file/" + name].as_string();
            outfile << outLines;
        }

    }


}


void getKeysHDF5(std::string& fileName, std::vector<std::string>& keysFinish)
{
    Node n;

    hid_t dock_hid=relay::io::hdf5_open_file_for_read(fileName);
    relay::io::hdf5_read(dock_hid, n);

    NodeIterator itrRec = n["gbsa"].children();

    while(itrRec.has_next())
    {
        Node &nRec=itrRec.next();

        NodeIterator itrLig = nRec.children();

        while(itrLig.has_next())
        {
            Node &nLig=itrLig.next();

            NodeIterator itrPose = nLig.children();

            while(itrPose.has_next()) {
                Node &nPose=itrPose.next();
                std::string key = nRec.name() + "/" + nLig.name() + "/" + nPose.name();
                keysFinish.push_back(key);
            }
        }

    }
    relay::io::hdf5_close_file(dock_hid);

}

void getKeysHDF5pIO(std::string& fileName, std::vector<std::string>& keysFinish)
{
    hid_t gbsa_hid;
    // Remove the corrupt gbsa HDF5 file
    try {
        gbsa_hid= relay::io::hdf5_open_file_for_read(fileName);
    } catch(conduit::Error &error){
        std::cout << "Warning: gbsa HDF5 file " << fileName << " is corrupt." << std::endl;
        std::cout << error.what() << std::endl;
        std::string cmd = "rm -f " + fileName;
        std::string errMesg = "Remove corrupt gbsa HDF5 file " +fileName;
        LBIND::command(cmd, errMesg);
        return;
    }

    std::vector<std::string> rec_names;
    relay::io::hdf5_group_list_child_names(gbsa_hid,"/gbsa/",rec_names);

    for(int i=0;i<rec_names.size();i++)
    {
        const std::string &curr_rec = rec_names[i];
        std::vector<std::string> lig_names;
        relay::io::hdf5_group_list_child_names(gbsa_hid,"/gbsa/"+curr_rec+"/",lig_names);

        for(int j=0; j<lig_names.size(); j++)
        {
            const std::string &curr_lig = lig_names[j];
            std::vector<std::string> pose_names;
            relay::io::hdf5_group_list_child_names(gbsa_hid,"/gbsa/"+curr_rec+"/"+curr_lig+"/", pose_names);

            for(int k=0; k<pose_names.size(); k++)
            {
                const std::string &curr_pose = pose_names[k];
                std::string key = curr_rec + "/" + curr_lig + "/" + curr_pose;
                keysFinish.push_back(key);
            }
        }


    }

    relay::io::hdf5_close_file(gbsa_hid);

}

void toConduit(CDTmeta &cdtMeta, std::string& gbsaHDF5File){
    try {

        Node n;

        std::string keyPath="gbsa/"+cdtMeta.key;

        n[keyPath+ "/status"]=cdtMeta.error;

        std::string recIDMeta =keyPath+ "/meta/";
        n[recIDMeta+"ligName"]=cdtMeta.ligName;
        n[recIDMeta+"comGB"]=cdtMeta.comGB;
        n[recIDMeta+"ligGB"]=cdtMeta.ligGB;
        n[recIDMeta+"recGB"]=cdtMeta.recGB;
        n[recIDMeta+"bindGB"]=cdtMeta.gbbind;
        n[recIDMeta+"dockScore"]=cdtMeta.dockscore;
        n[recIDMeta+"Mesg"]=cdtMeta.message;

        std::string recIDFile =keyPath + "/file/";

        std::vector<std::string> filenames={"Com.prmtop", "Com.inpcrd", "Com_min.rst",
                                            "Com_min.pdb", "Com_min_GB.out", "Com_min_GB_2.out", "Rec_minGB.out",
                                            "Rec_min.rst", "LIG_minGB.out "};

        for(std::string& name : filenames)
        {
            std::ifstream infile(name);
            if(infile.good())
            {
                std::string buffer((std::istreambuf_iterator<char>(infile)),
                                   std::istreambuf_iterator<char>());
                infile.close();
                n[recIDFile+name] = buffer;
            }
            else
            {
                std::cout << "File - " << name << " is not there." << std::endl;
            }
        }

        relay::io::hdf5_append(n, gbsaHDF5File);

    }catch(conduit::Error &error){
        cdtMeta.message= error.message();
    }


}

void mmgbsa(CDTmeta& cdtMeta) {

    try{
        if(cdtMeta.newapp) {
            CDTgbsa::runNew(cdtMeta);
        } else{
            CDTgbsa::run(cdtMeta);
        }
        cdtMeta.message= "Finished!";
        cdtMeta.error=true;

    } catch (conduit::Error& e){
        cdtMeta.message= e.what();
        cdtMeta.error=false;
    } catch (LBindException& e){
        cdtMeta.message= e.what();
        cdtMeta.error=false;
    }catch (...){
        cdtMeta.message= "Unknown error";
        cdtMeta.error=false;
    }

}

int main(int argc, char** argv){
    std::string workDir;
    std::string inputDir;
    std::string dataPath;
    std::string localDir;

    if(!initConveyorlcEnv(workDir, localDir, inputDir, dataPath)){
        return -1;
    }

    std::string subDir=workDir;
    JobInputData jobInput;
    jobInput.ambVersion=16;
    OBConversion conv;
    OBMol mol;
    conv.SetInFormat("mol2");
    conv.SetOutFormat("pdb");

    std::string mol2file = "3OMQ_1660.mol2";
    conv.ReadFile(&mol, mol2file);
    double tot=0;
    for(int i = 1; i <= mol.NumAtoms(); ++i) {

        /* ---- Get a pointer to ith atom ---- */
        OBAtom *atom = mol.GetAtom(i);
        tot+=atom->GetPartialCharge();
    }

    std::cout << "Charge = " <<tot<< std::endl;
    std::cout << mol.NumResidues()<< std::endl;
    OBResidue *residue=mol.GetResidue(0);
    residue->SetName("LIG");
    std::string pdb = conv.WriteString(&mol);
    std::cout << "PDB"<< std::endl;

    const std::string atomStr="ATOM";
    std::vector<std::string> lines;
    tokenize(lines, pdb,"\n");

    ofstream ofh("ligand.pdb");
    for(int i=0; i< lines.size(); i++){
        std::string line=lines[i];
        if (line.compare(0,4, atomStr)==0){
            ofh << line << std::endl;
        }
    }
    ofh.close();

    std::string mol2fileFix="ligand_fix.mol2";
    conv.SetOutFormat("mol2");
    std::string mol2fix = conv.WriteString(&mol);
    ofstream ofhMol2(mol2fileFix);
    ofhMol2 << mol2fix;
    ofhMol2.close();

    //std::cout << pdb << std::endl;
    std::string output="ligand.mol2";
    std::string options=" -c bcc -nc "+ std::to_string(int(tot));

    boost::scoped_ptr<Amber> pAmber(new Amber(jobInput.ambVersion));
    pAmber->antechamber(mol2fileFix, output, options);

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

    SAFmeta cdtMeta;
    std::string cmd;
    std::string errMesg;

    cdtMeta.version=16;
    cdtMeta.recFile="scratch/receptor.hdf5";
    //cdtMeta.ligFile=podata.ligFile;

    cdtMeta.minimize=false;
    cdtMeta.intDiel = 4;

    cdtMeta.workDir=workDir;
    cdtMeta.localDir=localDir;
    cdtMeta.dataPath=dataPath;
    cdtMeta.inputDir=inputDir;

    jobInput.key="3OMQ/DTXSID9064392/3OMQ_1660";
    cdtMeta.key=jobInput.key;
    cdtMeta.procID=jobInput.procID;

    std::vector<std::string> keystrs;
    tokenize(cdtMeta.key, keystrs, "/");
    if(keystrs.size()==3){
        cdtMeta.recID=keystrs[0];
        cdtMeta.ligID=keystrs[1];
        cdtMeta.poseID=keystrs[2];
    }else{
        throw LBindException("Key should have 3 fields: "+cdtMeta.key);
    }

    std::string libDir=cdtMeta.inputDir+"/lib/";
    std::string poseDir=cdtMeta.localDir+"/scratch/gbsa/"+cdtMeta.key;
    cdtMeta.poseDir=poseDir;


    getRecData(cdtMeta);

    cmd = "grep -v END std4pdbqt.pdb   > dd.pdb && cat dd.pdb ligand.pdb > com_init.pdb";
    errMesg="MMGBSA::run combine rec and lig to com pdb fails";
    command(cmd, errMesg);

    std::vector<std::vector<int> > ssList;
    {
        std::string stdPdbFile="std4pdbqt.pdb";
        boost::scoped_ptr<Pdb> pPdb(new Pdb() );
        pPdb->getDisulfide(stdPdbFile, ssList);
    }

    std::string tleapFName="Com_leap.in";
    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }

        if(cdtMeta.version==16 || cdtMeta.version==13){
            tleapFile << "source leaprc.ff14SB" << std::endl;
            tleapFile << "source leaprc.phosaa10\n";
        }else{
            tleapFile << "source leaprc.ff99SB" << std::endl;
        }
        tleapFile << "source leaprc.gaff\n"
                  << "source leaprc.water.tip3p\n";

        for(unsigned int i=0; i<cdtMeta.nonRes.size(); ++i){
            std::string nonResRaw=cdtMeta.nonRes[i];
            std::vector<std::string> nonResStrs;
            const std::string delimiter=".";
            tokenize(nonResRaw, nonResStrs, delimiter);
            if(nonResStrs.size()==2 && nonResStrs[1]=="M"){
                tleapFile << nonResStrs[0] <<" = loadmol2 "<< libDir << nonResStrs[0] << ".mol2 \n";
            }else{
                tleapFile << "loadoff " << libDir << nonResStrs[0] << ".off \n";
            }

            tleapFile << "loadamberparams "<< libDir << nonResStrs[0] <<".frcmod \n";
        }

        tleapFile << "loadamberparams ligand.frcmod\n"
                  << "loadoff LIG.lib\n"
                  << "COM = loadpdb com_init.pdb\n";

        for(unsigned int i=0; i<ssList.size(); ++i){
            std::vector<int> pair=ssList[i];
            if(pair.size()==2){
                tleapFile << "bond COM."<< pair[0] <<".SG COM." << pair[1] <<".SG \n";
            }
        }

        tleapFile << "set default PBRadii mbondi2\n"
                  << "saveamberparm COM Com.prmtop Com.inpcrd\n"
                  << "quit \n";

        tleapFile.close();
    }

    cmd="tleap -f "+tleapFName+" >& Com_leap.log";
    errMesg="Complex tleap fails";
    command(cmd, errMesg);

    checkFName="Com.prmtop";
    {
        if(!fileExist(checkFName)){
            std::string message="Com.prmtop file does not exist.";
            throw LBindException(message);
        }

        if(fileEmpty(checkFName)){
            std::string message="Com.prmtop is empty.";
            throw LBindException(message);
        }
    }

    std::string minFName="Com_min.in";
    {
        std::ofstream minFile;
        try {
            minFile.open(minFName.c_str());
        }
        catch(...){
            std::string mesg="Cannot open min file: "+minFName;
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
                << "  intdiel= " << cdtMeta.intDiel << ",\n"
                << "  cut    = 25,\n"
                << "  drms=1e-3,\n"
                << "  ntr=1,\n"
                << "  restraint_wt=5.0,\n"
                << "  restraintmask='@CA,C,N= & !:LIG'\n"
                << " /\n" << std::endl;

        minFile.close();
    }

    std::string sanderOut="Com_min_GB.out";
    if(cdtMeta.version==13){
        cmd="sander13  -O -i Com_min.in -o Com_min_GB.out -p Com.prmtop -c Com.inpcrd -ref Com.inpcrd  -x Com.mdcrd -r Com_min.rst";
    }else{
        cmd="sander  -O -i Com_min.in -o Com_min_GB.out -p Com.prmtop -c Com.inpcrd -ref Com.inpcrd  -x Com.mdcrd -r Com_min.rst";
    }
    //std::cout <<cmd <<std::endl;
    errMesg="Complex minimization fails";
    command(cmd, errMesg);

    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    //cdtMeta.comGB=0;
    //bool success=pSanderOutput->getEAmber(sanderOut,cdtMeta.comGB);
    //if(!success) throw LBindException("Cannot get complex GB energy");

    // end receptor energy re-calculation

    minFName="Com_minGB_2.in";
    {
        std::ofstream minFile;
        try {
            minFile.open(minFName.c_str());
        }
        catch(...){
            std::string mesg="Cannot open min file: "+minFName;
            throw LBindException(mesg);
        }

        minFile << "title..\n"
                << "&cntrl\n"
                << "  imin   = 1,\n"
                << "  ntmin   = 1,\n"
                << "  maxcyc = 0,\n"
                << "  ncyc   = 0,\n"
                << "  ntpr   = 1,\n"
                << "  ntb    = 0,\n"
                << "  igb    = 5,\n"
                << "  gbsa   = 1,\n"
                << "  intdiel= " << cdtMeta.intDiel << ",\n"
                << "  cut    = 999,\n"
                << "  ntr=0,\n"
                << "  restraint_wt=5.0,\n"
                << "  restraintmask='!@H='\n"
                << " /\n";

        minFile.close();
    }

    if(cdtMeta.version==13){
        cmd="sander13 -O -i Com_minGB_2.in -o Com_minGB_2.out  -p Com.prmtop -c Com_min.rst -ref Com_min.rst -x Com2.mdcrd -r Com_min2.rst";
    }else{
        cmd="sander -O -i Com_minGB_2.in -o Com_minGB_2.out -p Com.prmtop -c Com_min.rst -ref Com_min.rst -x Com2.mdcrd -r Com_min2.rst";
    }
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run complex energy fails";
    command(cmd, errMesg);

    sanderOut="Com_minGB_2.out";
    cdtMeta.comGB=0;
    bool success=pSanderOutput->getEnergy(sanderOut, cdtMeta.comGB);
    if(!success) throw LBindException("Cannot get complex GB energy");

    std::cout << "Complex GB Minimization Energy: " << cdtMeta.comGB <<" kcal/mol."<< std::endl;

    // receptor energy calculation
    if (cdtMeta.version == 16) {
        cmd="ambpdb -p Com.prmtop -aatm -c Com_min.rst > Com_min.pdb";
    } else {
        cmd="ambpdb -p Com.prmtop -aatm < Com_min.rst > Com_min.pdb";
    }
    errMesg="ambpdb complex fails";
    command(cmd, errMesg);

    cmd="grep -v LIG Com_min.pdb > rec_tmp.pdb ";
    errMesg="Grep REC Com_min.pdb fails";
    command(cmd, errMesg);

    // For receptor energy re-calculation
    tleapFName="rec_leap.in";

    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }

        if (cdtMeta.version == 16 || cdtMeta.version==13) {
            tleapFile << "source leaprc.ff14SB\n";
            tleapFile << "source leaprc.phosaa10\n";
        } else {
            tleapFile << "source leaprc.ff99SB\n";
        }

        tleapFile << "source leaprc.gaff\n";
        tleapFile << "source leaprc.water.tip3p\n";

        for(unsigned int i=0; i<cdtMeta.nonRes.size(); ++i){
            std::string nonResRaw=cdtMeta.nonRes[i];
            std::vector<std::string> nonResStrs;
            const std::string delimiter=".";
            tokenize(nonResRaw, nonResStrs, delimiter);
            if(nonResStrs.size()==2 && nonResStrs[1]=="M"){
                tleapFile << nonResStrs[0] <<" = loadmol2 "<< libDir << nonResStrs[0] << ".mol2 \n";
            }else{
                tleapFile << "loadoff " << libDir << nonResStrs[0] << ".off \n";
            }

            tleapFile << "loadamberparams "<< libDir << nonResStrs[0] <<".frcmod \n";
        }

        tleapFile << "REC = loadpdb rec_tmp.pdb\n";

        for(unsigned int i=0; i<ssList.size(); ++i){
            std::vector<int> pair=ssList[i];
            if(pair.size()==2){
                tleapFile << "bond REC."<< pair[0] <<".SG REC." << pair[1] <<".SG \n";
            }
        }
        tleapFile << "set default PBRadii mbondi2\n"
                  << "saveamberparm REC REC.prmtop REC.inpcrd\n"
                  << "quit\n";

        tleapFile.close();
    }

    cmd="tleap -f rec_leap.in >& rec_leap.log";
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run tleap receptor fails";
    command(cmd, errMesg);

    checkFName="REC.prmtop";
    {
        if(!fileExist(checkFName)){
            std::string message="REC.prmtop file does not exist.";
            throw LBindException(message);
        }

        if(fileEmpty(checkFName)){
            std::string message="REC.prmtop is empty.";
            throw LBindException(message);
        }
    }
    // end receptor energy re-calculation

    minFName="Rec_minGB.in";
    {
        std::ofstream minFile;
        try {
            minFile.open(minFName.c_str());
        }
        catch(...){
            std::string mesg="Cannot open min file: "+minFName;
            throw LBindException(mesg);
        }

        minFile << "title..\n"
                << "&cntrl\n"
                << "  imin   = 1,\n"
                << "  ntmin   = 1,\n"
                << "  maxcyc = 0,\n"
                << "  ncyc   = 0,\n"
                << "  ntpr   = 1,\n"
                << "  ntb    = 0,\n"
                << "  igb    = 5,\n"
                << "  gbsa   = 1,\n"
                << "  intdiel= " << cdtMeta.intDiel << ",\n"
                << "  cut    = 999,\n"
                << "  ntr=0,\n"
                << "  restraint_wt=5.0,\n"
                << "  restraintmask='!@H='\n"
                << " /\n";

        minFile.close();
    }

    if(cdtMeta.version==13){
        cmd="sander13 -O -i Rec_minGB.in -o Rec_minGB.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    }else{
        cmd="sander -O -i Rec_minGB.in -o Rec_minGB.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    }
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run receptor minimization fails";
    command(cmd, errMesg);

    sanderOut="Rec_minGB.out";
    cdtMeta.recGB=0;
    success=pSanderOutput->getEnergy(sanderOut, cdtMeta.recGB);
    if(!success) throw LBindException("Cannot get receptor GB energy");

    cmd="grep LIG Com_min.pdb > lig_tmp.pdb ";
    errMesg="Grep LIG Com_min.pdb fails";
    command(cmd, errMesg);

    tleapFName="Lig_leap2.in";
    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="mmgbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }
        if(cdtMeta.version==16 || cdtMeta.version==13){
            tleapFile << "source leaprc.ff14SB" << std::endl;
            tleapFile << "source leaprc.phosaa10\n";
        }else{
            tleapFile << "source leaprc.ff99SB" << std::endl;
        }
        tleapFile << "source leaprc.gaff\n"
                  << "source leaprc.water.tip3p\n"
                  << "loadamberparams ligand.frcmod\n"
                  << "loadoff LIG.lib\n"
                  << "LIG = loadpdb lig_tmp.pdb\n"
                  << "set default PBRadii mbondi2\n"
                  << "saveamberparm LIG LIG.prmtop LIG.inpcrd\n"
                  << "quit " << std::endl;

        tleapFile.close();
    }

    cmd="tleap -f "+tleapFName+" >& lig_leap.log";
    //std::cout <<cmd <<std::endl;
    errMesg="Ligand tleap fails";
    command(cmd, errMesg);
    checkFName="LIG.prmtop";
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
    minFName = "LIG_minGB.in";
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
                << "  maxcyc = 0,\n"
                << "  ncyc   = 0,\n"
                << "  ntpr   = 1,\n"
                << "  ntb    = 0,\n"
                << "  igb    = 5,\n"
                << "  gbsa   = 1,\n"
                << "  intdiel= " << cdtMeta.intDiel << ",\n"
                << "  cut    = 999,\n"
                << " /\n" << std::endl;

        minFile.close();
    }

    if (cdtMeta.version == 13) {
        cmd = "sander13  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
    } else {
        cmd = "sander  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
    }
    //std::cout <<cmd <<std::endl;
    errMesg = "sander ligand minimization fails";
    command(cmd, errMesg);
    sanderOut = "LIG_minGB.out";
    double ligGBen = 0;
    success = pSanderOutput->getEnergy(sanderOut, ligGBen);
    cdtMeta.ligGB = ligGBen;

    cdtMeta.gbbind=cdtMeta.comGB-cdtMeta.recGB-cdtMeta.ligGB;


}
/*
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

    POdata podata;
    bool success = SAF4mmgbsaPO(argc, argv, podata);
    if (!success) {
        world.abort(1);
    }

    if(!initConveyorlcEnv(workDir, localDir, inputDir, dataPath)){
        world.abort(1);
    }

    bool useLocalDir=(localDir!=workDir);

    if(useLocalDir){
        std::string cmd = "rm -rf " + localDir+"/scratch";
        std::string errMesg = "Clean up local disk fails before calculation";
        LBIND::command(cmd, errMesg);
        world.barrier();
    }

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        world.abort(1);
    }

    JobInputData jobInput;

    std::vector<std::string> dockingKeys;
    std::vector<std::vector<std::string> > allDockingKeys;


    std::unordered_map<std::string, int> keysCalc;
    std::vector<std::string> keysFinish;


    if (world.rank() == 0) {

        for (auto cKey : keysCalc) {

            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);

            world.send(freeProc, jobTag, jobFlag);

            jobInput.key = cKey.first;
            jobInput.procID=cKey.second;
            std::cout << "At Process: " << freeProc << " working on  Key: " << jobInput.key << std::endl;

            world.send(freeProc, inpTag, jobInput);

        }

        for(unsigned i=1; i < world.size(); ++i){
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;;
            world.send(freeProc, jobTag, jobFlag);
        }        
        
    }else {

        std::string gbsaHDF5File=workDir+"/scratch/gbsaHDF5/gbsa_proc"+std::to_string(world.rank())+".hdf5:/";
        while (1) {
            world.send(0, rankTag, world.rank());
            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }
            // Receive parameters
            world.recv(0, inpTag, jobInput);

            //initialize Meta data
            CDTmeta cdtMeta;
            cdtMeta.version=podata.version;
            cdtMeta.recFile=podata.recFile;
            cdtMeta.ligFile=podata.ligFile;
            cdtMeta.score_only=podata.score_only;
            cdtMeta.newapp=podata.newapp;


            if(podata.minimizeFlg=="on"){
                cdtMeta.minimize=true;
            }else{
                cdtMeta.minimize=false;
            }

            cdtMeta.intDiel = podata.intDiel;

            cdtMeta.workDir=workDir;
            cdtMeta.localDir=localDir;
            cdtMeta.dataPath=dataPath;
            cdtMeta.inputDir=inputDir;

            cdtMeta.key=jobInput.key;
            cdtMeta.procID=jobInput.procID;

            mmgbsa(cdtMeta);

            toConduit(cdtMeta, gbsaHDF5File);

            // Remove the working dire
            if(cdtMeta.error && !podata.keep) {
                chdir(cdtMeta.localDir.c_str());
                std::string cmd = "rm -rf " + cdtMeta.poseDir;
                std::string errMesg = "remove poseDir fails";
                LBIND::command(cmd, errMesg);
            } else if (useLocalDir && !cdtMeta.error && podata.keep){
                std::string scrPoseDir = cdtMeta.workDir+"/scratch/gbsa/"+cdtMeta.key;
                std::string cmd="mkdir -p "+scrPoseDir;
                std::string errMesg="Run mkdir srcPoseDir fails";
                LBIND::command(cmd, errMesg);

                cmd="cp * "+scrPoseDir;
                errMesg="copy file from local to file system fails";
                LBIND::command(cmd, errMesg);

                chdir(cdtMeta.localDir.c_str());
                cmd = "rm -rf " + cdtMeta.poseDir;
                errMesg = "remove poseDir fails";
                LBIND::command(cmd, errMesg);
            }

            
        }

    }

    if(useLocalDir){
        world.barrier();
        std::string cmd = "rm -rf " + localDir+"/scratch";
        std::string errMesg = "Clean up local disk fails before calculation";
        LBIND::command(cmd, errMesg);
    }
    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;
    
    return 0;
}
*/
