/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PPL2VolumeFilter.cpp
 * Author: zhang30
 *
 * Created on March 27, 2018, 11:58 AM
 */

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include "Parser/Sdf.h"
#include "Parser/Pdb.h"
#include "MM/Amber.h"
#include "Parser/SanderOutput.h" 
#include "Structure/Sstrm.hpp"
#include "Structure/Constants.h"
#include "Structure/Molecule.h"
#include "Structure/ElementContainer.h"
#include "Structure/Element.h"
#include "Structure/ParmContainer.h"
#include "Parser/Mol2.h"
#include "Common/File.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"

#include "PPL2VolumeFilterPO.h"

#include <boost/scoped_ptr.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace mpi = boost::mpi;

using namespace LBIND;

class JobInputData{
    
public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & protonate;
        ar & ligID;
        ar & sdfBuffer;        
    }
    
    bool protonate;
    int ligID;
    std::string sdfBuffer;
};

//struct JobInputData{        
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
        ar & protonate;
        ar & ligID;
        ar & volume;
        ar & ligName;       
        ar & message; 
        ar & sdfBuffer;
       
    }
    
    bool error;
    bool protonate;
    int ligID;
    double volume;
    std::string ligName;   
    std::string message;
    std::string sdfBuffer;
   
};

void toXML(JobOutData& jobOut, XMLElement* root, FILE* xmlTmpFile){
        XMLElement * element = new XMLElement("Ligand");
        
        XMLElement * pdbidEle = new XMLElement("LigID");
        XMLText * pdbidTx= new XMLText(Sstrm<std::string, int>(jobOut.ligID)); 
        pdbidEle->LinkEndChild(pdbidTx);
        element->LinkEndChild(pdbidEle);

        XMLElement * ligNameEle = new XMLElement("LigName");
        XMLText * ligNameTx= new XMLText(jobOut.ligName.c_str()); // has to use c-style string.
        ligNameEle->LinkEndChild(ligNameTx);
        element->LinkEndChild(ligNameEle);                 

        XMLElement * volEle = new XMLElement("Volume");
        XMLText * volTx= new XMLText(Sstrm<std::string, double>(jobOut.volume));
        volEle->LinkEndChild(volTx);        
        element->LinkEndChild(volEle);
        
        XMLElement * mesgEle = new XMLElement("Mesg");
        XMLText * mesgTx= new XMLText(jobOut.message);
        mesgEle->LinkEndChild(mesgTx);          
        element->LinkEndChild(mesgEle);           

        root->LinkEndChild(element);

        element->Print(xmlTmpFile,1);
        fputs("\n",xmlTmpFile);
        fflush(xmlTmpFile);          
        
}

