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

#include "Parser/Pdb.h"
#include "Parser/SanderOutput.h"
#include "MM/VinaLC.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Coor3d.h"
#include "Structure/Constants.h"
#include "Structure/Complex.h"
#include "Structure/Atom.h"
#include "Common/LBindException.h"
#include "Common/Tokenize.hpp"
#include "BackBone/Surface.h"
#include "BackBone/Grid.h"
#include "Structure/ParmContainer.h"
#include "Common/File.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"

#include "PPL1ReceptorPO.h"

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

namespace mpi = boost::mpi;
using namespace LBIND;

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

    srun -N4 -n48 -ppdebug /g/g92/zhang30/medchem/NetBeansProjects/MedCM/apps/mmpbsa/PPL1Receptor  --input pdb.list --output out 

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

class JobInputData{
    
public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
       ar & protonateFlg;
        ar & getPDBflg;
        ar & dirBuffer;  
        ar & keyRes;
        ar & nonRes;
    }
    
    bool protonateFlg;
    bool getPDBflg;
    std::string dirBuffer;
    std::vector<std::string> keyRes;
    std::vector<std::string> nonRes;
};

//struct JobInputData{ 
//    bool getPDBflg;
//    char dirBuffer[100];
//};

class JobOutData{
    
public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {        
        ar & error;
        ar & volume;
        ar & gbEn; 
        ar & centroid;
        ar & dimension;  
        ar & pdbid;
        ar & pdbFilePath;
        ar & recPath;
        ar & message; 
        ar & nonRes;
       
    }
    
    bool error;
    int volume;
    double gbEn;
    Coor3d centroid;
    Coor3d dimension;
    std::string pdbid;
    std::string pdbFilePath;
    std::string recPath;
    std::string message;
    std::vector<std::string> nonRes;
   
};

//struct JobOutData{  
//    bool error;
//    char dirBuffer[100];
//    char message[100];
//};

