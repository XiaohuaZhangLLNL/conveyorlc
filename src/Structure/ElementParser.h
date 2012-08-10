/* 
 * File:   ElementParser.h
 * Author: zhang
 *
 * Created on August 23, 2010, 4:00 PM
 */

#ifndef ELEMENTPARSER_H
#define	ELEMENTPARSER_H

#include "BaseParser.h"

#include <string>

namespace LBIND{
    class ElementContainer;

class ElementParser: public BaseParser {
public:
    ElementParser(ElementContainer* pEleCon);
    ElementParser(const ElementParser& orig);
    virtual ~ElementParser();

    void read(std::string& filename);

private:
    ElementContainer* pElementContainer;


};

}//namespace LBIND
#endif	/* ELEMENTPARSER_H */

