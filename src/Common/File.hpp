/* 
 * File:   file.hpp
 * Author: zhang30
 *
 * Created on August 22, 2012, 5:48 PM
 */

#ifndef FILE_HPP
#define	FILE_HPP

#include <string>

namespace LBIND {


void getPathName (const std::string& pathStr, std::string& pathName);
    
void getPathFileName (const std::string& pathStr, std::string& pathFileName);

void getFileBasename(const std::string& pathStr, std::string& pathFileName);

bool fileExist(const std::string& pathFile);

bool fileEmpty(std::string& pathFile);   

// !low overhead
int getFileSizebyCommand(std::string& pathFile);

int getFileSize( std::string& pathFile );

bool checkFile(std::string& pathFile);

bool dirExists(const std::string& path);

std::string joinPath(const std::string& dir, const std::string& file);
    
}//namespace LBIND 

#endif	/* FILE_HPP */

