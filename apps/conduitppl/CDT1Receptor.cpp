/* 
 * File:   PPL1Receptor.cpp
 * Author: zhang
 *
 * Created on March 18, 2014, 12:10 PM
 */

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

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

#include "Parser/Pdb.h"
#include "Parser/Mol2.h"
#include "Parser/SanderOutput.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Coor3d.h"
#include "Structure/Constants.h"
#include "Structure/Complex.h"
#include "Structure/Atom.h"
#include "Common/LBindException.h"
#include "Common/Tokenize.hpp"
#include "Common/Command.hpp"
#include "BackBone/Surface.h"
#include "BackBone/Grid.h"
#include "Structure/ParmContainer.h"
#include "Common/File.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "Common/Utils.h"
#include "XML/XMLHeader.hpp"

#include "CDT1Receptor.h"
#include "CDT1ReceptorPO.h"
#include "InitEnv.h"

namespace mpi = boost::mpi;
using namespace LBIND;
using namespace conduit;

/*!
 * \breif amberTools Calculate receptor GB/PB minimization energy from download PDB files.
 * \param argc
 * \param argv argv[1] takes the input file name
 * \return success 
 * \defgroup amberTools_Commands amberTools Commands
 *
 * 
 * Usage on HPC slurm
 * 
 * \verbatim
 
    export AMBERHOME=/usr/gapps/medchem/amber/amber12
    export PATH=$AMBERHOME/bin/:$PATH
    export WORKDIR=`pwd`
    export LBindData=/usr/gapps/medchem/medcm/data/

    srun -N4 -n48 -ppdebug CDT1Receptor  --input pdb.list --output out

    pdb.list: contain a list of path to receptor PDB Files.
 *  pdb/1NJS.pdb
 *  pdb/3BKL.pdb
 *  ...

    Requires: define LBindData 
 *  
 * 
 * Note: the non-standard residues are changed to ALA due to lack of amber forcefield.
   \endverbatim
 */


void toConduit(JobOutData& jobOut, std::string& recCdtFile){

    try {

        Node n;

        n["rec/"+jobOut.pdbid + "/status"]=jobOut.error;

        std::string recIDMeta ="rec/"+jobOut.pdbid + "/meta";
        n[recIDMeta] = jobOut.pdbid;

        n[recIDMeta + "/PDBPath"] = jobOut.pdbFilePath;

        for (unsigned i = 0; i < jobOut.nonRes.size(); ++i) {
            std::string iStr = Sstrm<std::string, unsigned>(i + 1);
            n[recIDMeta + "/NonStdAA/" + iStr] = jobOut.nonRes[i];
        }

        n[recIDMeta + "/IsCutProt"] = jobOut.cutProt;
        n[recIDMeta + "/RecPath"] = jobOut.recPath;
        n[recIDMeta + "/GBEN"] = jobOut.gbEn;
        n[recIDMeta + "/Site/Cluster"] = jobOut.clust;
        n[recIDMeta + "/Site/Volume"] = jobOut.volume;
        n[recIDMeta + "/Site/Centroid/X"] = jobOut.centroid.getX();
        n[recIDMeta + "/Site/Centroid/Y"] = jobOut.centroid.getY();
        n[recIDMeta + "/Site/Centroid/Z"] = jobOut.centroid.getZ();
        n[recIDMeta + "/Site/Dimension/X"] = jobOut.dimension.getX();
        n[recIDMeta + "/Site/Dimension/Y"] = jobOut.dimension.getY();
        n[recIDMeta + "/Site/Dimension/Z"] = jobOut.dimension.getZ();
        n[recIDMeta + "/Mesg"] = jobOut.message;
        std::string recIDFile ="rec/"+jobOut.pdbid + "/file/";

        std::vector<std::string> filenames={"rec_min.pdbqt", "rec_min.rst", "rec.prmtop", "rec_min.pdb",
                                            "rec_minGB.out", "site.txt", "rec_geo.txt"};
        if(jobOut.cutProt){
            filenames.push_back("recCut_min.rst");
            filenames.push_back("recCut.prmtop");
            filenames.push_back("recCut_minGB.out");
        }

        std::string gridfilename="Grid-"+std::to_string(jobOut.clust)+".pdb";
        filenames.push_back(gridfilename);

        for(std::string& name : filenames)
        {
            std::string filename=jobOut.recPath+"/"+name;
            std::ifstream infile(filename);
            if(infile.good())
            {
                std::string buffer((std::istreambuf_iterator<char>(infile)),
                                   std::istreambuf_iterator<char>());
                infile.close();
                n[recIDFile+name] = buffer;
            }
            else
            {
                std::cout << "File - " << filename << " is not there." << std::endl;
            }
        }

        relay::io::hdf5_append(n, recCdtFile);

    }catch(conduit::Error &error){
        jobOut.message= error.message();
    }

}


