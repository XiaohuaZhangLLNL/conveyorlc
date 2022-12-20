//
// Created by Zhang, Xiaohua on 2019-07-19.
//


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "Parser/Pdb.h"
#include "Parser/Mol2.h"
#include "Parser/Sdf.h"
#include "Common/LBindException.h"


using namespace LBIND;


void testBoundBox(){

    bool hasSubResCoor;

    {
        Coor3d centroid;
        Coor3d boxDim;
        boost::scoped_ptr<Mol2> pMol2(new Mol2());
        hasSubResCoor = pMol2->calcBoundBox("1a50_ligand.mol2", centroid, boxDim);
        std::cout << "Average coordinates of mol2: " << centroid << std::endl;
        std::cout << "Box of mol2:                 " << boxDim << std::endl;
    }
    {
        Coor3d centroid;
        Coor3d boxDim;
        boost::scoped_ptr<Pdb> pPdb(new Pdb());
        hasSubResCoor = pPdb->calcBoundBox("1a50_ligand.pdb", centroid, boxDim);
        std::cout << "Average coordinates of pdb: " << centroid << std::endl;
        std::cout << "Box of pdb:                 " << boxDim << std::endl;
    }

    {
        Coor3d centroid;
        Coor3d boxDim;
        boost::scoped_ptr<Sdf> pSdf(new Sdf());
        hasSubResCoor = pSdf->calcBoundBox("1a50_ligand.sdf", centroid, boxDim);
        std::cout << "Average coordinates of sdf: " << centroid << std::endl;
        std::cout << "Box of sdf:                 " << boxDim << std::endl;
        boxDim +=10;
        std::cout << "Box +10 of sdf:             " << boxDim << std::endl;
    }
}

double boostConv(){
    std::string substr = "nan";
    if(substr=="nan" || substr=="-nan"){
        throw LBIND::LBindException(std::string("\"") + substr + "\" is not a valid ");
    }
    try {
        return boost::lexical_cast<double>(substr);
    }
    catch(...) {
        throw LBIND::LBindException(std::string("\"") + substr + "\" is not a valid ");
    }
}
/*
void io_context(){

}

void boostTimer(){
    boost::asio::deadline_timer timer(io_context);

// Set an expiry time relative to now.
    timer.expires_from_now(boost::posix_time::seconds(5));

// Wait for the timer to expire.
    timer.wait();
}

int main(int argc, char** argv) {
    //testBoundBox();
    try {
        double a=boostConv();
        a=a+1;
        std::cout << "a=" << a << std::endl;
    }catch(LBIND::LBindException& e){
        std::cout << "Error: " << e.what() << std::endl;
    }
}
 */

// A custom implementation of the Clock concept from the standard C++ library.
struct time_t_clock
{
    // The duration type.
    typedef boost::asio::chrono::steady_clock::duration duration;

    // The duration's underlying arithmetic representation.
    typedef duration::rep rep;

    // The ratio representing the duration's tick period.
    typedef duration::period period;

    // An absolute time point represented using the clock.
    typedef boost::asio::chrono::time_point<time_t_clock> time_point;

    // The clock is not monotonically increasing.
    static const bool is_steady = false;

    // Get the current time.
    static time_point now()
    {
        return time_point() + boost::asio::chrono::seconds(std::time(0));
    }
};

// The boost::asio::basic_waitable_timer template accepts an optional WaitTraits
// template parameter. The underlying time_t clock has one-second granularity,
// so these traits may be customised to reduce the latency between the clock
// ticking over and a wait operation's completion. When the timeout is near
// (less than one second away) we poll the clock more frequently to detect the
// time change closer to when it occurs. The user can select the appropriate
// trade off between accuracy and the increased CPU cost of polling. In extreme
// cases, a zero duration may be returned to make the timers as accurate as
// possible, albeit with 100% CPU usage.
struct time_t_wait_traits
{
    // Determine how long until the clock should be next polled to determine
    // whether the duration has elapsed.
    static time_t_clock::duration to_wait_duration(
            const time_t_clock::duration& d)
    {
        if (d > boost::asio::chrono::seconds(1))
            return d - boost::asio::chrono::seconds(1);
        else if (d > boost::asio::chrono::seconds(0))
            return boost::asio::chrono::milliseconds(10);
        else
            return boost::asio::chrono::seconds(0);
    }

    // Determine how long until the clock should be next polled to determine
    // whether the absoluate time has been reached.
    static time_t_clock::duration to_wait_duration(
            const time_t_clock::time_point& t)
    {
        return to_wait_duration(t - time_t_clock::now());
    }
};

typedef boost::asio::basic_waitable_timer<
        time_t_clock, time_t_wait_traits> time_t_timer;

void handle_timeout(const boost::system::error_code&)
{

    std::cout << "handle_timeout\n";
    while(1){
        int a=a+1;
    }
}
/*
int main()
{
    try
    {
        boost::asio::io_context io_context;

        //time_t_timer timer(io_context);

        //timer.expires_after(boost::asio::chrono::seconds(5));
        //std::cout << "Starting synchronous wait\n";
        //timer.wait();
        //std::cout << "Finished synchronous wait\n";


        //timer.expires_after(boost::asio::chrono::seconds(30));
        //std::cout << "Starting asynchronous wait\n";
        //timer.async_wait(&handle_timeout);
        //io_context.run();
        //std::cout << "Finished asynchronous wait\n";

        boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();
        boost::posix_time::ptime end = start + boost::posix_time::seconds(10);

        boost::asio::deadline_timer timer(io_context, end);
        timer.async_wait(handle_timeout);
        io_context.run();

    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}
*/



using namespace std::chrono_literals;

void f(int i)
{
    std::cout << "i=" << i <<std::endl;
    std::this_thread::sleep_for(5s); //change value here to less than 1 second to see Success
    //return i;
}

int f_wrapper(int i)
{
    std::mutex m;
    std::condition_variable cv;
    int retValue;

    std::thread t([&cv, &i]()
                  {
                      f(i);
                      cv.notify_one();
                  });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, 10s) == std::cv_status::timeout)
            throw std::runtime_error("Timeout");
    }

    return retValue;
}

int main()
{
    bool timedout = false;
    try {
        int i=5;
        f_wrapper(i);
    }
    catch(std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        timedout = true;
    }

    if(!timedout)
        std::cout << "Success" << std::endl;

    return 0;
}