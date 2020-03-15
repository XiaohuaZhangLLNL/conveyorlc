//
// Created by Zhang, Xiaohua on 2018-12-26.
//

#include <unistd.h>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

#include <boost/scoped_ptr.hpp>

#include "Common/Command.hpp"
#include "Common/File.hpp"
#include "Common/Tokenize.hpp"
#include "MM/CDTgbsa.h"
#include "Parser/Pdb.h"
#include "Parser/SanderOutput.h"
#include "CDTgbsa.h"
#include "MM/Amber.h"

using namespace conduit;

namespace LBIND{

void CDTgbsa::getLigData(CDTmeta &cdtMeta) {

    Node nLig;

    //hid_t lig_hid = relay::io::hdf5_open_file_for_read(cdtMeta.workDir+"/"+cdtMeta.ligFile);
    //relay::io::hdf5_read(lig_hid, n);
    // Partial I/O
    std::string ligFilePath=cdtMeta.workDir+"/"+cdtMeta.ligFile+":lig/" + cdtMeta.ligID;
    relay::io::load(ligFilePath, nLig);

    //Node nLig = n["lig/" + cdtMeta.ligID];
    int status = nLig["status"].as_int();
    if (status != 1) {
        throw LBindException("Ligand " + cdtMeta.ligID + " preparation failed");
    }

    if(nLig.has_path("meta/GBEN")) {
        cdtMeta.ligGB = nLig["meta/GBEN"].as_double();
    }else if(cdtMeta.score_only){
        cdtMeta.ligGB = 0;
    }else{
        throw LBindException("Ligand " + cdtMeta.ligID + " has no GB energy");
    }
    std::vector<std::string> filenames = {"LIG.lib", "LIG_min.pdbqt", "ligand.frcmod", "LIG_min.pdb", "ligand.mol2"};

    for (std::string &name : filenames) {

        if(nLig.has_path("file/" + name)) {
            std::ofstream outfile(name);
            std::string outLines = nLig["file/" + name].as_string();
            outfile << outLines;
        }

    }

    //relay::io::hdf5_close_file(lig_hid);

}

void CDTgbsa::getRecData(CDTmeta &cdtMeta)
{
    Node nRec;

    //hid_t rec_hid=relay::io::hdf5_open_file_for_read(cdtMeta.workDir+"/"+cdtMeta.recFile);
    //relay::io::hdf5_read(rec_hid, n);
    //Partial I/O
    std::string recFilePath=cdtMeta.workDir+"/"+cdtMeta.recFile+":rec/" + cdtMeta.recID;
    relay::io::load(recFilePath, nRec);

    //Node nRec = n["rec/" + cdtMeta.recID];
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

    //relay::io::hdf5_close_file(rec_hid);

}

void CDTgbsa::getDockData(LBIND::CDTmeta &cdtMeta)
{
    Node n;

    //
    std::string dockHDF5file=cdtMeta.workDir+"/"+cdtMeta.dockInDir+"/dock_proc"+std::to_string(cdtMeta.procID)+".hdf5";
    //std::string pdbqtPath="dock/"+cdtMeta.recID+"/"+cdtMeta.ligID+"/file/"+name;
    std::string dockPath="dock/"+cdtMeta.recID+"/"+cdtMeta.ligID;
    std::string dockdataPath=dockHDF5file+":"+dockPath;

    //Partial I/O
    relay::io::load(dockdataPath, n);

    if(n.has_path("meta/ligName")){
        cdtMeta.ligName=n["meta/ligName"].as_string();
    }else{
        cdtMeta.ligName="NoName";
    }

    std::string name="poses.pdbqt";
    std::ofstream outfile(name);
    std::string outLines = n["file/"+name].as_string();
    outfile << outLines;

    if(cdtMeta.score_only) {
        if(n.has_path("meta/scores/1")) {
            cdtMeta.dockscore = n["meta/scores/1"].as_double();
        }
        return;
    }
    //hid_t dock_hid=relay::io::hdf5_open_file_for_read(dockHDF5file);
    //relay::io::hdf5_read(dock_hid, n);

    //std::string posespdbqt="poses.pdbqt";
    //std::vector<std::string> filenames = {posespdbqt};

    //for (std::string &name : filenames) {
    //    std::ofstream outfile(name);
    //    std::string outLines = n["dock/"+cdtMeta.recID+"/"+cdtMeta.ligID+"/file/" + name].as_string();
    //    outfile << outLines;
    //}
    //relay::io::hdf5_close_file(dock_hid);
    //! Processing poses

    std::string ligpdbqt="lig_model.pdbqt";
    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    std::string pIDstr=cdtMeta.poseID.substr(1, cdtMeta.poseID.size()-1);
    int pID=std::stoi(pIDstr);

    if(cdtMeta.useScoreCF){
        double pose1score=0;
        pPdb->readByModel(name, "pose1.pdbqt", 1, pose1score);
        if(pose1score > cdtMeta.scoreCF){
            cdtMeta.gbbind=0;
            cdtMeta.comGB=0;
            cdtMeta.recGB=0;
            cdtMeta.ligGB=0;
            throw LBindException("Docking score is higher that threshold");
        }
    }

    pPdb->readByModel(name, ligpdbqt, pID, cdtMeta.dockscore);

    std::string posePDB="lig_model.pdb";
    std::string cmd="obabel -ipdbqt "+ligpdbqt+" -opdb -O "+posePDB;
    std::string errMesg="obabel fail to convert ligand pdbqt to pdb";
    command(cmd, errMesg);


    std::string tleapFName="Lig_leap.in";
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
                  << "LIG = loadpdb lig_model.pdb\n"
                  << "set default PBRadii mbondi2\n"
                  << "savepdb LIG lig_full.pdb\n"
                  << "quit " << std::endl;

        tleapFile.close();
    }

