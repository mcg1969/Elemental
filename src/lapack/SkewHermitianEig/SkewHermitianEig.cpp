/*
   Copyright (c) 2009-2011, Jack Poulson
   All rights reserved.

   This file is part of Elemental.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    - Neither the name of the owner nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WITHOUT_COMPLEX
#ifndef WITHOUT_PMRRR
#include "elemental/lapack.hpp"
using namespace elemental;

//----------------------------------------------------------------------------//
// Grab the full set of eigenpairs of the real, skew-symmetric matrix G       //
//----------------------------------------------------------------------------//
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<double,              MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& Z,
  bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("SkewHermitian matrices must be square.");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<std::complex<double>,MC,MR> A(grid);
    A.Align( G.ColAlignment(), G.RowAlignment() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LocalLDim();
    const int GLDim = G.LocalLDim();
    const std::complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.LocalBuffer();
    std::complex<double>* ABuffer = A.LocalBuffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, A, s, Z, tryForHighAccuracy );

    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

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
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<double,              MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& Z,
  int a, int b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<std::complex<double>,MC,MR> A(grid);
    A.Align( G.ColAlignment(), G.RowAlignment() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LocalLDim();
    const int GLDim = G.LocalLDim();    
    const std::complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.LocalBuffer();
    std::complex<double>* ABuffer = A.LocalBuffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, A, s, Z, a, b, tryForHighAccuracy );
    
    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenpairs of the real, skew-symmetric n x n matrix  //
// G. The partial set is determined by the half-open imaginary interval (a,b] // 
//----------------------------------------------------------------------------//
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<double,              MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& Z,
  double a, double b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("SkewHermitian matrices must be square.");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<std::complex<double>,MC,MR> A(grid);
    A.Align( G.ColAlignment(), G.RowAlignment() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LocalLDim();
    const int GLDim = G.LocalLDim();    
    const std::complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.LocalBuffer();
    std::complex<double>* ABuffer = A.LocalBuffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, A, s, Z, a, b, tryForHighAccuracy );
    
    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab the full set of eigenvalues the of the real, skew-symmetric matrix G  //
//----------------------------------------------------------------------------//
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<double,              MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("SkewHermitian matrices must be square.");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<std::complex<double>,MC,MR> A(grid);
    A.Align( G.ColAlignment(), G.RowAlignment() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LocalLDim();
    const int GLDim = G.LocalLDim();    
    const std::complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.LocalBuffer();
    std::complex<double>* ABuffer = A.LocalBuffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, A, s, tryForHighAccuracy );
    
    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

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
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<double,              MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  int a, int b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<std::complex<double>,MC,MR> A(grid);
    A.Align( G.ColAlignment(), G.RowAlignment() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LocalLDim();
    const int GLDim = G.LocalLDim();    
    const std::complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.LocalBuffer();
    std::complex<double>* ABuffer = A.LocalBuffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, A, s, a, b, tryForHighAccuracy );
    
    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenvalues of the real, skew-symmetric n x n matrix //
// G. The partial set is determined by the half-open imaginary interval (a,b] //
//----------------------------------------------------------------------------//
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<double,              MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  double a, double b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");

    int n = G.Height();
    const Grid& grid = G.Grid();

    DistMatrix<std::complex<double>,MC,MR> A(grid);
    A.Align( G.ColAlignment(), G.RowAlignment() );
    A.ResizeTo( n, n );

    const int localHeight = A.LocalHeight();
    const int localWidth = A.LocalWidth();
    const int ALDim = A.LocalLDim();
    const int GLDim = G.LocalLDim();    
    const std::complex<double> negativeImagOne(0,-1.);
    const double* GBuffer = G.LocalBuffer();
    std::complex<double>* ABuffer = A.LocalBuffer();
    // Just copy the entire local matrix instead of worrying about symmetry
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            ABuffer[i+j*ALDim] = negativeImagOne*GBuffer[i+j*GLDim];

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, A, s, a, b, tryForHighAccuracy );
    
    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne); 

#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab the full set of eigenpairs of the complex, skew-hermitian matrix G    //
//----------------------------------------------------------------------------//
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<std::complex<double>,MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& Z,
  bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");

    const Grid& grid = G.Grid();

    // Make G Hermitian by scaling by -i
    const std::complex<double> negativeImagOne(0,-1.);
    G.ScaleTrapezoidal( negativeImagOne, Left, shape );

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, G, s, Z, tryForHighAccuracy );

    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne); 

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
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<std::complex<double>,MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& Z,
  int a, int b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");
    
    const Grid& grid = G.Grid();

    // Make G Hermitian by scaling by -i
    const std::complex<double> negativeImagOne(0,-1.);
    G.ScaleTrapezoidal( negativeImagOne, Left, shape );

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, G, s, Z, a, b, tryForHighAccuracy );

    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenpairs of the complex, skew-hermitian n x n      //
// matrix G. The partial set is determined by the half-open imaginary range   //
// (a,b].                                                                     //
//----------------------------------------------------------------------------//
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<std::complex<double>,MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& Z,
  double a, double b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");
    
    const Grid& grid = G.Grid();

    // Make G Hermitian by scaling by -i
    const std::complex<double> negativeImagOne(0,-1.);
    G.ScaleTrapezoidal( negativeImagOne, Left, shape );

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, G, s, Z, a, b, tryForHighAccuracy );

    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab the full set of eigenvalues of the complex, skew-Hermitian matrix G   //
//----------------------------------------------------------------------------//
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<std::complex<double>,MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");
    
    const Grid& grid = G.Grid();

    // Make G Hermitian by scaling by -i
    const std::complex<double> negativeImagOne(0,-1.);
    G.ScaleTrapezoidal( negativeImagOne, Left, shape );

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, G, s, tryForHighAccuracy );

    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

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
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<std::complex<double>,MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  int a, int b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");
    
    const Grid& grid = G.Grid();

    // Make G Hermitian by scaling by -i
    const std::complex<double> negativeImagOne(0,-1.);
    G.ScaleTrapezoidal( negativeImagOne, Left, shape );

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, G, s, a, b, tryForHighAccuracy );

    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Grab a partial set of eigenvalues of the complex, skew-Hermitian n x n     //
// matrix G. The partial set is determined by the half-open imaginary         //
// interval (a,b].                                                            //
//----------------------------------------------------------------------------//
void
elemental::lapack::SkewHermitianEig
( Shape shape, 
  DistMatrix<std::complex<double>,MC,  MR>& G,
  DistMatrix<std::complex<double>,Star,VR>& w,
  double a, double b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::SkewHermitianEig");
#endif
    if( G.Height() != G.Width() )
        throw std::logic_error("Skew-Hermitian matrices must be square.");
    
    const Grid& grid = G.Grid();

    // Make G Hermitian by scaling by -i
    const std::complex<double> negativeImagOne(0,-1.);
    G.ScaleTrapezoidal( negativeImagOne, Left, shape );

    // Perform the Hermitian eigensolve
    DistMatrix<double,Star,VR> s(grid);
    lapack::HermitianEig( shape, G, s, a, b, tryForHighAccuracy );

    // Backtransform the eigenvalues by multiplying by i
    w.Align( s.RowAlignment() );
    w.ResizeTo( 1, s.Width() );
    const int numLocalEigs = w.LocalWidth();
    const std::complex<double> imagOne(0,1.);
    for( int j=0; j<numLocalEigs; ++j )
        w.SetLocalEntry(0,j,s.GetLocalEntry(0,j)*imagOne);

#ifndef RELEASE
    PopCallStack();
#endif
}
#endif // WITHOUT_PMRRR
#endif // WITHOUT_COMPLEX
