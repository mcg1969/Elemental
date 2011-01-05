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
#include "elemental/environment.hpp"

//----------------------------------------------------------------------------//
// Level 1 BLAS                                                               //
//----------------------------------------------------------------------------//
void
elemental::wrappers::blas::Axpy
( int n, int alpha, const int* x, int incx, int* y, int incy )
{
    for( int i=0; i<n; ++i )
        y[i*incy] += alpha*x[i*incx];
}

void
elemental::wrappers::blas::Axpy
( int n, float alpha, const float* x, int incx, float* y, int incy )
{ BLAS(saxpy)( &n, &alpha, x, &incx, y, &incy ); }

void
elemental::wrappers::blas::Axpy
( int n, double alpha, const double* x, int incx, double* y, int incy )
{ BLAS(daxpy)( &n, &alpha, x, &incx, y, &incy ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Axpy
( int n, scomplex alpha, const scomplex* x, int incx, scomplex* y, int incy )
{ BLAS(caxpy)( &n, &alpha, x, &incx, y, &incy ); }

void
elemental::wrappers::blas::Axpy
( int n, dcomplex alpha, const dcomplex* x, int incx, dcomplex* y, int incy )
{ BLAS(zaxpy)( &n, &alpha, x, &incx, y, &incy ); }
#endif // WITHOUT_COMPLEX

float
elemental::wrappers::blas::Dot
( int n, const float* x, int incx, const float* y, int incy )
{ return BLAS(sdot)( &n, x, &incx, y, &incy ); }

double
elemental::wrappers::blas::Dot
( int n, const double* x, int incx, const double* y, int incy )
{ return BLAS(ddot)( &n, x, &incx, y, &incy ); }

#ifndef WITHOUT_COMPLEX
elemental::scomplex
elemental::wrappers::blas::Dot
( int n, const elemental::scomplex* x, int incx,
         const elemental::scomplex* y, int incy )
{ 
    elemental::scomplex alpha = 0;
    for( int i=0; i<n; ++i ) 
        alpha += std::conj(x[i*incx])*y[i*incy];
    return alpha;
/*
#ifdef NO_COMPLEX_RETURN_FROM_BLAS
    elemental::scomplex alpha;
    BLAS(cdotc)( &alpha, &n, x, &incx, y, &incy );
    return alpha;
#else
    return BLAS(cdotc)( &n, x, &incx, y, &incy ); 
#endif
*/
}

elemental::dcomplex
elemental::wrappers::blas::Dot
( int n, const elemental::dcomplex* x, int incx,
         const elemental::dcomplex* y, int incy )
{
    elemental::dcomplex alpha = 0;
    for( int i=0; i<n; ++i ) 
        alpha += std::conj(x[i*incx])*y[i*incy];
    return alpha;
/*
#ifdef NO_COMPLEX_RETURN_FROM_BLAS
    elemental::dcomplex alpha;
    BLAS(zdotc)( &alpha, &n, x, &incx, y, &incy );
    return alpha;
#else
    return BLAS(zdotc)( &n, x, &incx, y, &incy ); 
#endif
*/
}
#endif // WITHOUT_COMPLEX

float
elemental::wrappers::blas::Dotc
( int n, const float* x, int incx, const float* y, int incy )
{ return BLAS(sdot)( &n, x, &incx, y, &incy ); }

double
elemental::wrappers::blas::Dotc
( int n, const double* x, int incx, const double* y, int incy )
{ return BLAS(ddot)( &n, x, &incx, y, &incy ); }

#ifndef WITHOUT_COMPLEX
elemental::scomplex
elemental::wrappers::blas::Dotc
( int n, const elemental::scomplex* x, int incx,
         const elemental::scomplex* y, int incy )
{ 
    elemental::scomplex alpha = 0;
    for( int i=0; i<n; ++i ) 
        alpha += std::conj(x[i*incx])*y[i*incy];
    return alpha;
/*
#ifdef NO_COMPLEX_RETURN_FROM_BLAS
    elemental::scomplex alpha;
    BLAS(cdotc)( &alpha, &n, x, &incx, y, &incy );
    return alpha;
#else
    return BLAS(cdotc)( &n, x, &incx, y, &incy ); 
#endif
*/
}

elemental::dcomplex
elemental::wrappers::blas::Dotc
( int n, const elemental::dcomplex* x, int incx,
         const elemental::dcomplex* y, int incy )
{ 
    elemental::dcomplex alpha = 0;
    for( int i=0; i<n; ++i ) 
        alpha += std::conj(x[i*incx])*y[i*incy];
    return alpha;
/*
#ifdef NO_COMPLEX_RETURN_FROM_BLAS
    elemental::dcomplex alpha;
    BLAS(zdotc)( &alpha, &n, x, &incx, y, &incy );
    return alpha;
#else
    return BLAS(zdotc)( &n, x, &incx, y, &incy ); 
#endif
*/
}
#endif // WITHOUT_COMPLEX

float
elemental::wrappers::blas::Dotu
( int n, const float* x, int incx, const float* y, int incy )
{ return BLAS(sdot)( &n, x, &incx, y, &incy ); }

double
elemental::wrappers::blas::Dotu
( int n, const double* x, int incx, const double* y, int incy )
{ return BLAS(ddot)( &n, x, &incx, y, &incy ); }

#ifndef WITHOUT_COMPLEX
elemental::scomplex
elemental::wrappers::blas::Dotu
( int n, const elemental::scomplex* x, int incx,
         const elemental::scomplex* y, int incy )
{
    elemental::scomplex alpha = 0;
    for( int i=0; i<n; ++i ) 
        alpha += x[i*incx]*y[i*incy];
    return alpha;
/* 
#ifdef NO_COMPLEX_RETURN_FROM_BLAS
    elemental::scomplex alpha;
    BLAS(cdotu)( &alpha, &n, x, &incx, y, &incy );
    return alpha;
#else
    return BLAS(cdotu)( &n, x, &incx, y, &incy ); 
#endif
*/
}

elemental::dcomplex
elemental::wrappers::blas::Dotu
( int n, const elemental::dcomplex* x, int incx,
         const elemental::dcomplex* y, int incy )
{
    elemental::dcomplex alpha = 0;
    for( int i=0; i<n; ++i ) 
        alpha += x[i*incx]*y[i*incy];
    return alpha;
/* 
#ifdef NO_COMPLEX_RETURN_FROM_BLAS
    elemental::dcomplex alpha;
    BLAS(zdotu)( &alpha, &n, x, &incx, y, &incy );
    return alpha;
#else
    return BLAS(zdotu)( &n, x, &incx, y, &incy ); 
#endif
*/
}
#endif // WITHOUT_COMPLEX

float
elemental::wrappers::blas::Nrm2
( int n, const float* x, int incx )
{ return BLAS(snrm2)( &n, x, &incx ); }

double
elemental::wrappers::blas::Nrm2
( int n, const double* x, int incx )
{ return BLAS(dnrm2)( &n, x, &incx ); }

#ifndef WITHOUT_COMPLEX
float
elemental::wrappers::blas::Nrm2
( int n, const scomplex* x, int incx )
{ return BLAS(scnrm2)( &n, x, &incx ); }

double
elemental::wrappers::blas::Nrm2
( int n, const dcomplex* x, int incx )
{ return BLAS(dznrm2)( &n, x, &incx ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Scal
( int n, float alpha, float* x, int incx )
{ BLAS(sscal)( &n, &alpha, x, &incx ); }

void
elemental::wrappers::blas::Scal
( int n, double alpha, double* x, int incx )
{ BLAS(dscal)( &n, &alpha, x, &incx ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Scal
( int n, scomplex alpha, scomplex* x, int incx )
{ BLAS(cscal)( &n, &alpha, x, &incx ); }

void
elemental::wrappers::blas::Scal
( int n, dcomplex alpha, dcomplex* x, int incx )
{ BLAS(zscal)( &n, &alpha, x, &incx ); }
#endif // WITHOUT_COMPLEX

//----------------------------------------------------------------------------//
// Level 2 BLAS                                                               //
//----------------------------------------------------------------------------//
void
elemental::wrappers::blas::Gemv
( char trans, int m, int n,
  float alpha, const float* A, int lda, const float* x, int incx,
  float beta,        float* y, int incy )
{
    const char fixedTrans = ( trans == 'C' ? 'T' : trans );
    BLAS(sgemv)
    ( &fixedTrans, &m, &n, &alpha, A, &lda, x, &incx, &beta, y, &incy );
}

void
elemental::wrappers::blas::Gemv
( char trans, int m, int n,
  double alpha, const double* A, int lda, const double* x, int incx,
  double beta,        double* y, int incy )
{
    const char fixedTrans = ( trans == 'C' ? 'T' : trans );
    BLAS(dgemv)
    ( &fixedTrans, &m, &n, &alpha, A, &lda, x, &incx, &beta, y, &incy );
}

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Gemv
( char trans, int m, int n,
  scomplex alpha, const scomplex* A, int lda, const scomplex* x, int incx,
  scomplex beta,        scomplex* y, int incy )
{ BLAS(cgemv)( &trans, &m, &n, &alpha, A, &lda, x, &incx, &beta, y, &incy ); }

void
elemental::wrappers::blas::Gemv
( char trans, int m, int n,
  dcomplex alpha, const dcomplex* A, int lda, const dcomplex* x, int incx,
  dcomplex beta,        dcomplex* y, int incy )
{ BLAS(zgemv)( &trans, &m, &n, &alpha, A, &lda, x, &incx, &beta, y, &incy ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Ger
( int m, int n,
  float alpha, const float* x, int incx, const float* y, int incy,
                     float* A, int lda )
{ BLAS(sger)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Ger
( int m, int n,
  double alpha, const double* x, int incx, const double* y, int incy,
                      double* A, int lda  )
{ BLAS(dger)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Ger
( int m, int n,
  scomplex alpha, const scomplex* x, int incx, const scomplex* y, int incy,
                        scomplex* A, int lda )
{ BLAS(cgerc)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Ger
( int m, int n,
  dcomplex alpha, const dcomplex* x, int incx, const dcomplex* y, int incy,
                        dcomplex* A, int lda )
{ BLAS(zgerc)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Gerc
( int m, int n,
  float alpha, const float* x, int incx, const float* y, int incy,
                     float* A, int lda )
{ BLAS(sger)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Gerc
( int m, int n,
  double alpha, const double* x, int incx, const double* y, int incy,
                      double* A, int lda )
{ BLAS(dger)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Gerc
( int m, int n,
  scomplex alpha, const scomplex* x, int incx, const scomplex* y, int incy,
                        scomplex* A, int lda )
{ BLAS(cgerc)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Gerc
( int m, int n,
  dcomplex alpha, const dcomplex* x, int incx, const dcomplex* y, int incy,
                        dcomplex* A, int lda )
{ BLAS(zgerc)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Geru
( int m, int n,
  float alpha, const float* x, int incx, const float* y, int incy,
                     float* A, int lda )
{ BLAS(sger)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Geru
( int m, int n,
  double alpha, const double* x, int incx, const double* y, int incy,
                      double* A, int lda )
{ BLAS(dger)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Geru
( int m, int n,
  scomplex alpha, const scomplex* x, int incx, const scomplex* y, int incy,
                        scomplex* A, int lda )
{ BLAS(cgeru)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Geru
( int m, int n,
  dcomplex alpha, const dcomplex* x, int incx, const dcomplex* y, int incy,
                        dcomplex* A, int lda )
{ BLAS(zgeru)( &m, &n, &alpha, x, &incx, y, &incy, A, &lda ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Hemv
( char uplo, int m,
  float alpha, const float* A, int lda, const float* x, int incx,
  float beta,        float* y, int incy )
{ BLAS(ssymv)( &uplo, &m, &alpha, A, &lda, x, &incx, &beta, y, &incy ); }

void
elemental::wrappers::blas::Hemv
( char uplo, int m,
  double alpha, const double* A, int lda, const double* x, int incx,
  double beta,        double* y, int incy )
{ BLAS(dsymv)( &uplo, &m, &alpha, A, &lda, x, &incx, &beta, y, &incy ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Hemv
( char uplo, int m,
  scomplex alpha, const scomplex* A, int lda, const scomplex* x, int incx,
  scomplex beta,        scomplex* y, int incy )
{ BLAS(chemv)( &uplo, &m, &alpha, A, &lda, x, &incx, &beta, y, &incy ); }

void
elemental::wrappers::blas::Hemv
( char uplo, int m,
  dcomplex alpha, const dcomplex* A, int lda, const dcomplex* x, int incx,
  dcomplex beta,        dcomplex* y, int incy )
{ BLAS(zhemv)( &uplo, &m, &alpha, A, &lda, x, &incx, &beta, y, &incy ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Her
( char uplo, int m,
  float alpha, const float* x, int incx, float* A, int lda )
{ BLAS(ssyr)( &uplo, &m, &alpha, x, &incx, A, &lda ); }

void
elemental::wrappers::blas::Her
( char uplo, int m,
  double alpha, const double* x, int incx, double* A, int lda )
{ BLAS(dsyr)( &uplo, &m, &alpha, x, &incx, A, &lda ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Her
( char uplo, int m,
  scomplex alpha, const scomplex* x, int incx, scomplex* A, int lda )
{ BLAS(cher)( &uplo, &m, &alpha, x, &incx, A, &lda ); }

void
elemental::wrappers::blas::Her
( char uplo, int m,
  dcomplex alpha, const dcomplex* x, int incx, dcomplex* A, int lda )
{ BLAS(zher)( &uplo, &m, &alpha, x, &incx, A, &lda ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Her2
( char uplo, int m,
  float alpha, const float* x, int incx, const float* y, int incy,
                     float* A, int lda )
{ BLAS(ssyr2)( &uplo, &m, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Her2
( char uplo, int m,
  double alpha, const double* x, int incx, const double* y, int incy,
                      double* A, int lda )
{ BLAS(dsyr2)( &uplo, &m, &alpha, x, &incx, y, &incy, A, &lda ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Her2
( char uplo, int m,
  scomplex alpha, const scomplex* x, int incx, const scomplex* y, int incy,
                        scomplex* A, int lda )
{ BLAS(cher2)( &uplo, &m, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Her2
( char uplo, int m,
  dcomplex alpha, const dcomplex* x, int incx, const dcomplex* y, int incy,
                        dcomplex* A, int lda )
{ BLAS(zher2)( &uplo, &m, &alpha, x, &incx, y, &incy, A, &lda ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Symv
( char uplo, int m,
  float alpha, const float* A, int lda, const float* x, int incx,
  float beta,        float* y, int incy )
{ BLAS(ssymv)( &uplo, &m, &alpha, A, &lda, x, &incx, &beta, y, &incy ); }

void
elemental::wrappers::blas::Symv
( char uplo, int m,
  double alpha, const double* A, int lda, const double* x, int incx,
  double beta,        double* y, int incy )
{ BLAS(dsymv)( &uplo, &m, &alpha, A, &lda, x, &incx, &beta, y, &incy ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Symv
( char uplo, int m,
  scomplex alpha, const scomplex* A, int lda, const scomplex* x, int incx,
  scomplex beta,        scomplex* y, int incy )
{
    // Recall that 'csymv' is an LAPACK auxiliary routine
    LAPACK(csymv)( &uplo, &m, &alpha, A, &lda, x, &incx, &beta, y, &incy );
}

void
elemental::wrappers::blas::Symv
( char uplo, int m,
  dcomplex alpha, const dcomplex* A, int lda, const dcomplex* x, int incx,
  dcomplex beta,        dcomplex* y, int incy )
{
    // Recall that 'zsymv' is an LAPACK auxiliary routine
    LAPACK(zsymv)( &uplo, &m, &alpha, A, &lda, x, &incx, &beta, y, &incy );
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Syr
( char uplo, int m,
  float alpha, const float* x, int incx, float* A, int lda  )
{ BLAS(ssyr)( &uplo, &m, &alpha, x, &incx, A, &lda ); }

void
elemental::wrappers::blas::Syr
( char uplo, int m,
  double alpha, const double* x, int incx, double* A, int lda )
{ BLAS(dsyr)( &uplo, &m, &alpha, x, &incx, A, &lda ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Syr
( char uplo, int m,
  scomplex alpha, const scomplex* x, int incx, scomplex* A, int lda )
{
    // Recall that 'csyr' is an LAPACK auxiliary routine
    LAPACK(csyr)( &uplo, &m, &alpha, x, &incx, A, &lda ); 
}

void
elemental::wrappers::blas::Syr
( char uplo, int m,
  dcomplex alpha, const dcomplex* x, int incx, dcomplex* A, int lda )
{
    // Recall that 'zsyr' is an LAPACK auxiliary routine
    LAPACK(zsyr)( &uplo, &m, &alpha, x, &incx, A, &lda ); 
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Syr2
( char uplo, int m,
  float alpha, const float* x, int incx, const float* y, int incy,
                     float* A, int lda )
{ BLAS(ssyr2)( &uplo, &m, &alpha, x, &incx, y, &incy, A, &lda ); }

void
elemental::wrappers::blas::Syr2
( char uplo, int m,
  double alpha, const double* x, int incx, const double* y, int incy,
                      double* A, int lda )
{ BLAS(dsyr2)( &uplo, &m, &alpha, x, &incx, y, &incy, A, &lda ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Syr2
( char uplo, int m,
  scomplex alpha, const scomplex* x, int incx, const scomplex* y, int incy,
                        scomplex* A, int lda )
{
    // csyr2 doesn't exist, so we route through csyr2k. However, csyr2k expects 
    // contiguous access of 'x', so we treat x and y as a row vectors where 
    // their leading dimensions are 'incx' and 'incy'. Thus we must perform 
    // A += x' y + y' x
    const char trans = 'T';
    const int k = 1;
    const scomplex beta = 1.;
    BLAS(csyr2k)
    ( &uplo, &trans, &m, &k, &alpha, x, &incx, y, &incy, &beta, A, &lda );
}

void
elemental::wrappers::blas::Syr2
( char uplo, int m,
  dcomplex alpha, const dcomplex* x, int incx, const dcomplex* y, int incy,
                        dcomplex* A, int lda )
{
    // zsyr2 doesn't exist, so we route through zsyr2k. However, zsyr2k expects 
    // contiguous access of 'x', so we treat x and y as a row vectors where 
    // their leading dimensions are 'incx' and 'incy'. Thus we must perform 
    // A += x' y + y' x
    const char trans = 'T';
    const int k = 1;
    const dcomplex beta = 1.;
    BLAS(zsyr2k)
    ( &uplo, &trans, &m, &k, &alpha, x, &incx, y, &incy, &beta, A, &lda );
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Trmv
( char uplo, char trans, char diag, int m,
  const float* A, int lda, float* x, int incx )
{ BLAS(strmv)( &uplo, &trans, &diag, &m, A, &lda, x, &incx ); }

void
elemental::wrappers::blas::Trmv
( char uplo, char trans, char diag, int m,
  const double* A, int lda, double* x, int incx )
{ BLAS(dtrmv)( &uplo, &trans, &diag, &m, A, &lda, x, &incx ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Trmv
( char uplo, char trans, char diag, int m,
  const scomplex* A, int lda, scomplex* x, int incx )
{ BLAS(ctrmv)( &uplo, &trans, &diag, &m, A, &lda, x, &incx ); }

void
elemental::wrappers::blas::Trmv
( char uplo, char trans, char diag, int m,
  const dcomplex* A, int lda, dcomplex* x, int incx )
{ BLAS(ztrmv)( &uplo, &trans, &diag, &m, A, &lda, x, &incx ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Trsv
( char uplo, char trans, char diag, int m,
  const float* A, int lda, float* x, int incx )
{ BLAS(strsv)( &uplo, &trans, &diag, &m, A, &lda, x, &incx ); }

void
elemental::wrappers::blas::Trsv
( char uplo, char trans, char diag, int m,
  const double* A, int lda, double* x, int incx )
{ BLAS(dtrsv)( &uplo, &trans, &diag, &m, A, &lda, x, &incx ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Trsv
( char uplo, char trans, char diag, int m,
  const scomplex* A, int lda, scomplex* x, int incx )
{ BLAS(ctrsv)( &uplo, &trans, &diag, &m, A, &lda, x, &incx ); }

void
elemental::wrappers::blas::Trsv
( char uplo, char trans, char diag, int m,
  const dcomplex* A, int lda, dcomplex* x, int incx )
{ BLAS(ztrsv)( &uplo, &trans, &diag, &m, A, &lda, x, &incx ); }
#endif // WITHOUT_COMPLEX

//----------------------------------------------------------------------------//
// Level 3 BLAS                                                               //
//----------------------------------------------------------------------------//
void
elemental::wrappers::blas::Gemm
( char transA, char transB, int m, int n, int k, 
  float alpha, const float* A, int lda, const float* B, int ldb,
  float beta,        float* C, int ldc )
{
    const char fixedTransA = ( transA == 'C' ? 'T' : transA );
    const char fixedTransB = ( transB == 'C' ? 'T' : transB );
    BLAS(sgemm)( &fixedTransA, &fixedTransB, &m, &n, &k,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Gemm
( char transA, char transB,
  int m, int n, int k, 
  double alpha, const double* A, int lda, const double* B, int ldb,
  double beta,        double* C, int ldc )
{
    const char fixedTransA = ( transA == 'C' ? 'T' : transA );
    const char fixedTransB = ( transB == 'C' ? 'T' : transB );
    BLAS(dgemm)( &fixedTransA, &fixedTransB, &m, &n, &k,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Gemm
( char transA, char transB, int m, int n, int k, 
  scomplex alpha, const scomplex* A, int lda, const scomplex* B, int ldb,
  scomplex beta,        scomplex* C, int ldc )
{
    BLAS(cgemm)( &transA, &transB, &m, &n, &k,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Gemm
( char transA, char transB, int m, int n, int k, 
  dcomplex alpha, const dcomplex* A, int lda, const dcomplex* B, int ldb,
  dcomplex beta,        dcomplex* C, int ldc )
{
    BLAS(zgemm)( &transA, &transB, &m, &n, &k,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Hemm
( char side, char uplo, int m, int n,
  float alpha, const float* A, int lda, const float* B, int ldb,
  float beta,        float* C, int ldc )
{
    BLAS(ssymm)( &side, &uplo, &m, &n,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Hemm
( char side, char uplo, int m, int n,
  double alpha, const double* A, int lda, const double* B, int ldb,
  double beta,        double* C, int ldc )
{
    BLAS(dsymm)( &side, &uplo, &m, &n,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Hemm
( char side, char uplo, int m, int n,
  scomplex alpha, const scomplex* A, int lda, const scomplex* B, int ldb,
  scomplex beta,        scomplex* C, int ldc )
{
    BLAS(chemm)( &side, &uplo, &m, &n,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Hemm
( char side, char uplo, int m, int n,
  dcomplex alpha, const dcomplex* A, int lda, const dcomplex* B, int ldb,
  dcomplex beta,        dcomplex* C, int ldc )
{
    BLAS(zhemm)( &side, &uplo, &m, &n,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Her2k
( char uplo, char trans, int n, int k,
  float alpha, const float* A, int lda, const float* B, int ldb,
  float beta,        float* C, int ldc )
{
    const char transFixed = ( trans == 'C' ? 'T' : trans );
    BLAS(ssyr2k)
    ( &uplo, &transFixed, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Her2k
( char uplo, char trans, int n, int k,
  double alpha, const double* A, int lda, const double* B, int ldb,
  double beta,        double* C, int ldc )
{
    const char transFixed = ( trans == 'C' ? 'T' : trans );
    BLAS(dsyr2k)
    ( &uplo, &transFixed, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Her2k
( char uplo, char trans, int n, int k,
  scomplex alpha, const scomplex* A, int lda, const scomplex* B, int ldb,
  scomplex beta,        scomplex* C, int ldc )
{
    BLAS(cher2k)
    ( &uplo, &trans, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Her2k
( char uplo, char trans, int n, int k,
  dcomplex alpha, const dcomplex* A, int lda, const dcomplex* B, int ldb,
  dcomplex beta,        dcomplex* C, int ldc )
{
    BLAS(zher2k)
    ( &uplo, &trans, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Herk
( char uplo, char trans, int n, int k,
  float alpha, const float* A, int lda,
  float beta,        float* C, int ldc )
{
    const char transFixed = ( trans == 'C' ? 'T' : trans );
    BLAS(ssyrk)( &uplo, &transFixed, &n, &k, &alpha, A, &lda, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Herk
( char uplo, char trans, int n, int k,
  double alpha, const double* A, int lda,
  double beta,        double* C, int ldc )
{
    const char transFixed = ( trans == 'C' ? 'T' : trans );
    BLAS(dsyrk)( &uplo, &transFixed, &n, &k, &alpha, A, &lda, &beta, C, &ldc );
}

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Herk
( char uplo, char trans, int n, int k,
  scomplex alpha, const scomplex* A, int lda,
  scomplex beta,        scomplex* C, int ldc )
{ BLAS(cherk)( &uplo, &trans, &n, &k, &alpha, A, &lda, &beta, C, &ldc ); }

void
elemental::wrappers::blas::Herk
( char uplo, char trans, int n, int k,
  dcomplex alpha, const dcomplex* A, int lda,
  dcomplex beta,        dcomplex* C, int ldc )
{ BLAS(zherk)( &uplo, &trans, &n, &k, &alpha, A, &lda, &beta, C, &ldc ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Symm
( char side, char uplo, int m, int n,
  float alpha, const float* A, int lda, const float* B, int ldb,
  float beta,        float* C, int ldc )
{
    BLAS(ssymm)( &side, &uplo, &m, &n,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Symm
( char side, char uplo, int m, int n,
  double alpha, const double* A, int lda, const double* B, int ldb,
  double beta,        double* C, int ldc )
{
    BLAS(dsymm)( &side, &uplo, &m, &n,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Symm
( char side, char uplo, int m, int n,
  scomplex alpha, const scomplex* A, int lda, const scomplex* B, int ldb,
  scomplex beta,        scomplex* C, int ldc )
{
    BLAS(csymm)( &side, &uplo, &m, &n,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Symm
( char side, char uplo, int m, int n,
  dcomplex alpha, const dcomplex* A, int lda, const dcomplex* B, int ldb,
  dcomplex beta,        dcomplex* C, int ldc )
{
    BLAS(zsymm)( &side, &uplo, &m, &n,
                 &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Syr2k
( char uplo, char trans, int n, int k,
  float alpha, const float* A, int lda, const float* B, int ldb,
  float beta,        float* C, int ldc )
{
    BLAS(ssyr2k)
    ( &uplo, &trans, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Syr2k
( char uplo, char trans, int n, int k,
  double alpha, const double* A, int lda, const double* B, int ldb,
  double beta,        double* C, int ldc )
{
    BLAS(dsyr2k)
    ( &uplo, &trans, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Syr2k
( char uplo, char trans, int n, int k,
  scomplex alpha, const scomplex* A, int lda, const scomplex* B, int ldb,
  scomplex beta,        scomplex* C, int ldc )
{
    BLAS(csyr2k)
    ( &uplo, &trans, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}

void
elemental::wrappers::blas::Syr2k
( char uplo, char trans, int n, int k,
  dcomplex alpha, const dcomplex* A, int lda, const dcomplex* B, int ldb,
  dcomplex beta,        dcomplex* C, int ldc )
{
    BLAS(zsyr2k)
    ( &uplo, &trans, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc );
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Syrk
( char uplo, char trans, int n, int k,
  float alpha, const float* A, int lda,
  float beta,        float* C, int ldc )
{ BLAS(ssyrk)( &uplo, &trans, &n, &k, &alpha, A, &lda, &beta, C, &ldc ); }

void
elemental::wrappers::blas::Syrk
( char uplo, char trans, int n, int k,
  double alpha, const double* A, int lda,
  double beta,        double* C, int ldc )
{ BLAS(dsyrk)( &uplo, &trans, &n, &k, &alpha, A, &lda, &beta, C, &ldc ); }

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Syrk
( char uplo, char trans, int n, int k,
  scomplex alpha, const scomplex* A, int lda,
  scomplex beta,        scomplex* C, int ldc )
{ BLAS(csyrk)( &uplo, &trans, &n, &k, &alpha, A, &lda, &beta, C, &ldc ); }

void
elemental::wrappers::blas::Syrk
( char uplo, char trans, int n, int k,
  dcomplex alpha, const dcomplex* A, int lda,
  dcomplex beta,        dcomplex* C, int ldc )
{ BLAS(zsyrk)( &uplo, &trans, &n, &k, &alpha, A, &lda, &beta, C, &ldc ); }
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Trmm
( char side, char uplo, char trans, char unit, int m, int n,
  float alpha, const float* A, int lda, float* B, int ldb )
{
    const char fixedTrans = ( trans == 'C' ? 'T' : trans );    
    BLAS(strmm)( &side, &uplo, &fixedTrans, &unit, &m, &n,
                 &alpha, A, &lda, B, &ldb );
}

void
elemental::wrappers::blas::Trmm
( char side, char uplo, char trans, char unit, int m, int n,
  double alpha, const double* A, int lda, double* B, int ldb )
{
    const char fixedTrans = ( trans == 'C' ? 'T' : trans );    
    BLAS(dtrmm)( &side, &uplo, &fixedTrans, &unit, &m, &n,
                 &alpha, A, &lda, B, &ldb );
}

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Trmm
( char side, char uplo, char trans, char unit, int m, int n,
  scomplex alpha, const scomplex* A, int lda, scomplex* B, int ldb )
{
    BLAS(ctrmm)( &side, &uplo, &trans, &unit, &m, &n,
                 &alpha, A, &lda, B, &ldb );
}

void
elemental::wrappers::blas::Trmm
( char side, char uplo, char trans, char unit, int m, int n,
  dcomplex alpha, const dcomplex* A, int lda, dcomplex* B, int ldb )
{
    BLAS(ztrmm)( &side, &uplo, &trans, &unit, &m, &n,
                 &alpha, A, &lda, B, &ldb );
}
#endif // WITHOUT_COMPLEX

void
elemental::wrappers::blas::Trsm
( char side, char uplo, char trans, char unit, int m, int n,
  float alpha, const float* A, int lda, float* B, int ldb )
{
    const char fixedTrans = ( trans == 'C' ? 'T' : trans );
    BLAS(strsm)( &side, &uplo, &fixedTrans, &unit, &m, &n,
                 &alpha, A, &lda, B, &ldb );
} 

void
elemental::wrappers::blas::Trsm
( char side, char uplo, char trans, char unit, int m, int n,
  double alpha, const double* A, int lda, double* B, int ldb )
{
    const char fixedTrans = ( trans == 'C' ? 'T' : trans );
    BLAS(dtrsm)( &side, &uplo, &fixedTrans, &unit, &m, &n,
                 &alpha, A, &lda, B, &ldb );
} 

#ifndef WITHOUT_COMPLEX
void
elemental::wrappers::blas::Trsm
( char side, char uplo, char trans, char unit, int m, int n,
  scomplex alpha, const scomplex* A, int lda, scomplex* B, int ldb )
{
    BLAS(ctrsm)( &side, &uplo, &trans, &unit, &m, &n,
                 &alpha, A, &lda, B, &ldb );
} 

void
elemental::wrappers::blas::Trsm
( char side, char uplo, char trans, char unit, int m, int n,
  dcomplex alpha, const dcomplex* A, int lda, dcomplex* B, int ldb )
{
    BLAS(ztrsm)( &side, &uplo, &trans, &unit, &m, &n,
                 &alpha, A, &lda, B, &ldb );
} 
#endif // WITHOUT_COMPLEX
