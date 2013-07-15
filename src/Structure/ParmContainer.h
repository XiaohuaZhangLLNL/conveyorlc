/* 
 * File:   ParmContainer.h
 * Author: zhang
 *
 * Created on September 15, 2010, 3:32 PM
 */

#ifndef PARMCONTAINER_H
#define	PARMCONTAINER_H


namespace LBIND{
    class ElementContainer;
class ParmContainer {
public:
    ParmContainer();
    ParmContainer(const ParmContainer& orig);
    virtual ~ParmContainer();
    ElementContainer* addElementContainer();
    ElementContainer* getElementContainer();

private:
    ElementContainer* pElementContainer;

};

}//namespace LBIND

#endif	/* PARMCONTAINER_H */

