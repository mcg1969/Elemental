/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef LAPACK_SKEWHERMITIANEIG_HPP
#define LAPACK_SKEWHERMITIANEIG_HPP

#ifdef HAVE_PMRRR

#include "elemental/blas-like/level1/ScaleTrapezoid.hpp"

namespace elem {

//----------------------------------------------------------------------------//
// Grab the full set of eigenpairs of the real, skew-symmetric matrix G       //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<double>& G,
  DistMatrix<double,VR,STAR>& wImag,
  DistMatrix<Complex<double> >& Z )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("SkewHermitian matrices must be square");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<Complex<double> > A(grid);
    A.AlignWith( G.DistData() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LDim();
    const int GLDim = G.LDim();
    const Complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.Buffer();
    Complex<double>* ABuffer = A.Buffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, A, wImag, Z );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenpairs of the real, skew-symmetric n x n matrix  //
// G. The partial set is determined by the inclusive zero-indexed range       //
//   a,a+1,...,b    ; a >= 0, b < n                                           //
// of the n eigenpairs sorted from smallest to largest eigenvalues.           //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<double>& G,
  DistMatrix<double,VR,STAR>& wImag,
  DistMatrix<Complex<double> >& Z,
  int a, int b )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<Complex<double> > A(grid);
    A.AlignWith( G.DistData() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LDim();
    const int GLDim = G.LDim();    
    const Complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.Buffer();
    Complex<double>* ABuffer = A.Buffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, A, wImag, Z, a, b );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenpairs of the real, skew-symmetric n x n matrix  //
// G. The partial set is determined by the half-open imaginary interval (a,b] //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<double>& G,
  DistMatrix<double,VR,STAR>& wImag,
  DistMatrix<Complex<double> >& Z,
  double a, double b )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("SkewHermitian matrices must be square");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<Complex<double> > A(grid);
    A.AlignWith( G.DistData() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LDim();
    const int GLDim = G.LDim();    
    const Complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.Buffer();
    Complex<double>* ABuffer = A.Buffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, A, wImag, Z, a, b );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab the full set of eigenvalues the of the real, skew-symmetric matrix G  //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<double>& G,
  DistMatrix<double,VR,STAR>& wImag )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("SkewHermitian matrices must be square");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<Complex<double> > A(grid);
    A.AlignWith( G.DistData() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LDim();
    const int GLDim = G.LDim();    
    const Complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.Buffer();
    Complex<double>* ABuffer = A.Buffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, A, wImag );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenvalues of the real, skew-symmetric n x n matrix //
// G. The partial set is determined by the inclusive zero-indexed range       //
//   a,a+1,...,b    ; a >= 0, b < n                                           //
// of the n eigenpairs sorted from smallest to largest eigenvalues.           //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<double>& G,
  DistMatrix<double,VR,STAR>& wImag,
  int a, int b )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<Complex<double> > A(grid);
    A.AlignWith( G.DistData() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LDim();
    const int GLDim = G.LDim();    
    const Complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.Buffer();
    Complex<double>* ABuffer = A.Buffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, A, wImag, a, b );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenvalues of the real, skew-symmetric n x n matrix //
// G. The partial set is determined by the half-open imaginary interval (a,b] //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<double>& G,
  DistMatrix<double,VR,STAR>& wImag,
  double a, double b )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<Complex<double> > A(grid);
    A.AlignWith( G.DistData() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LDim();
    const int GLDim = G.LDim();    
    const Complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.Buffer();
    Complex<double>* ABuffer = A.Buffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, A, wImag, a, b );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab the full set of eigenpairs of the complex, skew-hermitian matrix G    //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<Complex<double> >& G,
  DistMatrix<double,VR,STAR>& wImag,
  DistMatrix<Complex<double> >& Z )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");

    // Make G Hermitian by scaling by -i
    const Complex<double> negativeImagOne(0,-1.);
    ScaleTrapezoid( negativeImagOne, LEFT, uplo, 0, G );

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, G, wImag, Z );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenpairs of the complex, skew-hermitian n x n      //
// matrix G. The partial set is determined by the inclusive zero-indexed      //
// range                                                                      //
//   a,a+1,...,b    ; a >= 0, b < n                                           //
// of the n eigenpairs sorted from smallest to largest eigenvalues.           //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<Complex<double> >& G,
  DistMatrix<double,VR,STAR>& wImag,
  DistMatrix<Complex<double> >& Z,
  int a, int b )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");
    
    // Make G Hermitian by scaling by -i
    const Complex<double> negativeImagOne(0,-1.);
    ScaleTrapezoid( negativeImagOne, LEFT, uplo, 0, G );

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, G, wImag, Z, a, b );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenpairs of the complex, skew-hermitian n x n      //
// matrix G. The partial set is determined by the half-open imaginary range   //
// (a,b].                                                                     //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<Complex<double> >& G,
  DistMatrix<double,VR,STAR>& wImag,
  DistMatrix<Complex<double> >& Z,
  double a, double b )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");
    
    // Make G Hermitian by scaling by -i
    const Complex<double> negativeImagOne(0,-1.);
    ScaleTrapezoid( negativeImagOne, LEFT, uplo, 0, G );

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, G, wImag, Z, a, b );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab the full set of eigenvalues of the complex, skew-Hermitian matrix G   //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<Complex<double> >& G,
  DistMatrix<double,VR,STAR>& wImag )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");
    
    // Make G Hermitian by scaling by -i
    const Complex<double> negativeImagOne(0,-1.);
    ScaleTrapezoid( negativeImagOne, LEFT, uplo, 0, G );

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, G, wImag );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenvalues of the complex, skew-Hermitian n x n     //
//  matrix G. The partial set is determined by the inclusive zero-indexed     //
// range                                                                      //
//   a,a+1,...,b    ; a >= 0, b < n                                           //
// of the n eigenpairs sorted from smallest to largest eigenvalues.           //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<Complex<double> >& G,
  DistMatrix<double,VR,STAR>& wImag,
  int a, int b )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");
    
    // Make G Hermitian by scaling by -i
    const Complex<double> negativeImagOne(0,-1.);
    ScaleTrapezoid( negativeImagOne, LEFT, uplo, 0, G );

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, G, wImag, a, b );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenvalues of the complex, skew-Hermitian n x n     //
// matrix G. The partial set is determined by the half-open imaginary         //
// interval (a,b].                                                            //
//----------------------------------------------------------------------------//
inline void
SkewHermitianEig
( UpperOrLower uplo, 
  DistMatrix<Complex<double> >& G,
  DistMatrix<double,VR,STAR>& wImag,
  double a, double b )
{
#ifndef RELEASE
    PushCallStack("SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square");
    
    // Make G Hermitian by scaling by -i
    const Complex<double> negativeImagOne(0,-1.);
    ScaleTrapezoid( negativeImagOne, LEFT, uplo, 0, G );

    // Perform the Hermitian eigensolve
    HermitianEig( uplo, G, wImag, a, b );
#ifndef RELEASE
    PopCallStack();
#endif
}

} // namespace elem

#endif // ifdef HAVE_PMRRR

#endif // ifndef LAPACK_SKEWHERMITIANEIG_HPP
