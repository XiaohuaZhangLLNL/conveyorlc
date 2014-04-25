/* 
 * File:   PPL4parseXML.cpp
 * Author: zhang
 *
 * Created on April 24, 2014, 4:36 PM
 */

#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

#include "Common/LBindException.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Constants.h"
#include "XML/XMLHeader.hpp"

using namespace std;
using namespace LBIND;

struct POdata{
    std::string inXML;
    std::string outXML;
};

using namespace boost::program_options;
/*
 * 
 */
bool PPL4parseXMLPO(int argc, char** argv, POdata& podata) {
    
    bool help;
    positional_options_description positional;
    
    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("inXML", value<std::string > (&podata.inXML), "PPL4Track XML file")
                ("outXML", value<std::string > (&podata.outXML), "Final XML ")
                ;   
        options_description info("Optional:");
        info.add_options()
                ("help", bool_switch(&help), "display usage summary")
                ;
        options_description desc;
        desc.add(inputs).add(info);        

        variables_map vm;
        try {
            store(command_line_parser(argc, argv)
                    .options(desc)
                    .style(command_line_style::default_style ^ command_line_style::allow_guessing)
                    .positional(positional)
                    .run(),
                    vm);
            notify(vm);
        } catch (boost::program_options::error& e) {
            std::cerr << "Command line parse error: " << e.what() << '\n' << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }  
        
        if (help) {
            std::cout << desc << '\n';
            return 0;
        }

        if (vm.count("inXML") <= 0) {
            std::cerr << "Missing PPL4Track.xml file.\n" << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }          

        if (vm.count("outXML") <= 0) {
            std::cerr << "Missing output xml file name.\n" << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }          

        
    }catch (boost::filesystem::filesystem_error& e) {
        std::cerr << "\n\nFile system error: " << e.what() << '\n';
        return false;
    }catch (...) {
        std::cerr << "\n\nAn unknown error occurred. \n";
        return false;
    }
       
    return true;
}


struct XmlData{
    std::string recID;
    int ligID;  
    int poseID;
    double vina;
    double gbsa;
};

void xmltoData(std::string& xmlFile, std::vector<XmlData*>& xmlList){
    XMLDocument doc(xmlFile);
    bool loadOkay = doc.LoadFile();

    if (!loadOkay) {
        std::string mesg = doc.ErrorDesc();
        mesg = "Could not load PPL4Track.xml file.\nError: " + mesg;
        throw LBindException(mesg);
    }
    
    XMLNode* node = doc.FirstChild("Complexes");
    assert(node);
    XMLNode* comNode = node->FirstChild("Complex");
    assert(comNode);

    for (comNode = node->FirstChild("Complex"); comNode != 0; comNode = comNode->NextSibling("Complex")) {

        XMLNode* mesgNode = comNode->FirstChild("Mesg");
        assert(mesgNode);
        XMLText* mesgTx =mesgNode->FirstChild()->ToText();
        assert(mesgTx);
        std::string mesgStr = mesgTx->ValueStr();

        if(mesgStr=="Finished!"){  
            XmlData* pXmlData=new XmlData();            
            
            XMLNode* recIDnode = comNode->FirstChild("RecID");
            assert(recIDnode);
            XMLText* recIDtx =recIDnode->FirstChild()->ToText(); 
            pXmlData->recID=recIDtx->ValueStr();

            XMLNode* ligIDnode = comNode->FirstChild("LigID");
            assert(ligIDnode);
            XMLText* ligIDtx =ligIDnode->FirstChild()->ToText(); 
            pXmlData->ligID=Sstrm<int, std::string>(ligIDtx->ValueStr());  
            
            XMLNode* poseIDnode = comNode->FirstChild("PoseID");
            assert(poseIDnode);
            XMLText* poseIDtx =poseIDnode->FirstChild()->ToText(); 
            pXmlData->poseID=Sstrm<int, std::string>(poseIDtx->ValueStr());              
            
            XMLNode* vinanode = comNode->FirstChild("Vina");
            assert(vinanode);
            XMLText* vinatx =vinanode->FirstChild()->ToText(); 
            pXmlData->vina=Sstrm<double, std::string>(vinatx->ValueStr()); 

            XMLNode* gbnode = comNode->FirstChild("GBSA");
            assert(gbnode);
            XMLText* gbtx =gbnode->FirstChild()->ToText(); 
            pXmlData->gbsa=Sstrm<double, std::string>(gbtx->ValueStr()); 
            
            xmlList.push_back(pXmlData);
        }
        
    } 
    
}

