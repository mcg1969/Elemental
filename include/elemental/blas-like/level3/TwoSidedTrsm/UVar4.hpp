/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef BLAS_TWOSIDEDTRSM_UVAR4_HPP
#define BLAS_TWOSIDEDTRSM_UVAR4_HPP

#include "elemental/blas-like/level1/Axpy.hpp"
#include "elemental/blas-like/level3/Gemm.hpp"
#include "elemental/blas-like/level3/Hemm.hpp"
#include "elemental/blas-like/level3/Her2k.hpp"
#include "elemental/blas-like/level3/Trsm.hpp"
#include "elemental/matrices/Zeros.hpp"

namespace elem {
namespace internal {

template<typename F> 
inline void
TwoSidedTrsmUVar4( UnitOrNonUnit diag, Matrix<F>& A, const Matrix<F>& U )
{
#ifndef RELEASE
    PushCallStack("internal::TwoSidedTrsmUVar4");
    if( A.Height() != A.Width() )
        throw std::logic_error("A must be square");
    if( U.Height() != U.Width() )
        throw std::logic_error("Triangular matrices must be square");
    if( A.Height() != U.Height() )
        throw std::logic_error("A and U must be the same size");
#endif
    // Matrix views
    Matrix<F>
        ATL, ATR,  A00, A01, A02,
        ABL, ABR,  A10, A11, A12,
                   A20, A21, A22;
    Matrix<F>
        UTL, UTR,  U00, U01, U02,
        UBL, UBR,  U10, U11, U12,
                   U20, U21, U22;

    // Temporary products
    Matrix<F> Y12;

    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    LockedPartitionDownDiagonal
    ( U, UTL, UTR,
         UBL, UBR, 0 );
    while( ATL.Height() < A.Height() )
    {
        RepartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, /**/ A01, A02,
         /*************/ /******************/
               /**/       A10, /**/ A11, A12,
          ABL, /**/ ABR,  A20, /**/ A21, A22 );

        LockedRepartitionDownDiagonal
        ( UTL, /**/ UTR,  U00, /**/ U01, U02,
         /*************/ /******************/
               /**/       U10, /**/ U11, U12,
          UBL, /**/ UBR,  U20, /**/ U21, U22 );

        //--------------------------------------------------------------------//
        // A01 := A01 inv(U11)
        Trsm( RIGHT, UPPER, NORMAL, diag, F(1), U11, A01 );

        // A11 := inv(U11)' A11 inv(U11)
        TwoSidedTrsmUUnb( diag, A11, U11 );

        // A02 := A02 - A01 U12
        Gemm( NORMAL, NORMAL, F(-1), A01, U12, F(1), A02 );

        // Y12 := A11 U12
        Zeros( A12.Height(), A12.Width(), Y12 );
        Hemm( LEFT, UPPER, F(1), A11, U12, F(0), Y12 );

        // A12 := inv(U11)' A12
        Trsm( LEFT, UPPER, ADJOINT, diag, F(1), U11, A12 );

        // A12 := A12 - 1/2 Y12
        Axpy( F(-1)/F(2), Y12, A12 );

        // A22 := A22 - (A12' U12 + U12' A12)
        Her2k( UPPER, ADJOINT, F(-1), A12, U12, F(1), A22 );

        // A12 := A12 - 1/2 Y12
        Axpy( F(-1)/F(2), Y12, A12 );
        //--------------------------------------------------------------------//

        SlidePartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, A01, /**/ A02,
               /**/       A10, A11, /**/ A12,
         /*************/ /******************/
          ABL, /**/ ABR,  A20, A21, /**/ A22 );