bool isRun(JobInputData& jobInput){

    std::string pdbid;
    getFileBasename(jobInput.dirBuffer, pdbid);
    //std::cout << "Receptor - " << jobInput.dirBuffer << " input!" << std::endl;

    Node n;
    relay::io::hdf5_read(jobInput.recCdtFile, n);
    std::string path="rec/"+pdbid;
    if(n.has_path(path))
    {
        std::cout << "Receptor - " << pdbid << " has already completed!" << std::endl;
        return true;
    }
    return false;
}

void rmRecDir(JobOutData& jobOut)
{
    std::string cmd="rm -rf " + jobOut.recPath;
    std::string errMesg="Remove fails for "+jobOut.recPath;
    command(cmd, errMesg);
}

bool getSiteFromLigand(JobOutData& jobOut, Coor3d& centriod, Coor3d& boxDim){
    bool hasSubResCoor=false;

    std::cout << "jobOut.subRes=" << jobOut.subRes << std::endl;
    std::string subResFileName=jobOut.subRes;
    std::string fileExtension=subResFileName.substr(subResFileName.find_last_of(".") + 1);
    std::cout << "fileExtension=" << fileExtension << std::endl;
    if( fileExtension == "mol2") {
        boost::scoped_ptr<Mol2> pMol2(new Mol2());
        hasSubResCoor=pMol2->calcBoundBox(subResFileName, centriod, boxDim);
        std::cout << "Average coordinates of sbustrate: " << centriod << std::endl;
    }else if(fileExtension == "pdb"){
        boost::scoped_ptr<Pdb> pPdb(new Pdb());
        hasSubResCoor=pPdb->calcBoundBox(subResFileName, centriod, boxDim);
        std::cout << "Average coordinates of sbustrate: " << centriod << std::endl;
    }
    return hasSubResCoor;
}


