/* 
 * File:   ElementContainer.h
 * Author: zhang
 *
 * Created on August 25, 2010, 10:43 AM
 */

#ifndef ELEMENTCONTAINER_H
#define	ELEMENTCONTAINER_H

#include <map>
#include <string>


namespace LBIND{
    class Element;
class ElementContainer {
public:
    ElementContainer();
    ElementContainer(const ElementContainer& orig);
    virtual ~ElementContainer();

    Element* addElement();
    void setElementMap(Element* pElement);
    std::map<std::string, Element*> getElementMap();
    Element* symbolToElement(std::string symbol);
private:
    void toLBindSymbol(std::string& symbol);

private:
    std::map<std::string, Element*> itsElementMap;
    typedef std::map<std::string, Element*>::iterator ElementMapIterator;
};

}//namespace LBIND
#endif	/* ELEMENTCONTAINER_H */