void toXML(JobOutData& jobOut, XMLElement* root, FILE* xmlTmpFile){
        XMLElement * element = new XMLElement("Receptor");
        
        XMLElement * pdbidEle = new XMLElement("RecID");
        XMLText * pdbidTx= new XMLText(jobOut.pdbid.c_str()); // has to use c-style string.
        pdbidEle->LinkEndChild(pdbidTx);
        element->LinkEndChild(pdbidEle);

        XMLElement * pdbPathEle = new XMLElement("PDBPath");
        XMLText * pdbPathTx= new XMLText(jobOut.pdbFilePath.c_str());
        pdbPathEle->LinkEndChild(pdbPathTx);        
        element->LinkEndChild(pdbPathEle);
        
        XMLElement * nonstdAAsEle = new XMLElement("NonStdAAList");
        for (unsigned i = 0; i < jobOut.nonRes.size(); ++i) {
            std::string iStr=Sstrm<std::string, unsigned>(i+1);
            XMLElement * scEle = new XMLElement("NonStdAA");
            scEle->SetAttribute("id", iStr.c_str() );
            XMLText * scTx = new XMLText(jobOut.nonRes[i].c_str());
            scEle->LinkEndChild(scTx);
            nonstdAAsEle->LinkEndChild(scEle);
        }
        element->LinkEndChild(nonstdAAsEle);        

        XMLElement * recPathEle = new XMLElement("PDBQTPath");
        XMLText * recPathTx= new XMLText(jobOut.recPath.c_str());
        recPathEle->LinkEndChild(recPathTx);        
        element->LinkEndChild(recPathEle);
        
        XMLElement * gbEle = new XMLElement("GBEN");
        XMLText * gbTx= new XMLText(Sstrm<std::string, double>(jobOut.gbEn));
        gbEle->LinkEndChild(gbTx);        
        element->LinkEndChild(gbEle);        
        
        XMLElement * siteEle = new XMLElement("Site");
        element->LinkEndChild(siteEle); 
        
        XMLElement * volEle = new XMLElement("Volume");
        XMLText * volTx= new XMLText(Sstrm<std::string, int>(jobOut.volume));
        volEle->LinkEndChild(volTx);         
        siteEle->LinkEndChild(volEle);  
        
        XMLElement * centEle = new XMLElement("Centroid");
        siteEle->LinkEndChild(centEle);  

        XMLElement * xcEle = new XMLElement("X");
        XMLText * xcTx= new XMLText(Sstrm<std::string, double>(jobOut.centroid.getX() ));
        xcEle->LinkEndChild(xcTx);          
        centEle->LinkEndChild(xcEle);         

        XMLElement * ycEle = new XMLElement("Y");
        XMLText * ycTx= new XMLText(Sstrm<std::string, double>(jobOut.centroid.getY() ));
        ycEle->LinkEndChild(ycTx);        
        centEle->LinkEndChild(ycEle);         

        XMLElement * zcEle = new XMLElement("Z");
        XMLText * zcTx= new XMLText(Sstrm<std::string, double>(jobOut.centroid.getZ() ));
        zcEle->LinkEndChild(zcTx); 
        centEle->LinkEndChild(zcEle);         
        
        XMLElement * DimEle = new XMLElement("Dimension");
        siteEle->LinkEndChild(DimEle);  

        XMLElement * xdEle = new XMLElement("X");
        XMLText * xdTx= new XMLText(Sstrm<std::string, double>(jobOut.dimension.getX() ));
        xdEle->LinkEndChild(xdTx);          
        DimEle->LinkEndChild(xdEle);         

        XMLElement * ydEle = new XMLElement("Y");
        XMLText * ydTx= new XMLText(Sstrm<std::string, double>(jobOut.dimension.getY() ));
        ydEle->LinkEndChild(ydTx);  
        DimEle->LinkEndChild(ydEle);         

        XMLElement * zdEle = new XMLElement("Z");
        XMLText * zdTx= new XMLText(Sstrm<std::string, double>(jobOut.dimension.getZ() ));
        zdEle->LinkEndChild(zdTx);  
        DimEle->LinkEndChild(zdEle);         
        
        XMLElement * mesgEle = new XMLElement("Mesg");
        XMLText * mesgTx= new XMLText(jobOut.message);
        mesgEle->LinkEndChild(mesgTx);          
        element->LinkEndChild(mesgEle);          
        
//        element->SetAttribute(std::string("item"), jobOut.pdbid);
//        element->SetAttribute(std::string("mesg"), jobOut.message);  
        root->LinkEndChild(element);

        element->Print(xmlTmpFile,1);
        fputs("\n",xmlTmpFile);
        fflush(xmlTmpFile);     
}


