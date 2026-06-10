/* 
 * File:   Amber.h
 * Author: zhang30
 *
 * Created on December 15, 2011, 5:43 PM
 */

#include <string>
#include <array>
#include <vector>
#include "Common/File.hpp"

#ifndef AMBER_H
#define	AMBER_H

namespace LBIND {
    class Protein;
class Amber {
public:
    Amber();
    Amber(int amberVersion);
    Amber(Protein* pProt);
    Amber(Protein* pProt, int amberVersion);
    Amber(const Amber& orig);
    virtual ~Amber();
    
    void run();
    
    void reduce(std::string& input, std::string& output, std::string& options);
    void antechamber(std::string& input, std::string& output, std::string& options);
    void parmchk(std::string mol2FName);
    void parmchk2(std::string mol2FName);
    void tleapInput(std::string& mol2FName, std::string& ligName, std::string& tleapFName, std::string& subDir);
    void tleap(std::string input);
    void minimization(std::string pdbid);
    void md(std::string pdbid);
    
    void ambpdb(std::string input, std::string output);
    
    void prepLigands();

    static std::string getAmberPath(void);
    static std::string getAmberForceFieldFile(void);
    static std::string getAmberPhosAAFile(void);
    static int getAmberVersion(void);

protected:
    static const std::array<const char*, 6> AMBER_PROTEIN_FF_CANDIDATES;
    static const std::array<const char*, 3> AMBER_PHOSAA_CANDIDATES;
    static const std::array<std::string, 2> AMBER_LEAP_DIRS;

    static std::string setAmberPath(void);
    static int setAmberVersion(void);

    template <std::size_t N>
    static std::string findAmberLeaprcFile(
        const std::array<const char*, N>& candidates);

    static std::string setAmberForceFieldFile(void);
    static std::string setAmberPhosAAFile(void);

    static void initStaticMembers();
    struct Initializer {
        Initializer() {
            Amber::initStaticMembers();
        }
    };

    // The declaration that triggers the magic
    static Initializer initializer; 

private:
    void ligLeapInput(std::string pdbid, std::string ligName, std::string tleapFName);
    void comLeapInput(std::string pdbid, std::string ligName, std::string tleapFName);
    
private:
    static int version;
    Protein* pProtein;
    static std::string AMBERPATH;
    static std::string forceFieldFile;
    static std::string phosAAFile;
};

template <size_t N>
std::string Amber::findAmberLeaprcFile(
    const std::array<const char*, N>& candidates)
{
    const std::vector<std::string> searchDirs = []() {
        std::vector<std::string> dirs;
        dirs.reserve(AMBER_LEAP_DIRS.size());
        for (const auto& dir : AMBER_LEAP_DIRS) {
            dirs.push_back(joinPath(AMBERPATH, dir));
        }
        return dirs;
    }();

    for (const auto& filename : candidates) {
        for (const auto& dir : searchDirs) {
            const std::string fullPath = joinPath(dir, filename);
            if (fileExist(fullPath)) {
                return fullPath;
            }
        }
    }
    return "";
}

}//namespace LBIND 
#endif	/* AMBER_H */