void minimization(JobInputData& jobInput, JobOutData& jobOut, std::string& checkFName, std::string& recType, std::string& libDir) {
    std::string tleapFName = recType + "_leap.in";
    std::string cmd = "";
    std::string errMesg = "";

    std::vector<std::vector<int> > ssList;
    {
        boost::scoped_ptr<Pdb> pPdb(new Pdb());
        pPdb->getDisulfide(checkFName, ssList);

        std::string stdPdbFile = recType + "_std.pdb";

        pPdb->standardlizeSS(checkFName, stdPdbFile, ssList);


    }

    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        } catch (...) {
            std::string mesg = "mmpbsa::receptor()\n\t Cannot open tleap file: " + tleapFName;
            throw LBindException(mesg);
        }

        if (jobInput.ambVersion == 16 || jobInput.ambVersion == 13) {
            tleapFile << "source leaprc.ff14SB\n";
            tleapFile << "source leaprc.phosaa10\n";
        } else {
            tleapFile << "source leaprc.ff99SB\n";
        }

        tleapFile << "source leaprc.gaff\n";

        tleapFile << "source leaprc.water.tip3p\n";

        for (unsigned int i = 0; i < jobInput.nonRes.size(); ++i) {
            std::string nonResRaw=jobInput.nonRes[i];
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

        tleapFile << "REC = loadpdb " + recType + "_std.pdb\n";

        for (unsigned int i = 0; i < ssList.size(); ++i) {
            std::vector<int> pair = ssList[i];
            if (pair.size() == 2) {
                tleapFile << "bond REC." << pair[0] << ".SG REC." << pair[1] << ".SG \n";
            }
        }

        tleapFile << "set default PBRadii mbondi2\n"
                << "saveamberparm REC " + recType + ".prmtop " + recType + ".inpcrd\n"
                << "quit\n";

        tleapFile.close();
    }

    cmd = "tleap -f " + tleapFName + " >& " + recType + "_leap.log";
    //std::cout <<cmd <<std::endl;
    errMesg = "tleap creating receptor prmtop fails";
    command(cmd, errMesg);

    std::string minFName = recType + "_minGB.in";
    {
        std::ofstream minFile;
        try {
            minFile.open(minFName.c_str());
        } catch (...) {
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
                << "  cut    = 15,\n"
                << "  ntr=1,\n"
                << "  restraint_wt=5.0,\n"
                << "  restraintmask='!@H=',\n"
                << " /\n" << std::endl;
        minFile.close();
    }
    if (jobInput.ambVersion == 13) {
        cmd = "sander13 -O -i " + recType + "_minGB.in -o " + recType + "_minGB.out  -p " + recType + ".prmtop -c " + recType + ".inpcrd -ref " + recType + ".inpcrd -x " + recType + ".mdcrd -r " + recType + "_min.rst";
    } else {
        cmd = "sander -O -i " + recType + "_minGB.in -o " + recType + "_minGB.out  -p " + recType + ".prmtop -c " + recType + ".inpcrd -ref " + recType + ".inpcrd -x " + recType + ".mdcrd -r " + recType + "_min.rst";
    }
    //std::cout <<cmd <<std::endl;
    errMesg = "sander receptor minimization fails";
    command(cmd, errMesg);

    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    std::string sanderOut = recType + "_minGB.out";
    double recGBen = 0;
    bool success = pSanderOutput->getEnergy(sanderOut, recGBen);
    jobOut.gbEn = recGBen;

    if (!success) {
        std::string message = "Receptor GB minimization fails.";
        throw LBindException(message);
    }

    if (jobInput.ambVersion == 16) {
        cmd = "ambpdb -p " + recType + ".prmtop -aatm -c " + recType + "_min.rst > " + recType + "_min_0.pdb";
    } else {
        cmd = "ambpdb -p " + recType + ".prmtop -aatm < " + recType + "_min.rst > " + recType + "_min_0.pdb";
    }

    //std::cout <<cmd <<std::endl;
    errMesg = "ambpdb converting rst to " + recType + "_min_0.pdb file fails";
    command(cmd, errMesg);

    checkFName = recType + "_min_0.pdb";
    if (!fileExist(checkFName)) {
        std::string message = checkFName + " does not exist.";
        throw LBindException(message);
    }

    cmd = "grep -v END " + recType + "_min_0.pdb > " + recType + "_min_orig.pdb ";
    //std::cout <<cmd <<std::endl;
    errMesg = "grep " + recType + "_min_0.pdb fails";
    command(cmd, errMesg);

    if (jobInput.ambVersion == 16) {
        cmd = "ambpdb -p " + recType + ".prmtop -c " + recType + "_min.rst > " + recType + "_min_1.pdb";
    } else {
        cmd = "ambpdb -p " + recType + ".prmtop < " + recType + "_min.rst > " + recType + "_min_1.pdb";
    }

    //std::cout <<cmd <<std::endl;
    errMesg = "ambpdb converting rst to " + recType + "_min_1.pdb file fails";
    command(cmd, errMesg);

    cmd="ln -sf "+recType + "_min_orig.pdb rec_min.pdb";
    errMesg="ln rec_min.pdb fails for"+recType + "_min_orig.pdb";
    command(cmd, errMesg);  
}

