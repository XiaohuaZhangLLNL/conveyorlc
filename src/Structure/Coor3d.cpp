/* 
 * File:   Coor3d.cpp
 * Author: xiaohua
 * 
 * Created on May 26, 2009, 10:44 AM
 */

#include "Coor3d.h"


namespace LBIND{

Coor3d::Coor3d():x(0.0),y(0.0),z(0.0) {
}

Coor3d::Coor3d(double coor):x(coor),y(coor),z(coor){

}

Coor3d::Coor3d(double coorx, double coory, double coorz)
      :x(coorx),y(coory),z(coorz){

}

Coor3d::Coor3d(const Coor3d& orig) {
}

Coor3d::~Coor3d() {
}

void Coor3d::set(const double coorx, const double coory, const double coorz)
{
    this->x=coorx;
    this->y=coory;
    this->z=coorz;
}

double Coor3d::getX()
{
    return this->x;
}

double Coor3d::getY()
{
    return this->y;
}

double Coor3d::getZ()
{
    return this->z;
}

void Coor3d::setX(double x)
{
    this->x=x;
}

void Coor3d::setY(double y)
{
    this->y=y;
}

void Coor3d::setZ(double z)
{
    this->z=z;
}

double Coor3d::length(){
    return sqrt(x*x+y*y+z*z);
}

void Coor3d::normalize(){
    double len= this->length();
    x=x/len;
    y=y/len;
    z=z/len;
}

double Coor3d::dist(const Coor3d* coord){
    return sqrt((x-coord->x)*(x-coord->x)+(y-coord->y)*(y-coord->y)
            +(z-coord->z)*(z-coord->z));
}

double Coor3d::dist2(const Coor3d* coord){
    return ((x-coord->x)*(x-coord->x)+(y-coord->y)*(y-coord->y)
            +(z-coord->z)*(z-coord->z));
}

double Coor3d::dist2(const Coor3d& coord){
    return ((x-coord.x)*(x-coord.x)+(y-coord.y)*(y-coord.y)
            +(z-coord.z)*(z-coord.z));
}

double Coor3d::dist2(const double xCoor, const double yCoor, const double zCoor){
    return ((x-xCoor)*(x-xCoor)+(y-yCoor)*(y-yCoor)
            +(z-zCoor)*(z-zCoor));
}

void Coor3d::rotate(const double rotMat[9], const Coor3d& o) {
            this->x -= o.x;
            this->y -= o.y;
            this->z -= o.z;
            double newx = this->x*rotMat[0] + this->y*rotMat[1] + this->z*rotMat[2];
            double newy = this->x*rotMat[3] + this->y*rotMat[4] + this->z*rotMat[5];
            double newz = this->x*rotMat[6] + this->y*rotMat[7] + this->z*rotMat[8];
            this->x = newx + o.x;
            this->y = newy + o.y;
            this->z = newz + o.z;
}

} //namespace LBIND
