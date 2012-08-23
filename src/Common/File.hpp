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


void getPathName (const std::string& pathStr, std::string& pathName)
{
  size_t found;
  found=pathStr.find_last_of("/\\");
  pathName=pathStr.substr(0,found);

}
    
void getPathFileName (const std::string& pathStr, std::string& pathFileName)
{
  size_t found;
  found=pathStr.find_last_of("/\\");
  pathFileName= pathStr.substr(found+1);
}    
    
}//namespace LBIND 

#endif	/* FILE_HPP */

