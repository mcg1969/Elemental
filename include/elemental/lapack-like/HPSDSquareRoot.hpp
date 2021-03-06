/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef LAPACK_HPSDSQUAREROOT_HPP
#define LAPACK_HPSDSQUAREROOT_HPP

#ifdef HAVE_PMRRR

#include "elemental/lapack-like/HermitianFunction.hpp"
#include "elemental/lapack-like/Norm/Max.hpp"

namespace elem {

//
// Square root the eigenvalues of A (and treat the sufficiently small negative
// ones as zero).
//

template<typename F>
inline void
HPSDSquareRoot( UpperOrLower uplo, DistMatrix<F>& A )
{
#ifndef RELEASE
    PushCallStack("HPSDSquareRoot");
#endif
    typedef typename Base<F>::type R;

    // Get the EVD of A
    const Grid& g = A.Grid();
    DistMatrix<R,VR,STAR> w(g);
    DistMatrix<F> Z(g);
    HermitianEig( uplo, A, w, Z );

    // Compute the two-norm of A as the maximum absolute value of the eigvals
    const R twoNorm = MaxNorm( w );

    // Compute the smallest eigenvalue of A
    R minLocalEig = twoNorm;
    const int numLocalEigs = w.LocalHeight();
    for( int iLocal=0; iLocal<numLocalEigs; ++iLocal )
    {
        const R omega = w.GetLocal(iLocal,0);
        minLocalEig = std::min(minLocalEig,omega);
    }
    R minEig;
    mpi::AllReduce( &minLocalEig, &minEig, 1, mpi::MIN, g.VCComm() );

    // Set the tolerance equal to n ||A||_2 eps
    const int n = A.Height();
    const R eps = lapack::MachineEpsilon<R>();
    const R tolerance = n*twoNorm*eps;

    // Ensure that the minimum eigenvalue is not less than - n ||A||_2 eps
    if( minEig < -tolerance )
        throw NonHPSDMatrixException();

    // Overwrite the eigenvalues with f(w)
    for( int iLocal=0; iLocal<numLocalEigs; ++iLocal )
    {
        const R omega = w.GetLocal(iLocal,0);
        if( omega > R(0) )
            w.SetLocal(iLocal,0,Sqrt(omega));
        else
            w.SetLocal(iLocal,0,0);
    }

    // Form the pseudoinverse
    hermitian_function::ReformHermitianMatrix( uplo, A, w, Z );
#ifndef RELEASE
    PopCallStack();
#endif
}

} // namespace elem

#endif // ifdef HAVE_PMRRR

#endif // ifndef LAPACK_HPSDSQUAREROOT_HPP