bool volumeFilter(JobOutData& jobOut, ElementContainer* pElementContainer) {
    bool jobStatus = false;

    //std::cout << jobOut.sdfBuffer << std::endl;
    std::vector<std::string> tokens;
    const std::string& delimiters = "\n";
    const bool trimEmpty = false;
    tokenize(jobOut.sdfBuffer, tokens, delimiters, trimEmpty);

    if (tokens.size() < 5) {
        throw LBindException("SDF entry incomplete");
        return jobStatus;
    }
    
    std::vector<std::string> titles;
    tokenize(tokens[0], titles); 
    
    if(titles.size()>0){
        jobOut.ligName=titles[0];
    }else{
        jobOut.ligName="NoName";
    }

    // 61 63  0  0  1  0            999 V2000 - Line 4
    std::vector<std::string> records;
    tokenize(tokens[3], records);

   //std::cout << "record |" << tokens[3] << std::endl;

    if (records.size() < 2) {
        throw LBindException("SDF count line error");
        return jobStatus;
    }
    
    
    std::map<std::string, int> mapElements;
    
    int atmNum = Sstrm<int, std::string>(records[0]);
    int bndNum = Sstrm<int, std::string>(records[1]);
    for (int i = 4; i < atmNum + 4; ++i) {
        std::vector<std::string> atomBlocks;
        tokenize(tokens[i], atomBlocks);
        if (atomBlocks.size() < 4) {
            std::string lineNum = Sstrm<std::string, int>(i);
            std::string mesg = "SDF atom block line " + lineNum + " is wrong";
            throw LBindException(mesg);
            return jobStatus;
        }
        std::string element = atomBlocks[3];
        if(mapElements.find(element) == mapElements.end()){
            mapElements[element]=1;
        }else{
            mapElements[element]+=1;
        }
    }

    double volume=0.0;
    typedef std::map<std::string, int>::iterator MapIterator;
    for(MapIterator it= mapElements.begin(); it != mapElements.end(); it++) {
        //std::cout << it->first << " :: " << it->second << std::endl;
        std::string symbol=it->first;
        int freq=it->second;
        Element *pElement=pElementContainer->symbolToElement(symbol);
        double r=pElement->getVDWRadius();
        double v=4.0/3.0*PI*r*r*r;
        volume+=freq*v;
    }    

    int hbondNum=0;
    if(mapElements.find("H") != mapElements.end()){
            hbondNum=mapElements["H"];
    }

    int heavybondNum=bndNum-hbondNum;

    if(!jobOut.protonate){ // The ligands are not protonated 
		           // estimate the number of bonds with hydrogens
	
    	int estimatedBond=0;
    	for(MapIterator it= mapElements.begin(); it != mapElements.end(); it++) {
        	std::string symbol=it->first;
        	int freq=it->second;
        	Element *pElement=pElementContainer->symbolToElement(symbol);
            if(pElement->getAtomicNumber()!=1){ 
        	estimatedBond+=freq*pElement->getCovalent();
	    }
    	}

	int estimatedH=estimatedBond/2-heavybondNum;

	if(estimatedH>0){
        	// Including volume of hydrogens for the un-protonated compounds
        	// r=1.2          4.0/3.0*math.pi*r*r*r
        	// excluding anything h count prevously
        	volume+=(estimatedH-hbondNum)*7.238229473870882; 
		hbondNum=estimatedH;
	}

    }
    //std::cout << "estimeateH = " << estimatedH<< " hydrogen " << hbondNum <<std::endl;
    if (heavybondNum < 0) {
        throw LBindException("Number of heavy bonds is negative");
        return jobStatus;
    }

    // Sphere-Sphere intersection Heavy-Heavy 7.844295  Heavy-Hydrogen 5.717829
    // R=1.7 r=1.2 d_Hv_Hv=1.5 d_Hv_H=1.0
    // Heavy-Heavy      1.0/12*math.pi*(4.0*R+d)*(2.0*R-d)**2
    // Heavy-Hydrogen   math.pi*(R+r-d)**2*(d**2+2*d*r-3*r**2+2*d*R+6*r*R-3*R**2)/12.0/d
    // Subtract the Sphere-Sphere intersection
    volume=volume-heavybondNum*7.844295 - hbondNum*5.717829;
       
    jobOut.volume=volume;
    
    return true;
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    // ! MPI Parallel

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

    POdata podata;
    int error=0;
    
    if (world.rank() == 0) {        
        bool success=PPL2VolumeFilterPO(argc, argv, podata);
        if(!success){
            error=1;           
        }        
    }

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        return 1;
    }
        
    std::cout << "Number of tasks= " << world.size() << " My rank= " << world.rank() << std::endl;    
        
    if (world.rank() == 0) {
                
//! Tracking error using XML file

	XMLDocument doc;  
 	XMLDeclaration* decl = new XMLDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	XMLElement * root = new XMLElement( "Ligands" );  
	doc.LinkEndChild( root );  

	XMLComment * comment = new XMLComment();
	comment->SetValue(" Tracking calculation error using XML file " );  
	root->LinkEndChild( comment );  
         
        FILE* xmlFile=fopen("PPL2TrackTemp.xml", "w"); 
        fprintf(xmlFile, "<?xml version=\"1.0\" ?>\n");
        fprintf(xmlFile, "<Ligands>\n");
        fprintf(xmlFile, "    <!-- Tracking calculation error using XML file -->\n");
        fflush(xmlFile);         
 //! END of XML header

// Open output file
        std::ofstream outFile;
        outFile.open(podata.outputFile.c_str());

        double volumeCutoff = podata.volume;
        // Start to read in the SDF file
        std::ifstream inFile;
        try {
            inFile.open(podata.sdfFile.c_str());
        } catch (...) {
            std::cout << "preLigands >> Cannot open file" << podata.sdfFile << std::endl;
        }

        const std::string delimter = "$$$$";
        std::string fileLine = "";
        std::string contents = "";

        int count = 0;

        while (inFile) {
            std::getline(inFile, fileLine);
            contents = contents + fileLine + "\n";
            if (fileLine.size() >= 4 && fileLine.compare(0, 4, delimter) == 0) {
                if (count > world.size() - 1) {
                    world.recv(mpi::any_source, outTag, jobOut);
                    toXML(jobOut, root, xmlFile);
                    if (jobOut.error && jobOut.volume < volumeCutoff) {
                        outFile <<jobOut.sdfBuffer;
                    }
                }
                //std::cout << contents << std::endl;

                //std::string dir=ligList[i];

                int freeProc;
                world.recv(mpi::any_source, rankTag, freeProc);
                std::cout << "At Process: " << freeProc << " working on: " << count << std::endl;
                world.send(freeProc, jobTag, jobFlag);

                jobInput.sdfBuffer = contents;
                jobInput.ligID = count;
                jobInput.protonate = podata.protonate;

                world.send(freeProc, inpTag, jobInput);

                contents = ""; //! clean up the contents for the next structure.
                count++;
            }
        }

        int nJobs = count;
        int ndata = (nJobs <= world.size() - 1) ? nJobs : world.size();
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;

        for (int i = 0; i < ndata; ++i) {
            world.recv(mpi::any_source, outTag, jobOut);
            toXML(jobOut, root, xmlFile);
            if (jobOut.error && jobOut.volume < volumeCutoff) {
                outFile <<jobOut.sdfBuffer;
            }
        } 
        
        fprintf(xmlFile, "</Ligands>\n");
        doc.SaveFile(podata.xmlOut);
        
        for(int i=1; i < world.size(); ++i){
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;;
            world.send(freeProc, jobTag, jobFlag);
        }
        
    }else {
        // READ in the element parameters
        boost::scoped_ptr<ParmContainer> pParmContainer(new ParmContainer());
        ElementContainer* pElementContainer = pParmContainer->addElementContainer();
    
        while (1) {
            world.send(0, rankTag, world.rank());
            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }
            // Receive parameters

            world.recv(0, inpTag, jobInput);
                        
            jobOut.ligID=jobInput.ligID;
            jobOut.protonate=jobInput.protonate;
            jobOut.sdfBuffer=jobInput.sdfBuffer;
            jobOut.message="Finished!";
            try{
                bool jobStatus=volumeFilter(jobOut, pElementContainer);            
                jobOut.error=jobStatus;
            } catch (LBindException& e){
                jobOut.message= e.what();  
            }
            
            world.send(0, outTag, jobOut);
        }
    }

    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;
    
    return 0;
}