struct PoseXml{
    int poseID;
    double vina;
    double gbsa;    
};

struct LigXml {
    int ligID; 
    int miniGbPoseID;
    double miniGb;
    std::vector<PoseXml*> poseList;    
};

struct RecXml {
    std::string recID;
    std::vector<LigXml*> ligList; 
};

struct less_than_PoseXml
{
    inline bool operator() (const PoseXml* struct1, const PoseXml* struct2)
    {
        return (struct1->poseID < struct2->poseID);
    }
};

struct less_than_LigXml
{
    inline bool operator() (const LigXml* struct1, const LigXml* struct2)
    {
        return (struct1->ligID < struct2->ligID);
    }
};

struct less_than_RecXml
{
    inline bool operator() (const RecXml* struct1, const RecXml* struct2)
    {
        return (struct1->recID < struct2->recID);
    }
};


void toXML(std::vector<RecXml*>& recXmlList, std::string xmlFileName) {
    //! Tracking error using XML file
    XMLDocument doc;  
    XMLDeclaration* decl = new XMLDeclaration( "1.0", "", "" );  
    doc.LinkEndChild( decl );  

    XMLElement * root = new XMLElement( "Complex" );  
    doc.LinkEndChild( root );  

    XMLComment * comment = new XMLComment();
    comment->SetValue(" Tracking calculation error using XML file " );  
    root->LinkEndChild( comment );      
    
    for(unsigned i=0; i<recXmlList.size(); ++i){

        XMLElement * element = new XMLElement("Receptor");
        element->SetAttribute("RecID", recXmlList[i]->recID.c_str() );
        root->LinkEndChild(element);
 
        for(unsigned j=0; j<recXmlList[i]->ligList.size(); ++j){

            XMLElement * ligidEle = new XMLElement("Ligand");
            std::string ligIDstr=Sstrm<std::string, int>(recXmlList[i]->ligList[j]->ligID);
            ligidEle->SetAttribute("LigID", ligIDstr.c_str() );
            std::string minVinaPoseIDstr=Sstrm<std::string, int>(recXmlList[i]->ligList[j]->poseList[0]->poseID);
            ligidEle->SetAttribute("MinVinaPoseID", minVinaPoseIDstr.c_str() ); //Minimum vina score is the first pose
            std::string minVinaStr=Sstrm<std::string, double>(recXmlList[i]->ligList[j]->poseList[0]->vina);
            ligidEle->SetAttribute("MinVina", minVinaStr.c_str() );
            std::string minGbPoseIDstr=Sstrm<std::string, int>(recXmlList[i]->ligList[j]->miniGbPoseID);
            ligidEle->SetAttribute("MinGBPoseID",  minGbPoseIDstr.c_str() );
            std::string minGbStr=Sstrm<std::string, double>(recXmlList[i]->ligList[j]->miniGb);
            ligidEle->SetAttribute("MinGB",  minGbStr.c_str() );
            element->LinkEndChild(ligidEle);            

            for(unsigned k=0; k<recXmlList[i]->ligList[j]->poseList.size(); ++k){
                XMLElement * poseEle = new XMLElement("Pose");
                std::string poseIDstr=Sstrm<std::string, int>(recXmlList[i]->ligList[j]->poseList[k]->poseID);
                poseEle->SetAttribute("PoseID", poseIDstr.c_str() );
                std::string vinaStr=Sstrm<std::string, double>(recXmlList[i]->ligList[j]->poseList[k]->vina);
                poseEle->SetAttribute("Vina", vinaStr.c_str() );                
                std::string gbStr=Sstrm<std::string, double>(recXmlList[i]->ligList[j]->poseList[k]->gbsa);
                poseEle->SetAttribute("GBSA", gbStr.c_str() ); 
                std::string poseDir="scratch/com/"+recXmlList[i]->recID+"/gbsa/lig_"+ligIDstr+"/pose_"+poseIDstr;
                XMLText * poseDirTx = new XMLText(poseDir.c_str() );
                poseEle->LinkEndChild(poseDirTx);                
                ligidEle->LinkEndChild(poseEle);
            }

        }
    }  
        
    doc.SaveFile( xmlFileName.c_str() );
               
}