bool preReceptor(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir, std::string& dataPath){
    
    bool jobStatus=false;
    
    chdir(workDir.c_str());
    jobOut.pdbFilePath=jobInput.dirBuffer; 
    jobOut.nonRes=jobInput.nonRes;
    if(!fileExist(jobOut.pdbFilePath)){
        std::string mesg="PPL1Receptor::preReceptors: PDB file "+jobOut.pdbFilePath+" does NOT exist.";
        throw LBindException(mesg);  
        return jobStatus; 
    }
    
//    std::string pdbBasename;
    getFileBasename(jobOut.pdbFilePath, jobOut.pdbid);
        
    std::string recDir=workDir+"/scratch/com/"+jobOut.pdbid+"/rec";
    std::string libDir=workDir+"/lib/";
    std::string cmd="mkdir -p "+recDir;
    system(cmd.c_str());  
    
    cmd="cp "+jobOut.pdbFilePath+" "+recDir;
    system(cmd.c_str());  
    
    // cd to the rec directory to perform calculation
    chdir(recDir.c_str());
    
    std::string pdbFile;
    getPathFileName(jobOut.pdbFilePath, pdbFile);
      
    std::string checkFName="";
    if(jobInput.protonateFlg){
         //! begin energy minimization of receptor 
        cmd="reduce -Quiet -Trim  "+pdbFile+" >& rec_noh.pdb ";
        std::cout <<cmd <<std::endl;
        system(cmd.c_str());

        checkFName="rec_noh.pdb";
        if(!fileExist(checkFName)){
            std::string message=checkFName+" does not exist.";
            throw LBindException(message);  
            return jobStatus;        
        }     

        cmd="reduce -Quiet -BUILD rec_noh.pdb -DB \""+dataPath+"/reduce_wwPDB_het_dict.txt\" >& rec_rd.pdb";
        std::cout <<cmd <<std::endl;
        system(cmd.c_str()); 


        checkFName="rec_rd.pdb";
        if(!fileExist(checkFName)){
            std::string message=checkFName+" does not exist.";
            throw LBindException(message); 
            return jobStatus;        
        }  
    }else{
       checkFName=pdbFile;
    }
    
    std::vector<std::vector<int> > ssList;
    {
        boost::scoped_ptr<Pdb> pPdb(new Pdb() );
        pPdb->getDisulfide(checkFName, ssList);
        
        std::string stdPdbFile="rec_std.pdb";
        
        pPdb->standardlizeSS(checkFName, stdPdbFile, ssList);
        

    }
        
    std::string tleapFName="rec_leap.in";
    
    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="mmpbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }
        
        tleapFile << "source leaprc.protein.ff14SB\n"                
                  << "source leaprc.gaff\n";
        
        for(unsigned int i=0; i<jobInput.nonRes.size(); ++i){
            tleapFile << "loadoff "<< libDir << jobInput.nonRes[i] <<".off \n";
            tleapFile << "loadamberparams "<< libDir << jobInput.nonRes[i] <<".frcmod \n";
        }
        
        tleapFile << "REC = loadpdb rec_std.pdb\n";
               
        for(unsigned int i=0; i<ssList.size(); ++i){
            std::vector<int> pair=ssList[i];
            if(pair.size()==2){
                tleapFile << "bond REC."<< pair[0] <<".SG REC." << pair[1] <<".SG \n";
            }
        }        
                
        tleapFile << "saveamberparm REC REC.prmtop REC.inpcrd\n"
                  << "quit\n";
        
        tleapFile.close();
    }
    
    cmd="tleap -f rec_leap.in >& rec_leap.log";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());

    std::string minFName="Rec_minGB.in";
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
                << "  ntr=1,\n" 
                << "  restraint_wt=5.0,\n" 
                << "  restraintmask='!@H=',\n"        
                << " /\n" << std::endl;
        minFile.close();    
    }
    
    cmd="sander -O -i Rec_minGB.in -o Rec_minGB.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    std::string sanderOut="Rec_minGB.out";
    double recGBen=0;
    bool success=pSanderOutput->getEnergy(sanderOut, recGBen);
    jobOut.gbEn=recGBen;
    
    if(!success){
        std::string message="Receptor GB minimization fails.";
        throw LBindException(message); 
        return jobStatus;          
    }
    
       
    cmd="ambpdb -p REC.prmtop -aatm < Rec_min.rst > Rec_min_0.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  

    checkFName="Rec_min_0.pdb";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        return jobStatus;        
    }  
    
    cmd="grep -v END Rec_min_0.pdb > Rec_min.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    cmd="ambpdb -p REC.prmtop < Rec_min.rst > Rec_min_1.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());     
    
    cmd="prepare_receptor4.py -r Rec_min_1.pdb -o "+jobOut.pdbid+".pdbqt";
    system(cmd.c_str());
    
    checkFName=jobOut.pdbid+".pdbqt";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message);  
        return jobStatus;        
    } 
    jobOut.recPath="scratch/com/"+jobOut.pdbid+"/rec/"+jobOut.pdbid+".pdbqt";
    
    // Get geometry
    std::string stdPDBfile="rec_std.pdb";    
    boost::scoped_ptr<Complex> pComplex(new Complex());
    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    pPdb->parse(stdPDBfile, pComplex.get());

    boost::scoped_ptr<ParmContainer> pParmContainer(new ParmContainer());
    ElementContainer* pElementContainer = pParmContainer->addElementContainer();
    pComplex->assignElement(pElementContainer);

    boost::scoped_ptr<Surface> pSurface(new Surface(pComplex.get()));
    std::cout << "Start Calculation " << std::endl;
    pSurface->run(1.4, 960);
    std::cout << " Total SASA is: " << pSurface->getTotalSASA() << std::endl << std::endl;

    boost::scoped_ptr<Grid> pGrid(new Grid(pComplex.get(), false));
    pGrid->run(1.4, 100, 50);
    
    Coor3d aveKeyResCoor;
    bool hasKeyResCoor=pPdb->aveKeyResCoor(pdbFile, jobInput.keyRes, aveKeyResCoor);
 
    Coor3d dockDim;
    Coor3d centroid; 
    
    bool hasKeyDockGeo=false;
    if(hasKeyResCoor){
        hasKeyDockGeo=pGrid->getKeySiteGeo(aveKeyResCoor, dockDim, centroid, jobOut.volume);
    }
    
    if(!hasKeyDockGeo){
        pGrid->getTopSiteGeo(dockDim, centroid, jobOut.volume);
    }
    
    jobOut.centroid=centroid;
    jobOut.dimension=dockDim;
    
    std::ofstream outFile;
    outFile.open("rec_geo.txt");
    outFile << centroid.getX() << " " << centroid.getY() << " " << centroid.getZ() << " " 
             << dockDim.getX() << " " << dockDim.getY() << " "  << dockDim.getZ() << "\n";
    outFile.close();
    //delete pElementContainer;
    
    //END    
    jobStatus=true;
    return jobStatus;
}

