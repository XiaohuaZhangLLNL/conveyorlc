/* 
 * File:   DataStruct.h
 * Author: zhang
 *
 * Created on April 1, 2011, 11:56 AM
 */

#ifndef DATASTRUCT_H
#define	DATASTRUCT_H

namespace LBIND {

template <class T1, class T2>
class DataTwo {
public:
    DataTwo(T1 f, T2 s): first(f), second(s){       
    }
    void setFirst(T1 f){
        first=f;
    };
    T1 getFirst(){
        return first;
    };
    void setSecond(T2 s){
        second=s;
    };
    T2 getSecond(){
        return second;
    };


private:
    T1 first;
    T2 second;
};

template <class T1, class T2, class T3>
class DataThree {
public:
    DataThree(T1 f, T2 s, T3 t): first(f), second(s), third(t){
    }
    void setFirst(T1 f){
        first=f;
    };
    T1 getFirst(){
        return first;
    };
    void setSecond(T2 s){
        second=s;
    };
    T2 getSecond(){
        return second;
    };
    void setThird(T3 t){
        third=t;
    };
    T3 getThird(){
        return third;
    };

private:
    T1 first;
    T2 second;
    T3 third;
};


template <class T1, class T2, class T3, class T4>
class DataFour {
public:
    DataFour(T1 f, T2 s, T3 t, T4 fr): first(f), second(s), third(t), fourth(fr){
    }
    void setFirst(T1 f){
        first=f;
    };
    T1 getFirst(){
        return first;
    };
    void setSecond(T2 s){
        second=s;
    };
    T2 getSecond(){
        return second;
    };
    void setThird(T3 t){
        third=t;
    };
    T3 getThird(){
        return third;
    };
    void setFourth(T4 fr){
        fourth=fr;
    };
    T4 getFourth(){
        return fourth;
    };    

private:
    T1 first;
    T2 second;
    T3 third;
    T4 fourth;
};

}//namespace LBIND

#endif	/* DATASTRUCT_H */