void preReceptor(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir, std::string& inputDir, std::string& dataPath){

    try{
        chdir(workDir.c_str());

        if(jobInput.dirBuffer.find('/')==0){
            jobOut.pdbFilePath=jobInput.dirBuffer;
        } else{
            jobOut.pdbFilePath=inputDir+"/"+jobInput.dirBuffer;
        }

        if(jobInput.subRes.size()!=0){
            if(jobInput.subRes.find('/')==0) {
                jobOut.subRes = jobInput.subRes;
            }else{
                jobOut.subRes = inputDir + "/" + jobInput.subRes;
            }
        }else{
            jobOut.subRes="";
        }
        jobOut.nonRes=jobInput.nonRes;

        getFileBasename(jobOut.pdbFilePath, jobOut.pdbid);
        std::string recDir=workDir+"/scratch/rec/"+jobOut.pdbid;
        jobOut.recPath=recDir;

        if(!fileExist(jobOut.pdbFilePath)){
            std::string mesg="CDT1Receptor::preReceptors: PDB file "+jobOut.pdbFilePath+" does NOT exist.";
            throw LBindException(mesg);   
        }
    
        std::string libDir=inputDir+"/lib/";
        std::string cmd="mkdir -p "+recDir;
        std::string errMesg="mkdir recDir fails";
        command(cmd, errMesg);

        cmd="cp "+jobOut.pdbFilePath+" "+recDir;
        errMesg="cp receptor pdb file fails";
        command(cmd,errMesg); 

        // cd to the rec directory to perform calculation
        chdir(recDir.c_str());

        std::string pdbFile;
        getPathFileName(jobOut.pdbFilePath, pdbFile);

        std::string checkFName="";
        if(jobInput.protonateFlg){
            boost::scoped_ptr<Pdb> pPdb(new Pdb() );
            pPdb->selectAForm(pdbFile, "rec_AForm.pdb");
             //! begin energy minimization of receptor 
            cmd="reduce -Quiet -Trim  rec_AForm.pdb >& rec_noh.pdb ";
            //std::cout <<cmd <<std::endl;
            errMesg="reduce converting rec_AForm.pdb fails";
            command(cmd,errMesg); 

            checkFName="rec_noh.pdb";
            if(!fileExist(checkFName)){
                std::string message=checkFName+" does not exist.";
                throw LBindException(message);       
            }     

            if(jobInput.ambVersion==16 || jobInput.ambVersion==13){
                cmd="reduce -Quiet -BUILD rec_noh.pdb -DB \""+dataPath+"/amber16_reduce_wwPDB_het_dict.txt\" >& rec_rd.pdb";
            }else{
                cmd="reduce -Quiet -BUILD rec_noh.pdb -DB \""+dataPath+"/amber10_reduce_wwPDB_het_dict.txt\" >& rec_rd.pdb";
            }

            //std::cout <<cmd <<std::endl;
            errMesg="reduce converting rec_noh.pdb fails";
            command(cmd,errMesg); 


            checkFName="rec_rd.pdb";
            if(!fileExist(checkFName)){
                std::string message=checkFName+" does not exist.";
                throw LBindException(message);         
            }  
        }else{
           checkFName=pdbFile;
        }

        std::string b4pdbqt=checkFName;

        if(jobInput.minimizeFlg){
            std::string recType="rec";
            minimization(jobInput, jobOut, checkFName, recType, libDir);
            b4pdbqt="rec_min_0.pdb";
        }

        {
            boost::scoped_ptr<Pdb> pPdb(new Pdb() );
            pPdb->standardlize(b4pdbqt, "std4pdbqt.pdb");
            //cmd="prepare_receptor4.py -r "+b4pdbqt+" -o "+jobOut.pdbid+".pdbqt";
            cmd="obabel -ipdb std4pdbqt.pdb -opdbqt -xr -O temp.pdbqt >& pdbqt.log";
            errMesg="obabel converting std4pdbqt.pdb  temp.pdbqt to fails";
            command(cmd,errMesg);  
            cmd="grep -v REMARK temp.pdbqt > rec_min.pdbqt";
            errMesg="grep to remove REMARK fails";
            command(cmd,errMesg);  

        }

        checkFName="rec_min.pdbqt";
        if(!fileExist(checkFName)){
            std::string message=checkFName+" does not exist.";
            throw LBindException(message);         
        } 


        // Skip the site calculation
        if(!jobInput.siteFlg){
            jobOut.error=true;
            jobOut.clust=-1; // -1 indidate skipping site calc
            jobOut.centroid=Coor3d(0,0,0);
            jobOut.dimension=Coor3d(0,0,0);
            return;
        }

        if(jobInput.dockBX.size()==2){

            for(int i=0; i<jobInput.dockBX.size(); i++){
                std::vector<std::string> tokens;
                tokenize(jobInput.dockBX[i], tokens, ",");
                std::vector<double> xyz;
                for(int j=0; j<tokens.size(); j++){
                    xyz.push_back(Sstrm<double, std::string>(tokens[j]));
                }
                if(xyz.size()==3) {
                    if (i == 0) {
                        jobOut.centroid=Coor3d(xyz[0], xyz[1], xyz[2]);
                    }
                    if (i == 1) {
                        jobOut.dimension=Coor3d(xyz[0], xyz[1], xyz[2]);
                    }
                }else{
                    jobOut.error=false;
                    return;
                }

            }

            jobOut.clust=0; // 0 indicate user define box
            jobOut.error=true;
            return;
        }

        if(true) {
            Coor3d centroid;
            Coor3d boxDim;
            getSiteFromLigand(jobOut, centroid, boxDim);
        }
        // Get geometry
        std::string stdPDBfile="rec_std.pdb"; 
        if(!jobInput.minimizeFlg){
            stdPDBfile=b4pdbqt;
        }
        boost::scoped_ptr<Complex> pComplex(new Complex());
        boost::scoped_ptr<Pdb> pPdb(new Pdb());
        pPdb->parse(stdPDBfile, pComplex.get());

        boost::scoped_ptr<ParmContainer> pParmContainer(new ParmContainer());
        ElementContainer* pElementContainer = pParmContainer->addElementContainer();
        pComplex->assignElement(pElementContainer);

        boost::scoped_ptr<Surface> pSurface(new Surface(pComplex.get()));
        std::cout << "Start Calculation " << std::endl;
        pSurface->run(jobInput.radius, jobInput.surfSphNum);
        std::cout << " Total SASA is: " << pSurface->getTotalSASA() << std::endl << std::endl;

        boost::scoped_ptr<Grid> pGrid(new Grid(pComplex.get(), true));
        pGrid->setSpacing(jobInput.spacing);
        pGrid->setCutoffCoef(jobInput.cutoffCoef);
        pGrid->setBoxExtend(jobInput.boxExtend);
        pGrid->run(jobInput.radius, jobInput.gridSphNum, jobInput.minVol);


        // Calculate the average coordinates for identify active site cavity
        // Priority 1. Crystal substrate 2. Key residues 3. Top volume
        Coor3d aveKeyResCoor;
        Coor3d boxDim;

        bool hasSubResCoor=false;
        if(fileExist(jobOut.subRes)){
            hasSubResCoor=getSiteFromLigand(jobOut, aveKeyResCoor, boxDim);
        }

        bool hasKeyResCoor=false;
        if(!hasSubResCoor){
            hasKeyResCoor=pPdb->aveKeyResCoor(pdbFile, jobInput.keyRes, aveKeyResCoor);
            if(hasKeyResCoor){
                std::cout << "Average coordinates of key residues: " << aveKeyResCoor << std::endl;
            }
        }

        Coor3d dockDim;
        Coor3d centroid; 

        bool hasKeyDockGeo=false;
        if(hasKeyResCoor || hasSubResCoor){
            hasKeyDockGeo=pGrid->getKeySiteGeo(aveKeyResCoor, dockDim, centroid, jobOut.volume);
        }

        if(!hasKeyDockGeo){
            pGrid->getTopSiteGeo(dockDim, centroid, jobOut.volume);
        }

        jobOut.clust=pGrid->getSiteIndex()+1; // Cluster print out index start with 1
        jobOut.centroid=centroid;
        jobOut.dimension=dockDim;

        std::ofstream outFile;
        outFile.open("rec_geo.txt");
        outFile << centroid.getX() << " " << centroid.getY() << " " << centroid.getZ() << " " 
                 << dockDim.getX() << " " << dockDim.getY() << " "  << dockDim.getZ() << "\n";
        outFile.close();
        
        if(jobInput.cutProt){
            jobOut.cutProt=jobInput.cutProt;
            std::string fileName="recCut.pdb";
            pGrid->writeCutRecPDB(fileName, pComplex.get(), jobInput.cutRadius);
            std::string recType="recCut";
            minimization(jobInput, jobOut, fileName, recType, libDir);           
        }

        chdir(workDir.c_str());
        //delete pElementContainer;
    
    } catch (LBindException& e){
        jobOut.message=e.what();
        jobOut.error=false;
        return;
    }

    //END
    jobOut.error=true;
    return;
}