    cmd="tleap -f "+tleapFName+" >& lig_leap.log";
    //std::cout <<cmd <<std::endl;
    errMesg="Ligand tleap fails";
    command(cmd, errMesg);
}

void CDTgbsa::ligMinimize(CDTmeta &cdtMeta){

    boost::scoped_ptr<Amber> pAmber(new Amber(cdtMeta.version));
    //! leap to obtain forcefield for ligand
    std::string ligName="LIG";
    std::string tleapFile="leap.in";
    std::string output="ligand.mol2";

    pAmber->tleapInput(output,ligName,tleapFile, cdtMeta.poseDir);
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
                << "  ntmin   = 3,\n"
                << "  maxcyc = 2000,\n"
                << "  ncyc   = 1000,\n"
                << "  ntpr   = 200,\n"
                << "  ntb    = 0,\n"
                << "  igb    = 5,\n"
                << "  gbsa   = 1,\n"
                << "  intdiel= " << cdtMeta.intDiel << ",\n"
                << "  cut    = 15,\n"
                << " /\n" << std::endl;

        minFile.close();
    }

    std::string cmd;
    if (cdtMeta.version == 13) {
        cmd = "sander13  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
    } else {
        cmd = "sander  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
    }
    //std::cout <<cmd <<std::endl;
    std::string errMesg = "sander ligand minimization fails";
    command(cmd, errMesg);
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    std::string sanderOut = "LIG_minGB.out";
    double ligGBen = 0;
    bool success = pSanderOutput->getEnergy(sanderOut, ligGBen);
    cdtMeta.ligGB = ligGBen;
    return;
}

void CDTgbsa::run(CDTmeta &cdtMeta){

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

    std::string cmd="mkdir -p "+poseDir;
    std::string errMesg="CDTgbsa::run mkdir poseDir fails";
    command(cmd, errMesg);
    chdir(poseDir.c_str());

    getLigData(cdtMeta);
    getRecData(cdtMeta);
    getDockData(cdtMeta);

    if(cdtMeta.score_only){
        ligMinimize(cdtMeta);
        cmd = "grep -v END std4pdbqt.pdb > dd.pdb && cat dd.pdb LIG_min.pdb > com_init.pdb";
    }else {
        cmd = "grep -v END rec_min.pdb   > dd.pdb && cat dd.pdb lig_full.pdb > com_init.pdb";
    }
    errMesg="MMGBSA::run combine rec and lig to com pdb fails";
    command(cmd, errMesg);

    std::vector<std::vector<int> > ssList;
    {
        std::string stdPdbFile="rec_min.pdb";
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

    std::string checkFName="Com.prmtop";
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

    // Skip minimization
    if(!cdtMeta.minimize){
        // receptor energy calculation
        cmd="ambpdb -p Com.prmtop -aatm < Com.inpcrd > Com_min.pdb";
        errMesg="ambpdb complex fails";
        command(cmd, errMesg);
        return;
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
                << "  ntmin   = 3,\n"
                << "  maxcyc = 2000,\n"
                << "  ncyc   = 1000,\n"
                << "  ntpr   = 200,\n"
                << "  ntb    = 0,\n"
                << "  igb    = 5,\n"
                << "  gbsa   = 1,\n"
                << "  intdiel= " << cdtMeta.intDiel << ",\n"
                << "  cut    = 15,\n"
                << "  ntr=1,\n"
                << "  restraint_wt=5.0,\n"
                << "  restraintmask='!@H= & !:LIG'\n"
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
    cdtMeta.comGB=0;
    bool success=pSanderOutput->getEAmber(sanderOut,cdtMeta.comGB);
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
    errMesg="Grep LIG Com_min.pdb fails";
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
                << "  ntmin   = 3,\n"
                << "  maxcyc = 1,\n"
                << "  ncyc   = 1,\n"
                << "  ntpr   = 1,\n"
                << "  ntb    = 0,\n"
                << "  igb    = 5,\n"
                << "  gbsa   = 1,\n"
                << "  intdiel= " << cdtMeta.intDiel << ",\n"
                << "  cut    = 15,\n"
                << "  ntr=1,\n"
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

    cdtMeta.gbbind=cdtMeta.comGB-cdtMeta.recGB-cdtMeta.ligGB;


}


void CDTgbsa::runNew(CDTmeta &cdtMeta){

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

        std::string cmd="mkdir -p "+poseDir;
        std::string errMesg="CDTgbsa::run mkdir poseDir fails";
        command(cmd, errMesg);
        chdir(poseDir.c_str());

        getLigData(cdtMeta);

        getDockData(cdtMeta);
        
        getRecData(cdtMeta);

        if(cdtMeta.score_only){
            cmd = "grep -v END std4pdbqt.pdb > dd.pdb && cat dd.pdb LIG_min.pdb > com_init.pdb";
        }else {
            cmd = "grep -v END rec_min.pdb   > dd.pdb && cat dd.pdb lig_full.pdb > com_init.pdb";
        }
        errMesg="MMGBSA::run combine rec and lig to com pdb fails";
        command(cmd, errMesg);

        std::vector<std::vector<int> > ssList;
        {
            std::string stdPdbFile="rec_min.pdb";
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

        std::string checkFName="Com.prmtop";
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

        // Skip minimization
        if(!cdtMeta.minimize){
            // receptor energy calculation
            cmd="ambpdb -p Com.prmtop -aatm < Com.inpcrd > Com_min.pdb";
            errMesg="ambpdb complex fails";
            command(cmd, errMesg);
            return;
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


} //namespace LBIND
