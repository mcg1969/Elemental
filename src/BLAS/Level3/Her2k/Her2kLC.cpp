/*
   Copyright 2009-2010 Jack Poulson

   This file is part of Elemental.

   Elemental is free software: you can redistribute it and/or modify it under
   the terms of the GNU Lesser General Public License as published by the
   Free Software Foundation; either version 3 of the License, or 
   (at your option) any later version.

   Elemental is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with Elemental. If not, see <http://www.gnu.org/licenses/>.
*/
#include "ElementalBLASInternal.h"
using namespace std;
using namespace Elemental;

template<typename T>
void
Elemental::BLAS::Internal::Her2kLC
( const T alpha, const DistMatrix<T,MC,MR>& A,
                 const DistMatrix<T,MC,MR>& B,
  const T beta,        DistMatrix<T,MC,MR>& C )
{
#ifndef RELEASE
    PushCallStack("BLAS::Internal::Her2kLC");
    if( A.GetGrid() != B.GetGrid() || B.GetGrid() != C.GetGrid() )
        throw "{A,B,C} must be distributed over the same grid.";
    if( A.Width() != C.Height() || 
        A.Width() != C.Width()  ||
        B.Width() != C.Height() ||
        B.Width() != C.Width()  ||
        A.Height() != B.Height()  )
    {
        ostringstream msg;
        msg << "Nonconformal Her2kLC:" << endl
            << "  A ~ " << A.Height() << " x " << A.Width() << endl
            << "  B ~ " << B.Height() << " x " << B.Width() << endl
            << "  C ~ " << C.Height() << " x " << C.Width() << endl;
        throw msg.str();
    }
#endif
    const Grid& grid = A.GetGrid();

    // Matrix views
    DistMatrix<T,MC,MR> AT(grid),  A0(grid),
                        AB(grid),  A1(grid),
                                   A2(grid);

    DistMatrix<T,MC,MR> BT(grid),  B0(grid),
                        BB(grid),  B1(grid),
                                   B2(grid);

    // Temporary distributions
    DistMatrix<T,Star,MC> A1_Star_MC(grid);
    DistMatrix<T,Star,MR> A1_Star_MR(grid);
    DistMatrix<T,Star,MC> B1_Star_MC(grid);
    DistMatrix<T,Star,MR> B1_Star_MR(grid);

    // Start the algorithm
    BLAS::Scal( beta, C );
    LockedPartitionDown( A, AT, 
                            AB );
    LockedPartitionDown( B, BT,
                            BB );
    while( AB.Height() > 0 )
    {
        LockedRepartitionDown( AT,  A0,
                              /**/ /**/
                                    A1,
                               AB,  A2 );

        LockedRepartitionDown( BT,  B0,
                              /**/ /**/
                                    B1,
                               BB,  B2 );

        A1_Star_MC.AlignWith( C );
        A1_Star_MR.AlignWith( C );
        B1_Star_MC.AlignWith( C );
        B1_Star_MR.AlignWith( C );
        //--------------------------------------------------------------------//
        A1_Star_MR = A1;
        A1_Star_MC = A1_Star_MR;
        B1_Star_MR = B1;
        B1_Star_MC = B1_Star_MR;

        BLAS::Internal::Her2kLCUpdate
        ( alpha, A1_Star_MC, A1_Star_MR, 
                 B1_Star_MC, B1_Star_MR, (T)1, C ); 
        //--------------------------------------------------------------------//
        A1_Star_MC.FreeConstraints();
        A1_Star_MR.FreeConstraints();
        B1_Star_MC.FreeConstraints();
        B1_Star_MR.FreeConstraints();

        SlideLockedPartitionDown( AT,  A0,
                                       A1,
                                 /**/ /**/
                                  AB,  A2 );

        SlideLockedPartitionDown( BT,  B0,
                                       B1,
                                 /**/ /**/
                                  BB,  B2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
Elemental::BLAS::Internal::Her2kLCUpdate
( const T alpha, const DistMatrix<T,Star,MC>& A_Star_MC,
                 const DistMatrix<T,Star,MR>& A_Star_MR,
                 const DistMatrix<T,Star,MC>& B_Star_MC,
                 const DistMatrix<T,Star,MR>& B_Star_MR,
  const T beta,        DistMatrix<T,MC,  MR>& C         )
{
#ifndef RELEASE
    PushCallStack("BLAS::Internal::Her2kLCUpdate");
    if( A_Star_MC.GetGrid() != A_Star_MR.GetGrid() || 
        A_Star_MR.GetGrid() != B_Star_MC.GetGrid() ||
        B_Star_MC.GetGrid() != B_Star_MR.GetGrid() ||
        B_Star_MR.GetGrid() != C.GetGrid()           )
    {
        throw "{A,B,C} must be distributed over the same grid.";
    }
    if( A_Star_MC.Width() != C.Height() ||
        A_Star_MR.Width() != C.Width()  ||
        B_Star_MC.Width() != C.Height() ||
        B_Star_MR.Width() != C.Width()  ||
        A_Star_MC.Height() != A_Star_MR.Height() ||
        A_Star_MC.Width()  != A_Star_MR.Width()  ||  
        B_Star_MC.Height() != B_Star_MR.Height() ||
        B_Star_MC.Width()  != B_Star_MR.Width()     )
    {
        ostringstream msg;
        msg << "Nonconformal Her2kLCUpdate: " << endl
            << "  A[* ,MC] ~ " << A_Star_MC.Height() << " x "
                               << A_Star_MC.Width()  << endl
            << "  A[* ,MR] ~ " << A_Star_MR.Height() << " x "
                               << A_Star_MR.Width()  << endl
            << "  B[* ,MC] ~ " << B_Star_MC.Height() << " x "
                               << B_Star_MC.Width()  << endl
            << "  B[* ,MR] ~ " << B_Star_MR.Height() << " x "
                               << B_Star_MR.Width()  << endl
            << "  C[MC,MR] ~ " << C.Height() << " x " << C.Width() << endl;
        throw msg.str();
    }
    if( A_Star_MC.RowAlignment() != C.ColAlignment() ||
        A_Star_MR.RowAlignment() != C.RowAlignment() ||  
        B_Star_MC.RowAlignment() != C.ColAlignment() ||
        B_Star_MR.RowAlignment() != C.RowAlignment()    )
    {
        ostringstream msg;
        msg << "Misaligned Her2kLCUpdate: " << endl
            << "  A[* ,MC] ~ " << A_Star_MC.RowAlignment() << endl
            << "  A[* ,MR] ~ " << A_Star_MR.RowAlignment() << endl
            << "  B[* ,MC] ~ " << B_Star_MC.RowAlignment() << endl
            << "  B[* ,MR] ~ " << B_Star_MR.RowAlignment() << endl
            << "  C[MC,MR] ~ " << C.ColAlignment() << " , " << 
                                  C.RowAlignment() << endl;
        throw msg.str();
    }
#endif
    const Grid& grid = C.GetGrid();

    if( C.Height() < 2*grid.Width()*Blocksize() )
    {
        BLAS::Internal::Her2kLCUpdateKernel
        ( alpha, A_Star_MC, A_Star_MR, B_Star_MC, B_Star_MR, beta, C );
    }
    else
    {
        // Split C in four roughly equal pieces, perform a large gemm on CBL
        // and recurse on CTL and CBR.

        DistMatrix<T,Star,MC> AL_Star_MC(grid), AR_Star_MC(grid);
        DistMatrix<T,Star,MR> AL_Star_MR(grid), AR_Star_MR(grid);
        DistMatrix<T,Star,MC> BL_Star_MC(grid), BR_Star_MC(grid);
        DistMatrix<T,Star,MR> BL_Star_MR(grid), BR_Star_MR(grid);
        DistMatrix<T,MC,MR> CTL(grid), CTR(grid),
                            CBL(grid), CBR(grid);

        const unsigned half = C.Height() / 2;

        LockedPartitionRight( A_Star_MC, AL_Star_MC, AR_Star_MC, half );
        LockedPartitionRight( A_Star_MR, AL_Star_MR, AR_Star_MR, half );
        LockedPartitionRight( B_Star_MC, BL_Star_MC, BR_Star_MC, half );
        LockedPartitionRight( B_Star_MR, BL_Star_MR, BR_Star_MR, half );
        PartitionDownDiagonal( C, CTL, CTR,
                                  CBL, CBR, half );

        BLAS::Gemm
        ( ConjugateTranspose, Normal,
          alpha, AR_Star_MC.LockedLocalMatrix(),
                 BL_Star_MR.LockedLocalMatrix(),
          beta,  CBL.LocalMatrix()              );

        BLAS::Gemm
        ( ConjugateTranspose, Normal,
          alpha, BR_Star_MC.LockedLocalMatrix(),
                 AL_Star_MR.LockedLocalMatrix(),
          beta,  CBL.LocalMatrix()              );

        // Recurse
        BLAS::Internal::Her2kLCUpdate
        ( alpha, AL_Star_MC, AL_Star_MR, BL_Star_MC, BL_Star_MR, beta, CTL );

        BLAS::Internal::Her2kLCUpdate
        ( alpha, AR_Star_MC, AR_Star_MR, BR_Star_MC, BR_Star_MR, beta, CBR );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
Elemental::BLAS::Internal::Her2kLCUpdateKernel
( const T alpha, const DistMatrix<T,Star,MC>& A_Star_MC,
                 const DistMatrix<T,Star,MR>& A_Star_MR,
                 const DistMatrix<T,Star,MC>& B_Star_MC,
                 const DistMatrix<T,Star,MR>& B_Star_MR,
  const T beta,        DistMatrix<T,MC,  MR>& C         )
{
#ifndef RELEASE
    PushCallStack("BLAS::Internal::Her2kLCUpdateKernel");
    if( A_Star_MC.GetGrid() != A_Star_MR.GetGrid() || 
        A_Star_MR.GetGrid() != B_Star_MC.GetGrid() ||
        B_Star_MC.GetGrid() != B_Star_MR.GetGrid() ||
        B_Star_MR.GetGrid() != C.GetGrid()           )
    {
        throw "{A,B,C} must be distributed over the same grid.";
    }
    if( A_Star_MC.Width() != C.Height() ||
        A_Star_MR.Width() != C.Width()  ||
        B_Star_MC.Width() != C.Height() ||
        B_Star_MR.Width() != C.Width()  ||
        A_Star_MC.Height() != A_Star_MR.Height() ||
        A_Star_MC.Width()  != A_Star_MR.Width()  ||  
        B_Star_MC.Height() != B_Star_MR.Height() ||
        B_Star_MC.Width()  != B_Star_MR.Width()     )
    {
        ostringstream msg;
        msg << "Nonconformal Her2kLCUpdateKernel: " << endl
            << "  A[* ,MC] ~ " << A_Star_MC.Height() << " x "
                               << A_Star_MC.Width()  << endl
            << "  A[* ,MR] ~ " << A_Star_MR.Height() << " x "
                               << A_Star_MR.Width()  << endl
            << "  B[* ,MC] ~ " << B_Star_MC.Height() << " x "
                               << B_Star_MC.Width()  << endl
            << "  B[* ,MR] ~ " << B_Star_MR.Height() << " x "
                               << B_Star_MR.Width()  << endl
            << "  C[MC,MR] ~ " << C.Height() << " x " << C.Width() << endl;
        throw msg.str();
    }
    if( A_Star_MC.RowAlignment() != C.ColAlignment() ||
        A_Star_MR.RowAlignment() != C.RowAlignment() ||  
        B_Star_MC.RowAlignment() != C.ColAlignment() ||
        B_Star_MR.RowAlignment() != C.RowAlignment()    )
    {
        ostringstream msg;
        msg << "Misaligned Her2kLCUpdateKernel: " << endl
            << "  A[* ,MC] ~ " << A_Star_MC.RowAlignment() << endl
            << "  A[* ,MR] ~ " << A_Star_MR.RowAlignment() << endl
            << "  B[* ,MC] ~ " << B_Star_MC.RowAlignment() << endl
            << "  B[* ,MR] ~ " << B_Star_MR.RowAlignment() << endl
            << "  C[MC,MR] ~ " << C.ColAlignment() << " , " <<
                                  C.RowAlignment() << endl;
        throw msg.str();
    }
#endif
    const Grid& grid = A_Star_MC.GetGrid();

    DistMatrix<T,Star,MC> AL_Star_MC(grid), AR_Star_MC(grid);
    DistMatrix<T,Star,MR> AL_Star_MR(grid), AR_Star_MR(grid);
    DistMatrix<T,Star,MC> BL_Star_MC(grid), BR_Star_MC(grid);
    DistMatrix<T,Star,MR> BL_Star_MR(grid), BR_Star_MR(grid);
    DistMatrix<T,MC,MR>
        CTL(grid), CTR(grid),
        CBL(grid), CBR(grid);

    DistMatrix<T,MC,MR> DTL(grid), DBR(grid);

    const unsigned half = C.Height()/2;

    BLAS::Scal( beta, C );

    LockedPartitionRight( A_Star_MC, AL_Star_MC, AR_Star_MC, half );
    LockedPartitionRight( A_Star_MR, AL_Star_MR, AR_Star_MR, half );
    LockedPartitionRight( B_Star_MC, BL_Star_MC, BR_Star_MC, half );
    LockedPartitionRight( B_Star_MR, BL_Star_MR, BR_Star_MR, half );
    PartitionDownDiagonal( C, CTL, CTR,
                              CBL, CBR, half );

    DTL.AlignWith( CTL );
    DBR.AlignWith( CBR );
    DTL.ResizeTo( CTL.Height(), CTL.Width() );
    DBR.ResizeTo( CBR.Height(), CBR.Width() );
    //------------------------------------------------------------------------//
    BLAS::Gemm( ConjugateTranspose, Normal,
                alpha, AR_Star_MC.LockedLocalMatrix(),
                       BL_Star_MR.LockedLocalMatrix(),
                (T)1,  CBL.LocalMatrix()              );
    BLAS::Gemm( ConjugateTranspose, Normal,
                alpha, BR_Star_MC.LockedLocalMatrix(),
                       AL_Star_MR.LockedLocalMatrix(),
                (T)1,  CBL.LocalMatrix()              );

    BLAS::Gemm( ConjugateTranspose, Normal,
                alpha, AL_Star_MC.LockedLocalMatrix(),
                       BL_Star_MR.LockedLocalMatrix(),
                (T)0,  DTL.LocalMatrix()              );
    BLAS::Gemm( ConjugateTranspose, Normal,
                alpha, BL_Star_MC.LockedLocalMatrix(),
                       AL_Star_MR.LockedLocalMatrix(),
                (T)1,  DTL.LocalMatrix()              );
    DTL.MakeTrapezoidal( Left, Lower );
    BLAS::Axpy( (T)1, DTL, CTL );

    BLAS::Gemm( ConjugateTranspose, Normal,
                alpha, AR_Star_MC.LockedLocalMatrix(),
                       BR_Star_MR.LockedLocalMatrix(),
                (T)0,  DBR.LocalMatrix()              );
    BLAS::Gemm( ConjugateTranspose, Normal,
                alpha, BR_Star_MC.LockedLocalMatrix(),
                       AR_Star_MR.LockedLocalMatrix(),
                (T)1,  DBR.LocalMatrix()              );
    DBR.MakeTrapezoidal( Left, Lower );
    BLAS::Axpy( (T)1, DBR, CBR );
    //------------------------------------------------------------------------//

#ifndef RELEASE
    PopCallStack();
#endif
}

template void Elemental::BLAS::Internal::Her2kLC
( const float alpha, const DistMatrix<float,MC,MR>& A,
                     const DistMatrix<float,MC,MR>& B,
  const float beta,        DistMatrix<float,MC,MR>& C );

template void Elemental::BLAS::Internal::Her2kLC
( const double alpha, const DistMatrix<double,MC,MR>& A,
                      const DistMatrix<double,MC,MR>& B,
  const double beta,        DistMatrix<double,MC,MR>& C );

#ifndef WITHOUT_COMPLEX
template void Elemental::BLAS::Internal::Her2kLC
( const scomplex alpha, const DistMatrix<scomplex,MC,MR>& A,
                        const DistMatrix<scomplex,MC,MR>& B,
  const scomplex beta,        DistMatrix<scomplex,MC,MR>& C );

template void Elemental::BLAS::Internal::Her2kLC
( const dcomplex alpha, const DistMatrix<dcomplex,MC,MR>& A,
                        const DistMatrix<dcomplex,MC,MR>& B,
  const dcomplex beta,        DistMatrix<dcomplex,MC,MR>& C );
#endif