struct RecData{
    std::string pdbFile;
    std::vector<std::string> keyRes; // key residues to help locate binding site
    std::vector<std::string> nonRes; //non-standard residue list
};

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
                }
            }
            strList.push_back(pRecData);
        }        
    }
    
}


int main(int argc, char** argv) {
 
    //! get  working directory
    char* WORKDIR=getenv("WORKDIR");
    std::string workDir;
    if(WORKDIR==0) {
        // use current working directory for working directory
        char BUFFER[200];
        getcwd(BUFFER, sizeof (BUFFER));
        workDir = BUFFER;        
    }else{
        workDir=WORKDIR;
    }
    //! get LBindData
    char* LBINDDATA=getenv("LBindData");

    if(LBINDDATA==0){
        std::cerr << "LBdindData environment is not defined!" << std::endl;
        return 1;
    }    
    std::string dataPath=LBINDDATA;     
           
    // ! start MPI parallel
    
    int jobFlag=1; // 1: doing job,  0: done job
    
    JobInputData jobInput;
    JobOutData jobOut;
            
    int rankTag=1;
    int jobTag=2;

    int inpTag=3;
    int outTag=4;

    mpi::environment env(argc, argv);
    mpi::communicator world;    

    mpi::timer runingTime;
//    int error=0;
    
    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
//        error=1;
        return 1;
    }
    
    POdata podata;
    
    
    if (world.rank() == 0) {        
        bool success=PPL1ReceptorPO(argc, argv, podata);
        if(!success){
//            error=1; 
            return 1;
        }        
    }