void saveStrList(std::string& fileName, std::vector<RecData*>& strList){
    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Cannot open file: " << fileName << std::endl;
    } 

    const std::string comment="#";
    std::string fileLine;
    while(inFile){
        std::getline(inFile, fileLine);
        if(fileLine.compare(0, 1, comment)==0) continue;
        std::vector<std::string> tokens;
        tokenize(fileLine, tokens); 
        if(tokens.size() > 0){
            RecData* pRecData=new RecData();
            pRecData->pdbFile=tokens[0];
            for(unsigned i=1; i<tokens.size(); ++i){
                std::string str=tokens[i];
                std::string flg=str.substr(0, 6);
                std::string strline=str.substr(7,str.size()-7);
                std::vector<std::string> strTokens;
                tokenize(strline, strTokens, "|");
                if(flg=="KeyRes"){                    
                    pRecData->keyRes=strTokens;
                }else if(flg=="NonRes"){
                    pRecData->nonRes=strTokens;
                }else if(flg=="SubRes"){
                    pRecData->subRes=strTokens[0];
                }else if(flg=="DockBX"){
                    pRecData->dockBX=strTokens;
                }
            }
            strList.push_back(pRecData);
        }        
    }
    
}

void initInputData(JobInputData& jobInput, POdata& podata){
    // Turn on/off protonate procedure
    if(podata.protonateFlg=="on"){
        jobInput.protonateFlg=true;
    }else{
        jobInput.protonateFlg=false;
    }

    if(podata.minimizeFlg=="on"){
        jobInput.minimizeFlg=true;
    }else{
        jobInput.minimizeFlg=false;
    }

    if(podata.siteFlg=="on"){
        jobInput.siteFlg=true;
    }else{
        jobInput.siteFlg=false;
    }

    if(podata.forceRedoFlg=="on"){
        jobInput.forceRedoFlg=true;
    }else{
        jobInput.forceRedoFlg=false;
    }

    if(podata.cutProt=="on"){
        jobInput.cutProt=true;
    }else{
        jobInput.cutProt=false;
    }

    jobInput.ambVersion=podata.version;
    jobInput.surfSphNum=podata.surfSphNum;
    jobInput.gridSphNum=podata.gridSphNum;
    jobInput.radius=podata.radius;
    jobInput.spacing=podata.spacing;
    jobInput.cutoffCoef=podata.cutoffCoef;
    jobInput.boxExtend=podata.boxExtend;
    jobInput.minVol=podata.minVol;
    jobInput.cutRadius=podata.cutRadius;
}


