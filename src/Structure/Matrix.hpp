/* 
 * File:   Matrix.h
 * Author: zhang
 *
 * Created on January 27, 2011, 1:16 PM
 */

#ifndef MATRIX_HPP
#define	MATRIX_HPP

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace ublas  = boost::numeric::ublas;

namespace LBIND{
 /* Matrix inversion routine.
 Uses lu_factorize and lu_substitute in uBLAS to invert a matrix */
template<class T>
bool InvertMatrix(const ublas::matrix<T>& input, ublas::matrix<T>& inverse)
{
    typedef ublas::permutation_matrix<std::size_t> pmatrix;

    // create a working copy of the input
    ublas::matrix<T> A(input);

    // create a permutation matrix for the LU-factorization
    pmatrix pm(A.size1());

    // perform LU-factorization
    int res = ublas::lu_factorize(A, pm);
    if (res != 0)
        return false;

    // create identity matrix of "inverse"
    inverse.assign(ublas::identity_matrix<T> (A.size1()));

    // backsubstitute to get the inverse
    ublas::lu_substitute(A, pm, inverse);

    return true;
}

}//namespace LBIND

#endif	/* MATRIX_HPP */

