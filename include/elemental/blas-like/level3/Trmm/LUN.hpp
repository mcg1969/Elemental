/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   Copyright (c) 2013, The University of Texas at Austin
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef BLAS_TRMM_LUN_HPP
#define BLAS_TRMM_LUN_HPP

#include "elemental/blas-like/level1/Axpy.hpp"
#include "elemental/blas-like/level1/MakeTriangular.hpp"
#include "elemental/blas-like/level1/Scale.hpp"
#include "elemental/blas-like/level1/SetDiagonal.hpp"
#include "elemental/blas-like/level1/Transpose.hpp"
#include "elemental/blas-like/level3/Gemm.hpp"
#include "elemental/matrices/Zeros.hpp"

namespace elem {
namespace internal {

template<typename T>
inline void
LocalTrmmAccumulateLUN
( Orientation orientation, UnitOrNonUnit diag, T alpha,
  const DistMatrix<T,MC,  MR  >& U,
  const DistMatrix<T,STAR,MR  >& XTrans_STAR_MR,
        DistMatrix<T,MC,  STAR>& Z_MC_STAR )
{
#ifndef RELEASE
    PushCallStack("internal::LocalTrmmAccumulateLUN");
    if( U.Grid() != XTrans_STAR_MR.Grid() ||
        XTrans_STAR_MR.Grid() != Z_MC_STAR.Grid() )
        throw std::logic_error
        ("{U,X,Z} must be distributed over the same grid");
    if( U.Height() != U.Width() ||
        U.Height() != XTrans_STAR_MR.Width() ||
        U.Height() != Z_MC_STAR.Height() ||
        XTrans_STAR_MR.Height() != Z_MC_STAR.Width() )
    {
        std::ostringstream msg;
        msg << "Nonconformal LocalTrmmAccumulateLUN: \n"
            << "  U ~ " << U.Height() << " x " << U.Width() << "\n"
            << "  X^H/T[* ,MR] ~ " << XTrans_STAR_MR.Height() << " x "
                                   << XTrans_STAR_MR.Width() << "\n"
            << "  Z[MC,* ] ~ " << Z_MC_STAR.Height() << " x "
                               << Z_MC_STAR.Width() << "\n";
        throw std::logic_error( msg.str().c_str() );
    }
    if( XTrans_STAR_MR.RowAlignment() != U.RowAlignment() ||
        Z_MC_STAR.ColAlignment() != U.ColAlignment() )
        throw std::logic_error("Partial matrix distributions are misaligned");
#endif
    const Grid& g = U.Grid();

    // Matrix views
    DistMatrix<T>
        UTL(g), UTR(g),  U00(g), U01(g), U02(g),
        UBL(g), UBR(g),  U10(g), U11(g), U12(g),
                         U20(g), U21(g), U22(g);

    DistMatrix<T> D11(g);

    DistMatrix<T,STAR,MR>
        XLTrans_STAR_MR(g), XRTrans_STAR_MR(g),
        X0Trans_STAR_MR(g), X1Trans_STAR_MR(g), 
        X2Trans_STAR_MR(g);

    DistMatrix<T,MC,STAR>
        ZT_MC_STAR(g),  Z0_MC_STAR(g),
        ZB_MC_STAR(g),  Z1_MC_STAR(g),
                        Z2_MC_STAR(g);

    const int ratio = std::max( g.Height(), g.Width() );
    PushBlocksizeStack( ratio*Blocksize() );

    LockedPartitionDownDiagonal
    ( U, UTL, UTR,
         UBL, UBR, 0 );
    LockedPartitionRight
    ( XTrans_STAR_MR, XLTrans_STAR_MR, XRTrans_STAR_MR, 0 );
    PartitionDown
    ( Z_MC_STAR, ZT_MC_STAR,
                 ZB_MC_STAR, 0 );
    while( UTL.Height() < U.Height() )
    {
        LockedRepartitionDownDiagonal
        ( UTL, /**/ UTR,  U00, /**/ U01, U02,
         /*************/ /******************/
               /**/       U10, /**/ U11, U12,
          UBL, /**/ UBR,  U20, /**/ U21, U22 );

        LockedRepartitionRight
        ( XLTrans_STAR_MR, /**/ XRTrans_STAR_MR,
          X0Trans_STAR_MR, /**/ X1Trans_STAR_MR, X2Trans_STAR_MR );

        RepartitionDown
        ( ZT_MC_STAR,  Z0_MC_STAR,
         /**********/ /**********/
                       Z1_MC_STAR,
          ZB_MC_STAR,  Z2_MC_STAR );

        D11.AlignWith( U11 );
        //--------------------------------------------------------------------//
        D11 = U11;
        MakeTriangular( UPPER, D11 );
        if( diag == UNIT )
            SetDiagonal( D11, T(1) );
        LocalGemm
        ( NORMAL, orientation, alpha, D11, X1Trans_STAR_MR, T(1), Z1_MC_STAR );
        LocalGemm
        ( NORMAL, orientation, alpha, U01, X1Trans_STAR_MR, T(1), Z0_MC_STAR );
        //--------------------------------------------------------------------//
        D11.FreeAlignments();

        SlideLockedPartitionDownDiagonal
        ( UTL, /**/ UTR,  U00, U01, /**/ U02,
               /**/       U10, U11, /**/ U12,
         /*************/ /******************/
          UBL, /**/ UBR,  U20, U21, /**/ U22 );

        SlideLockedPartitionRight
        ( XLTrans_STAR_MR,                  /**/ XRTrans_STAR_MR,
          X0Trans_STAR_MR, X1Trans_STAR_MR, /**/ X2Trans_STAR_MR );

        SlidePartitionDown
        ( ZT_MC_STAR,  Z0_MC_STAR,
                       Z1_MC_STAR,
         /**********/ /**********/
          ZB_MC_STAR,  Z2_MC_STAR );
    }
    PopBlocksizeStack();
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
TrmmLUNA
( UnitOrNonUnit diag,
  T alpha, const DistMatrix<T>& U,
                 DistMatrix<T>& X )
{
#ifndef RELEASE
    PushCallStack("internal::TrmmULNA");
    if( U.Grid() != X.Grid() )
        throw std::logic_error
        ("U and X must be distributed over the same grid");
    if( U.Height() != U.Width() || U.Width() != X.Height() )
    {
        std::ostringstream msg;
        msg << "Nonconformal TrmmLUNA: \n"
            << "  U ~ " << U.Height() << " x " << U.Width() << "\n"
            << "  X ~ " << X.Height() << " x " << X.Width() << "\n";
        throw std::logic_error( msg.str().c_str() );
    }
#endif
    const Grid& g = U.Grid();

    DistMatrix<T>
        XL(g), XR(g),
        X0(g), X1(g), X2(g);

    DistMatrix<T,VR,  STAR> X1_VR_STAR(g);
    DistMatrix<T,STAR,MR  > X1Trans_STAR_MR(g);
    DistMatrix<T,MC,  STAR> Z1_MC_STAR(g);

    X1_VR_STAR.AlignWith( U );
    X1Trans_STAR_MR.AlignWith( U );
    Z1_MC_STAR.AlignWith( U );

    PartitionRight( X, XL, XR, 0 );
    while( XL.Width() < X.Width() )
    {
        RepartitionRight
        ( XL, /**/ XR,
          X0, /**/ X1, X2 );

        Zeros( X1.Height(), X1.Width(), Z1_MC_STAR );
        //--------------------------------------------------------------------//
        X1_VR_STAR = X1;
        X1Trans_STAR_MR.TransposeFrom( X1_VR_STAR );
        LocalTrmmAccumulateLUN
        ( TRANSPOSE, diag, alpha, U, X1Trans_STAR_MR, Z1_MC_STAR );

        X1.SumScatterFrom( Z1_MC_STAR );
        //--------------------------------------------------------------------//

        SlidePartitionRight
        ( XL,     /**/ XR,
          X0, X1, /**/ X2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
TrmmLUNCOld
( UnitOrNonUnit diag,
  T alpha, const DistMatrix<T>& U,
                 DistMatrix<T>& X )
{
#ifndef RELEASE
    PushCallStack("internal::TrmmLUNCOld");
    if( U.Grid() != X.Grid() )
        throw std::logic_error
        ("U and X must be distributed over the same grid");
    if( U.Height() != U.Width() || U.Width() != X.Height() )
    {
        std::ostringstream msg;
        msg << "Nonconformal TrmmLUN: \n"
            << "  U ~ " << U.Height() << " x " << U.Width() << "\n"
            << "  X ~ " << X.Height() << " x " << X.Width() << "\n";
        throw std::logic_error( msg.str().c_str() );
    }
#endif
    const Grid& g = U.Grid();

    // Matrix views
    DistMatrix<T> 
        UTL(g), UTR(g),  U00(g), U01(g), U02(g),
        UBL(g), UBR(g),  U10(g), U11(g), U12(g),
                         U20(g), U21(g), U22(g);
    DistMatrix<T> XT(g),  X0(g),
                  XB(g),  X1(g),
                          X2(g);

    // Temporary distributions
    DistMatrix<T,STAR,STAR> U11_STAR_STAR(g);
    DistMatrix<T,STAR,MC  > U12_STAR_MC(g);
    DistMatrix<T,STAR,VR  > X1_STAR_VR(g);
    DistMatrix<T,MR,  STAR> D1Trans_MR_STAR(g);
    DistMatrix<T,MR,  MC  > D1Trans_MR_MC(g);
    DistMatrix<T,MC,  MR  > D1(g);

    // Start the algorithm
    Scale( alpha, X );
    LockedPartitionDownDiagonal
    ( U, UTL, UTR,
         UBL, UBR, 0 );
    PartitionDown
    ( X, XT,
         XB, 0 );
    while( XB.Height() > 0 )
    {
        LockedRepartitionDownDiagonal
        ( UTL, /**/ UTR,   U00, /**/ U01, U02,
         /*************/  /******************/
               /**/        U10, /**/ U11, U12,
          UBL, /**/ UBR,   U20, /**/ U21, U22 );

        RepartitionDown
        ( XT,  X0,
         /**/ /**/
               X1,
          XB,  X2 );

        U12_STAR_MC.AlignWith( X2 );
        D1Trans_MR_STAR.AlignWith( X1 );
        D1Trans_MR_MC.AlignWith( X1 );
        D1.AlignWith( X1 );
        Zeros( X1.Width(), X1.Height(), D1Trans_MR_STAR );
        Zeros( X1.Height(), X1.Width(), D1 );
        //--------------------------------------------------------------------//
        X1_STAR_VR = X1;
        U11_STAR_STAR = U11;
        LocalTrmm
        ( LEFT, UPPER, NORMAL, diag, T(1), U11_STAR_STAR, X1_STAR_VR );
        X1 = X1_STAR_VR;
 
        U12_STAR_MC = U12;
        LocalGemm
        ( TRANSPOSE, TRANSPOSE, T(1), X2, U12_STAR_MC, T(0), D1Trans_MR_STAR );
        D1Trans_MR_MC.SumScatterFrom( D1Trans_MR_STAR );
        Transpose( D1Trans_MR_MC.Matrix(), D1.Matrix() );
        Axpy( T(1), D1, X1 );
        //--------------------------------------------------------------------//
        D1.FreeAlignments();
        D1Trans_MR_MC.FreeAlignments();
        D1Trans_MR_STAR.FreeAlignments();
        U12_STAR_MC.FreeAlignments();

        SlideLockedPartitionDownDiagonal
        ( UTL, /**/ UTR,  U00, U01, /**/ U02,
               /**/       U10, U11, /**/ U12,
         /*************/ /******************/
          UBL, /**/ UBR,  U20, U21, /**/ U22 );

        SlidePartitionDown
        ( XT,  X0,
               X1,
         /**/ /**/
          XB,  X2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
TrmmLUNC
( UnitOrNonUnit diag,
  T alpha, const DistMatrix<T>& U,
                 DistMatrix<T>& X )
{
#ifndef RELEASE
    PushCallStack("internal::TrmmLUNC");
    if( U.Grid() != X.Grid() )
        throw std::logic_error
        ("U and X must be distributed over the same grid");
    if( U.Height() != U.Width() || U.Width() != X.Height() )
    {
        std::ostringstream msg;
        msg << "Nonconformal TrmmLUN: \n"
            << "  U ~ " << U.Height() << " x " << U.Width() << "\n"
            << "  X ~ " << X.Height() << " x " << X.Width() << "\n";
        throw std::logic_error( msg.str().c_str() );
    }
#endif
    const Grid& g = U.Grid();

    // Matrix views
    DistMatrix<T> 
        UTL(g), UTR(g),  U00(g), U01(g), U02(g),
        UBL(g), UBR(g),  U10(g), U11(g), U12(g),
                         U20(g), U21(g), U22(g);
    DistMatrix<T> XT(g),  X0(g),
                  XB(g),  X1(g),
                          X2(g);

    // Temporary distributions
    DistMatrix<T,STAR,STAR> U11_STAR_STAR(g);
    DistMatrix<T,MC,  STAR> U01_MC_STAR(g);
    DistMatrix<T,STAR,VR  > X1_STAR_VR(g);
    DistMatrix<T,MR,  STAR> X1Trans_MR_STAR(g);

    // Start the algorithm
    Scale( alpha, X );
    LockedPartitionDownDiagonal
    ( U, UTL, UTR,
         UBL, UBR, 0 );
    PartitionDown
    ( X, XT,
         XB, 0 );
    while( XB.Height() > 0 )
    {
        LockedRepartitionDownDiagonal
        ( UTL, /**/ UTR,   U00, /**/ U01, U02,
         /*************/  /******************/
               /**/        U10, /**/ U11, U12,
          UBL, /**/ UBR,   U20, /**/ U21, U22 );

        RepartitionDown
        ( XT,  X0,
         /**/ /**/
               X1,
          XB,  X2 );

        U01_MC_STAR.AlignWith( X0 );
        X1Trans_MR_STAR.AlignWith( X0 );
        X1_STAR_VR.AlignWith( X1 );
        //--------------------------------------------------------------------//
        U01_MC_STAR = U01;
        X1Trans_MR_STAR.TransposeFrom( X1 );
        LocalGemm
        ( NORMAL, TRANSPOSE, T(1), U01_MC_STAR, X1Trans_MR_STAR, T(1), X0 );

        U11_STAR_STAR = U11;
        X1_STAR_VR.TransposeFrom( X1Trans_MR_STAR );
        LocalTrmm( LEFT, UPPER, NORMAL, diag, T(1), U11_STAR_STAR, X1_STAR_VR );
        X1 = X1_STAR_VR;
        //--------------------------------------------------------------------//
        U01_MC_STAR.FreeAlignments();
        X1Trans_MR_STAR.FreeAlignments();
        X1_STAR_VR.FreeAlignments();

        SlideLockedPartitionDownDiagonal
        ( UTL, /**/ UTR,  U00, U01, /**/ U02,
               /**/       U10, U11, /**/ U12,
         /*************/ /******************/
          UBL, /**/ UBR,  U20, U21, /**/ U22 );

        SlidePartitionDown
        ( XT,  X0,
               X1,
         /**/ /**/
          XB,  X2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

// Left Upper Normal (Non)Unit Trmm
//   X := triu(U)  X, or
//   X := triuu(U) X
template<typename T>
inline void
TrmmLUN
( UnitOrNonUnit diag,
  T alpha, const DistMatrix<T>& U,
                 DistMatrix<T>& X )
{
#ifndef RELEASE
    PushCallStack("internal::TrmmLUN");
#endif
    // TODO: Come up with a better routing mechanism
    if( U.Height() > 5*X.Width() )
        TrmmLUNA( diag, alpha, U, X );
    else
        TrmmLUNC( diag, alpha, U, X );
#ifndef RELEASE
    PopCallStack();
#endif
}

} // namespace internal
} // namespace elem

#endif // ifndef BLAS_TRMM_LUN_HPP