int main(int argc, char** argv) {

    // ! start MPI parallel
    
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


    POdata podata;

    if (world.rank() == 0) {        
        bool success=CDT1ReceptorPO(argc, argv, podata);
        if(!success){
            world.abort(1);
        }        
    }
    
    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        world.abort(1);
    }

    std::cout << "Number of tasks= " << world.size() << " My rank= " << world.rank() << std::endl;

    JobInputData jobInput;
    JobOutData jobOut;

    if (world.rank() == 0) {

        // if force re-do just delete the receptor.hdf5
        if (jobInput.forceRedoFlg) {
            std::string cmd = "rm -f " + workDir + "/scratch/receptor.hdf5";
            std::string errMesg = "Remove fails for " + workDir + "/scratch/receptor.hdf5";
            command(cmd, errMesg);
        }
        //! Open a Conduit file to track the calculation
        std::string cmd = "mkdir -p " + workDir + "/scratch";
        std::string errMesg = "mkdir scratch directory fails";
        LBIND::command(cmd, errMesg);

        Node n;
        std::string recCdtFile = workDir + "/scratch/receptor.hdf5:/";
        n["date"] = "Create By CDT1Receptor at " + timeStamp();
        relay::io::hdf5_append(n, recCdtFile);

        jobInput.recCdtFile=recCdtFile;

        initInputData(jobInput, podata);

        std::vector<RecData*> dirList;        
        
        std::string pdbList=inputDir+"/"+podata.inputFile;

        saveStrList(pdbList, dirList);
        int count=0;
           
        for(unsigned i=0; i<dirList.size(); ++i){
            jobInput.dirBuffer=dirList[i]->pdbFile;
            //For restart
            // If force re-do the calculation skip check the hdf5 file and continue calculation (by default not force re-do)
            if(!jobInput.forceRedoFlg){
                if(isRun(jobInput)){
                    continue;
                }
            }

            ++count;
            
            if(count >world.size()-1){
                world.recv(mpi::any_source, outTag, jobOut);

                toConduit(jobOut, recCdtFile);
                if(jobOut.error) {
                    rmRecDir(jobOut);
                }
            }   
            
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            std::cout << "At Process: " << freeProc << " working on: " << dirList[i]->pdbFile  << std::endl;
            world.send(freeProc, jobTag, jobFlag);

            jobInput.keyRes=dirList[i]->keyRes;
            jobInput.nonRes=dirList[i]->nonRes;
            jobInput.subRes=dirList[i]->subRes;
            jobInput.dockBX=dirList[i]->dockBX;

            world.send(freeProc, inpTag, jobInput);            
        }

        int nJobs=count;
        int nWorkers=world.size()-1;
        int ndata=(nJobs<nWorkers)? nJobs: nWorkers;

        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(int i=0; i < ndata; ++i){
            world.recv(mpi::any_source, outTag, jobOut);

            toConduit(jobOut, recCdtFile);
            if(jobOut.error) {
                rmRecDir(jobOut);
            }
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
            // Receive parameters

            world.recv(0, inpTag, jobInput);
                                    
            jobOut.message="Finished!";

            preReceptor(jobInput, jobOut, workDir, inputDir, dataPath);

            world.send(0, outTag, jobOut);
           
        }
    }

    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;

    return 0;
}