        SlideLockedPartitionDownDiagonal
        ( UTL, /**/ UTR,  U00, U01, /**/ U02,
               /**/       U10, U11, /**/ U12,
         /**********************************/
          UBL, /**/ UBR,  U20, U21, /**/ U22 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename F> 
inline void
TwoSidedTrsmUVar4
( UnitOrNonUnit diag, DistMatrix<F>& A, const DistMatrix<F>& U )
{
#ifndef RELEASE
    PushCallStack("internal::TwoSidedTrsmUVar4");
    if( A.Height() != A.Width() )
        throw std::logic_error("A must be square");
    if( U.Height() != U.Width() )
        throw std::logic_error("Triangular matrices must be square");
    if( A.Height() != U.Height() )
        throw std::logic_error("A and U must be the same size");
#endif
    const Grid& g = A.Grid();

    // Matrix views
    DistMatrix<F>
        ATL(g), ATR(g),  A00(g), A01(g), A02(g),
        ABL(g), ABR(g),  A10(g), A11(g), A12(g),
                         A20(g), A21(g), A22(g);
    DistMatrix<F>
        UTL(g), UTR(g),  U00(g), U01(g), U02(g),
        UBL(g), UBR(g),  U10(g), U11(g), U12(g),
                         U20(g), U21(g), U22(g);

    // Temporary distributions
    DistMatrix<F,VC,  STAR> A01_VC_STAR(g);
    DistMatrix<F,STAR,MC  > A01Trans_STAR_MC(g);
    DistMatrix<F,STAR,STAR> A11_STAR_STAR(g);
    DistMatrix<F,STAR,VR  > A12_STAR_VR(g);
    DistMatrix<F,STAR,VC  > A12_STAR_VC(g);
    DistMatrix<F,STAR,MC  > A12_STAR_MC(g);
    DistMatrix<F,STAR,MR  > A12_STAR_MR(g);
    DistMatrix<F,STAR,STAR> U11_STAR_STAR(g);
    DistMatrix<F,MR,  STAR> U12Trans_MR_STAR(g);
    DistMatrix<F,VR,  STAR> U12Trans_VR_STAR(g);
    DistMatrix<F,STAR,VR  > U12_STAR_VR(g);
    DistMatrix<F,STAR,VC  > U12_STAR_VC(g);
    DistMatrix<F,STAR,MC  > U12_STAR_MC(g);
    DistMatrix<F,STAR,VR  > Y12_STAR_VR(g);

    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    LockedPartitionDownDiagonal
    ( U, UTL, UTR,
         UBL, UBR, 0 );
    while( ATL.Height() < A.Height() )
    {
        RepartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, /**/ A01, A02,
         /*************/ /******************/
               /**/       A10, /**/ A11, A12,
          ABL, /**/ ABR,  A20, /**/ A21, A22 );

        LockedRepartitionDownDiagonal
        ( UTL, /**/ UTR,  U00, /**/ U01, U02,
         /*************/ /******************/
               /**/       U10, /**/ U11, U12,
          UBL, /**/ UBR,  U20, /**/ U21, U22 );

        A01_VC_STAR.AlignWith( A02 );
        A01Trans_STAR_MC.AlignWith( A02 );
        A12_STAR_VR.AlignWith( A22 );
        A12_STAR_VC.AlignWith( A22 );
        A12_STAR_MC.AlignWith( A22 );
        A12_STAR_MR.AlignWith( A22 );
        U12Trans_MR_STAR.AlignWith( A02 );
        U12Trans_VR_STAR.AlignWith( A02 );
        U12_STAR_VR.AlignWith( A02 );
        U12_STAR_VC.AlignWith( A22 );
        U12_STAR_MC.AlignWith( A22 );
        Y12_STAR_VR.AlignWith( A12 );
        //--------------------------------------------------------------------//
        // A01 := A01 inv(U11)
        A01_VC_STAR = A01;
        U11_STAR_STAR = U11;
        LocalTrsm
        ( RIGHT, UPPER, NORMAL, diag, F(1), U11_STAR_STAR, A01_VC_STAR );
        A01 = A01_VC_STAR;

        // A11 := inv(U11)' A11 inv(U11)
        A11_STAR_STAR = A11;
        LocalTwoSidedTrsm( UPPER, diag, A11_STAR_STAR, U11_STAR_STAR );
        A11 = A11_STAR_STAR;

        // A02 := A02 - A01 U12
        A01Trans_STAR_MC.TransposeFrom( A01_VC_STAR );
        U12Trans_MR_STAR.TransposeFrom( U12 );
        LocalGemm
        ( TRANSPOSE, TRANSPOSE, 
          F(-1), A01Trans_STAR_MC, U12Trans_MR_STAR, F(1), A02 );

        // Y12 := A11 U12
        U12Trans_VR_STAR = U12Trans_MR_STAR;
        Zeros( A12.Height(), A12.Width(), U12_STAR_VR );
        Transpose( U12Trans_VR_STAR.Matrix(), U12_STAR_VR.Matrix() );
        Zeros( A12.Height(), A12.Width(), Y12_STAR_VR );
        Hemm
        ( LEFT, UPPER, 
          F(1), A11_STAR_STAR.Matrix(), U12_STAR_VR.Matrix(), 
          F(0), Y12_STAR_VR.Matrix() );

        // A12 := inv(U11)' A12
        A12_STAR_VR = A12;
        LocalTrsm
        ( LEFT, UPPER, ADJOINT, diag, F(1), U11_STAR_STAR, A12_STAR_VR );

        // A12 := A12 - 1/2 Y12
        Axpy( F(-1)/F(2), Y12_STAR_VR, A12_STAR_VR );

        // A22 := A22 - (A12' U12 + U12' A12)
        A12_STAR_MR = A12_STAR_VR;
        A12_STAR_VC = A12_STAR_VR;
        U12_STAR_VC = U12_STAR_VR;
        A12_STAR_MC = A12_STAR_VC;
        U12_STAR_MC = U12_STAR_VC;
        LocalTrr2k
        ( UPPER, ADJOINT, TRANSPOSE, ADJOINT,
          F(-1), A12_STAR_MC, U12Trans_MR_STAR,
                 U12_STAR_MC, A12_STAR_MR,
          F(1), A22 );

        // A12 := A12 - 1/2 Y12
        Axpy( F(-1)/F(2), Y12_STAR_VR, A12_STAR_VR );
        A12 = A12_STAR_VR;
        //--------------------------------------------------------------------//
        A01_VC_STAR.FreeAlignments();
        A01Trans_STAR_MC.FreeAlignments();
        A12_STAR_VR.FreeAlignments();
        A12_STAR_VC.FreeAlignments();
        A12_STAR_MC.FreeAlignments();
        A12_STAR_MR.FreeAlignments();
        U12Trans_MR_STAR.FreeAlignments();
        U12Trans_VR_STAR.FreeAlignments();
        U12_STAR_VR.FreeAlignments();
        U12_STAR_VC.FreeAlignments();
        U12_STAR_MC.FreeAlignments();
        Y12_STAR_VR.FreeAlignments();

        SlidePartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, A01, /**/ A02,
               /**/       A10, A11, /**/ A12,
         /*************/ /******************/
          ABL, /**/ ABR,  A20, A21, /**/ A22 );

        SlideLockedPartitionDownDiagonal
        ( UTL, /**/ UTR,  U00, U01, /**/ U02,
               /**/       U10, U11, /**/ U12,
         /**********************************/
          UBL, /**/ UBR,  U20, U21, /**/ U22 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

} // namespace internal
} // namespace elem

#endif // ifndef BLAS_TWOSIDEDTRSM_UVAR4_HPP
