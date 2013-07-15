/* 
 * File:   GZstream.h
 * Author: zhang30
 *
 * Created on August 22, 2012, 4:45 PM
 */

// ============================================================================
// GZstream, C++ iostream classes wrapping the zlib compression library.
// Copyright (C) 2001  Deepak Bandyopadhyay, Lutz Kettner
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ============================================================================
//
// File          : GZstream.h
// Revision      : $Revision: 1.5 $
// Revision_date : $Date: 2002/04/26 23:30:15 $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// 
// Standard streambuf implementation following Nicolai Josuttis, "The 
// Standard C++ Library".
// ============================================================================

#ifndef GZSTREAM_H
#define GZSTREAM_H 1

// standard C++ with new header file names and std:: namespace
#include <iostream>
#include <fstream>
#include <zlib.h>

namespace LBIND{
// ----------------------------------------------------------------------------
// Internal classes to implement GZstream. See below for user classes.
// ----------------------------------------------------------------------------

class GZstreambuf : public std::streambuf {
private:
    static const int bufferSize = 47+256;    // size of data buff
    // totals 512 bytes under g++ for iGZstream at the end.

    gzFile           file;               // file handle for compressed file
    char             buffer[bufferSize]; // data buffer
    char             opened;             // open/close state of stream
    int              mode;               // I/O mode

    int flush_buffer();
public:
    GZstreambuf() : opened(0) {
        setp( buffer, buffer + (bufferSize-1));
        setg( buffer + 4,     // beginning of putback area
              buffer + 4,     // read position
              buffer + 4);    // end position      
        // ASSERT: both input & output capabilities will not be used together
    }
    int is_open() { return opened; }
    GZstreambuf* open( const char* name, int open_mode);
    GZstreambuf* close();
    ~GZstreambuf() { close(); }
    
    virtual int     overflow( int c = EOF);
    virtual int     underflow();
    virtual int     sync();
};

class GZstreambase : virtual public std::ios {
protected:
    GZstreambuf buf;
public:
    GZstreambase() { init(&buf); }
    GZstreambase( const char* name, int open_mode);
    ~GZstreambase();
    void open( const char* name, int open_mode);
    void close();
    GZstreambuf* rdbuf() { return &buf; }
};

// ----------------------------------------------------------------------------
// User classes. Use iGZstream and oGZstream analogously to ifstream and
// ofstream respectively. They read and write files based on the gz* 
// function interface of the zlib. Files are compatible with gzip compression.
// ----------------------------------------------------------------------------

class iGZstream : public GZstreambase, public std::istream {
public:
    iGZstream() : std::istream( &buf) {} 
    iGZstream( const char* name, int open_mode = std::ios::in)
        : GZstreambase( name, open_mode), std::istream( &buf) {}  
    GZstreambuf* rdbuf() { return GZstreambase::rdbuf(); }
    void open( const char* name, int open_mode = std::ios::in) {
        GZstreambase::open( name, open_mode);
    }
};

class oGZstream : public GZstreambase, public std::ostream {
public:
    oGZstream() : std::ostream( &buf) {}
    oGZstream( const char* name, int mode = std::ios::out)
        : GZstreambase( name, mode), std::ostream( &buf) {}  
    GZstreambuf* rdbuf() { return GZstreambase::rdbuf(); }
    void open( const char* name, int open_mode = std::ios::out) {
        GZstreambase::open( name, open_mode);
    }
};


#endif // GZSTREAM_H

}//namespace LBIND
// ============================================================================
// EOF //