/*
 * 
 */
int main(int argc, char** argv) {
    
    POdata podata;
    bool success=PPL4parseXMLPO(argc, argv, podata);
    if(!success){
        return 1;
    }  
    
    std::vector<XmlData*> xmlList;
    xmltoData(podata.inXML, xmlList);
    
    std::vector<RecXml*> recXmlList;
    for(unsigned i=0; i<xmlList.size(); ++i){
        bool hasRec=false;
        unsigned recIndex;
        for(unsigned j=0; j<recXmlList.size(); ++j){
            if(recXmlList[j]->recID==xmlList[i]->recID){
                hasRec=true;
                recIndex=j;
                break;
            }
        }
        
        if(hasRec){
            std::vector<LigXml*> ligXmlList=recXmlList[recIndex]->ligList;
            bool hasLig=false;
            unsigned ligIndex;            
            for(unsigned k=0; k<ligXmlList.size(); ++k){
                if(ligXmlList[k]->ligID==xmlList[i]->ligID){
                    hasLig=true;
                    ligIndex=k;
                    break;
                }                
            }
            
            if(hasLig){
                PoseXml* pPoseXml=new PoseXml();
                pPoseXml->poseID=xmlList[i]->poseID;
                pPoseXml->vina=xmlList[i]->vina;
                pPoseXml->gbsa=xmlList[i]->gbsa; 
                recXmlList[recIndex]->ligList[ligIndex]->poseList.push_back(pPoseXml);
                
            }else{
                LigXml* pLigXml=new LigXml();
                pLigXml->ligID=xmlList[i]->ligID;
                PoseXml* pPoseXml=new PoseXml();
                pPoseXml->poseID=xmlList[i]->poseID;
                pPoseXml->vina=xmlList[i]->vina;
                pPoseXml->gbsa=xmlList[i]->gbsa;

                pLigXml->poseList.push_back(pPoseXml); 
                recXmlList[recIndex]->ligList.push_back(pLigXml);
            }
                                    
        }else{
            RecXml* pRecXml=new RecXml();
            pRecXml->recID=xmlList[i]->recID;
            LigXml* pLigXml=new LigXml();
            pLigXml->ligID=xmlList[i]->ligID;
            PoseXml* pPoseXml=new PoseXml();
            pPoseXml->poseID=xmlList[i]->poseID;
            pPoseXml->vina=xmlList[i]->vina;
            pPoseXml->gbsa=xmlList[i]->gbsa;
            
            pLigXml->poseList.push_back(pPoseXml);
            pRecXml->ligList.push_back(pLigXml);
            recXmlList.push_back(pRecXml);
        }
        
    }
    
    //sort the elements
    std::sort(recXmlList.begin(), recXmlList.end(), less_than_RecXml());
    
    for(unsigned i=0; i<recXmlList.size(); ++i){
        std::sort(recXmlList[i]->ligList.begin(), recXmlList[i]->ligList.end(), less_than_LigXml());
    }
    
    for(unsigned i=0; i<recXmlList.size(); ++i){
        for(unsigned j=0; j<recXmlList[i]->ligList.size(); ++j){
            std::sort(recXmlList[i]->ligList[j]->poseList.begin(), 
                    recXmlList[i]->ligList[j]->poseList.end(), less_than_PoseXml());
        }
    } 
    
    //get minimum gb value;
    for(unsigned i=0; i<recXmlList.size(); ++i){
        for(unsigned j=0; j<recXmlList[i]->ligList.size(); ++j){
            
            double curMinGb=BIGPOSITIVE;
            int curMinGgIndex=-1;
            for(unsigned k=0; k<recXmlList[i]->ligList[j]->poseList.size(); ++k){
                if(curMinGb>recXmlList[i]->ligList[j]->poseList[k]->gbsa){
                    curMinGb=recXmlList[i]->ligList[j]->poseList[k]->gbsa;
                    curMinGgIndex=recXmlList[i]->ligList[j]->poseList[k]->poseID;                            
                }
                
            }
            recXmlList[i]->ligList[j]->miniGbPoseID=curMinGgIndex;
            recXmlList[i]->ligList[j]->miniGb=curMinGb;
        }
    } 

    toXML(recXmlList, podata.outXML);
        
    return 0;
}