//    MPI_Bcast(&error, 1, MPI_INT, 0, MPI_COMM_WORLD);
//    mpi::broadcast<int>(world, error, 1);
//    if (error !=0) {
//        MPI_Finalize();
//        return 1;
//    }     

    std::cout << "Number of tasks= " << world.size() << " My rank= " << world.rank() << std::endl;

    if (world.rank() == 0) {
                        
        //! Tracking error using XML file
	XMLDocument doc;  
 	XMLDeclaration* decl = new XMLDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	XMLElement * root = new XMLElement( "Receptors" );  
	doc.LinkEndChild( root );  

	XMLComment * comment = new XMLComment();
	comment->SetValue(" Tracking calculation error using XML file " );  
	root->LinkEndChild( comment );     
        
        
        FILE* xmlTmpFile=fopen("PPL1TrackTemp.xml", "w");
        fprintf(xmlTmpFile, "<?xml version=\"1.0\" ?>\n");
        fprintf(xmlTmpFile, "<Receptors>\n");
        fprintf(xmlTmpFile, "    <!-- Tracking calculation error using XML file -->\n");
//        root->Print(xmlTmpFile, 0);
//        fputs("\n",xmlTmpFile);
        fflush(xmlTmpFile);        
        //! END of XML header   
        
        // Turn on/off protonate procedure
        if(podata.protonateFlg=="on"){
           jobInput.protonateFlg=true;
        }else{
           jobInput.protonateFlg=false;
        }
       
        std::vector<RecData*> dirList;        
        
        saveStrList(podata.inputFile, dirList);
        int count=0;
           
        for(unsigned i=0; i<dirList.size(); ++i){
            ++count;
            
            if(count >world.size()-1){
//                MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
                world.recv(mpi::any_source, outTag, jobOut);
                toXML(jobOut, root, xmlTmpFile);                
            }   
            
            int freeProc;
//            MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
            world.recv(mpi::any_source, rankTag, freeProc);
            std::cout << "At Process: " << freeProc << " working on: " << dirList[i]->pdbFile  << std::endl;            
//            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 
            world.send(freeProc, jobTag, jobFlag);

//            strcpy(jobInput.dirBuffer, dirList[i].c_str());
            jobInput.dirBuffer=dirList[i]->pdbFile;
            jobInput.keyRes=dirList[i]->keyRes;
            jobInput.nonRes=dirList[i]->nonRes;

//            MPI_Send(&jobInput, sizeof(JobInputData), MPI_CHAR, freeProc, inpTag, MPI_COMM_WORLD);
            world.send(freeProc, inpTag, jobInput);            
        }

        int nJobs=count;
        int ndata=(nJobs<world.size()-1)? nJobs: world.size()-1;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(int i=0; i < ndata; ++i){
//            MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
            world.recv(mpi::any_source, outTag, jobOut);
            toXML(jobOut, root, xmlTmpFile);  
        } 

        fprintf(xmlTmpFile, "</Receptors>\n");        
        doc.SaveFile( "PPL1Track.xml" );        
        
        for(int i=1; i < world.size(); ++i){
            int freeProc;
//            MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;;
//            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 
            world.send(freeProc, jobTag, jobFlag);
        }        
        
    }else {
        while (1) {
//            MPI_Send(&rank, 1, MPI_INTEGER, 0, rankTag, MPI_COMM_WORLD);
            world.send(0, rankTag, world.rank());
//            MPI_Recv(&jobFlag, 20, MPI_CHAR, 0, jobTag, MPI_COMM_WORLD, &status2);
            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }
            // Receive parameters

            world.recv(0, inpTag, jobInput);
//            MPI_Recv(&jobInput, sizeof(JobInputData), MPI_CHAR, 0, inpTag, MPI_COMM_WORLD, &status1);
                                    
            jobOut.message="Finished!";
//            strcpy(jobOut.message, "Finished!");
            try{
                jobOut.error=preReceptor(jobInput, jobOut, workDir, dataPath);            
            } catch (LBindException& e){
//                std::string message= e.what();  
//                strcpy(jobOut.message, message.c_str());
                jobOut.message=e.what();
            }            
            
//            MPI_Send(&jobOut, sizeof(JobOutData), MPI_CHAR, 0, outTag, MPI_COMM_WORLD);    
            world.send(0, outTag, jobOut);
           
        }
    }


    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;
    
    
    return 0;
}

