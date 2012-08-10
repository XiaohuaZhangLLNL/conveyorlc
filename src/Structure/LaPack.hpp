/* 
 * File:   LaPack.h
 * Author: zhang
 *
 * Created on December 15, 2010, 7:28 PM
 */

#ifndef LAPACK_HPP
#define	LAPACK_HPP

#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/bindings/lapack/syev.hpp>
#include <boost/numeric/bindings/lapack/gesvd.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/traits/ublas_vector.hpp>

namespace ublas  = boost::numeric::ublas;
namespace lapack = boost::numeric::bindings::lapack;

namespace LBIND{

/*!
   \brief Diagonalize a matrix

   Diag(S):
    Diagonalizing S involves finding U such that:
    transpose(U) * S * U = D
    where D is the diagonal matrix containig the eigenvalues,
    and U is the eigenvector matrix.

    \param eigenvectors Matrix to be diagonalized, it is destroyed, returned containing the eigenvectors
    \param eigenvalues Vector of resulting eigenvalues
    \return success
*/
inline int diagonalize(ublas::matrix<double, ublas::column_major>& eigenvectors,
                       ublas::vector<double>& eigenvalues) {
    int r = lapack::syev( 'V', 'U', eigenvectors, eigenvalues, lapack::minimal_workspace() );
    return r;
};

/*!
   \brief SVD

http://en.wikipedia.org/wiki/Singular_value_decomposition
http://www.netlib.org/lapack/lug/node32.html
    \return success
*/
//inline int svd(ublas::matrix<double, ublas::column_major>& eigenvectors,
//                       ublas::vector<double>& eigenvalues) {
//    int r = lapack::gesvd( 'V', 'U', eigenvectors, eigenvalues, lapack::minimal_workspace() );
//    return r;
//};

/*!
   \brief Sorts eigenvalues and applies this ordering on the eigenvector matrix
   \param eigenvectors Eigenvector matrix
   \param eigenvalues Eigenvalue matrix
   \param order Ascending = 0, Desending = 1
*/
inline void eigenValueSort(ublas::matrix<double, ublas::column_major>& eigenvectors,
                           ublas::vector<double>& eigenvalues, int order) {
    int k;
    double p;
    int size = eigenvectors.size1();

    // Ascending
    if (!order) {
      for (int i = 0; i < size-1 ; ++i) {
        k = i;
        p = eigenvalues(i);

        for (int j = i+1; j < size; ++j) {
          if (eigenvalues(j) < p) {
            k = j;
            p = eigenvalues(j);
          }
        }
        if ( k != i ) {
          eigenvalues(k) = eigenvalues(i);
          eigenvalues(i) = p;
          for (int m = 0; m < size; ++m) {
            p = eigenvectors(m,i);
            eigenvectors(m,i) = eigenvectors(m,k);
            eigenvectors(m,k) = p;
          }
        }
      }
    }
    // Descending
    else {
      for (int i = 0; i < size-1 ; ++i) {
        k = i;
        p = eigenvalues(i);

        for (int j = i+1; j < size; ++j) {
          if (eigenvalues(j) > p) {
            k = j;
            p = eigenvalues(j);
          }
        }
        if ( k != i ) {
          eigenvalues(k) = eigenvalues(i);
          eigenvalues(i) = p;
          for (int m = 0; m < size; ++m) {
            p = eigenvectors(m,i);
            eigenvectors(m,i) = eigenvectors(m,k);
            eigenvectors(m,k) = p;
          }
        }
      }
    }
};

} // namespace LBIND

#endif	/* LAPACK_HPP */

