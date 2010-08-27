#include "elemental/blas_internal.hpp"
#include "elemental/lapack_internal.hpp"
using namespace elemental;
using namespace std;

#include "./UTUtil.hpp"

// This routine reverses the accumulation of Householder transforms stored 
// in the portion of H below the diagonal marked by 'offset'. It is assumed 
// that the Householder transforms were accumulated left-to-right.

template<typename R>
void
elemental::lapack::internal::UTLLC
( int offset, 
  const DistMatrix<R,MC,MR>& H,
        DistMatrix<R,MC,MR>& A )
{
#ifndef RELEASE
    PushCallStack("lapack::internal::UTLLC");
    if( H.GetGrid() != A.GetGrid() )
        throw logic_error( "H and A must be distributed over the same grid." );
    if( offset > 0 )
        throw logic_error( "Transforms cannot extend above matrix." );
    if( offset < -H.Height() )
        throw logic_error( "Transforms cannot extend below matrix." );
    if( H.Height() != A.Height() )
        throw logic_error
              ( "Height of transforms must equal height of target matrix." );
#endif
    const Grid& g = H.GetGrid();

    // Matrix views    
    DistMatrix<R,MC,MR>
        HTL(g), HTR(g),  H00(g), H01(g), H02(g),  HPan(g), HPanCopy(g),
        HBL(g), HBR(g),  H10(g), H11(g), H12(g),
                         H20(g), H21(g), H22(g);
    DistMatrix<R,MC,MR>
        ATL(g), ATR(g),  A00(g), A01(g), A02(g),  ABottom(g),
        ABL(g), ABR(g),  A10(g), A11(g), A12(g),
                         A20(g), A21(g), A22(g);

    DistMatrix<R,VC,  Star> HPan_VC_Star(g);
    DistMatrix<R,MC,  Star> HPan_MC_Star(g);
    DistMatrix<R,Star,Star> SInv_Star_Star(g);
    DistMatrix<R,Star,MR  > Z_Star_MR(g);
    DistMatrix<R,Star,VR  > Z_Star_VR(g);

    LockedPartitionUpDiagonal
    ( H, HTL, HTR,
         HBL, HBR, 0 );
    PartitionUpDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    while( HBR.Height() < H.Height() && HBR.Width() < H.Width() )
    {
        LockedRepartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, H01, /**/ H02,
               /**/       H10, H11, /**/ H12,
         /*************/ /******************/
          HBL, /**/ HBR,  H20, H21, /**/ H22 );

        RepartitionUpDiagonal
        ( ATL, /**/ ATR,  A00, A01, /**/ A02,
               /**/       A10, A11, /**/ A12,
         /*************/ /******************/
          ABL, /**/ ABR,  A20, A21, /**/ A22 );

        int HPanHeight = H11.Height() + H21.Height();
        int HPanWidth = min( H11.Width(), max(HPanHeight+offset,0) );
        HPan.LockedView( H, H00.Height(), H00.Width(), HPanHeight, HPanWidth );

        ABottom.View( A, A00.Height(), 0, A.Height()-A00.Height(), A.Width() );

        HPan_MC_Star.AlignWith( ABottom );
        Z_Star_MR.AlignWith( ABottom );
        Z_Star_VR.AlignWith( ABottom );
        Z_Star_MR.ResizeTo( HPan.Width(), ABottom.Width() );
        SInv_Star_Star.ResizeTo( HPan.Width(), HPan.Width() );
        //--------------------------------------------------------------------//
        HPanCopy = HPan;
        HPanCopy.MakeTrapezoidal( Left, Lower, offset );
        SetDiagonalToOne( Left, offset, HPanCopy );

        HPan_VC_Star = HPanCopy;
        blas::Syrk
        ( Upper, Transpose, 
          (R)1, HPan_VC_Star.LockedLocalMatrix(),
          (R)0, SInv_Star_Star.LocalMatrix() );     
        SInv_Star_Star.AllSum();
        HalveMainDiagonal( SInv_Star_Star );

        HPan_MC_Star = HPanCopy;
        blas::internal::LocalGemm
        ( Transpose, Normal, 
          (R)1, HPan_MC_Star, ABottom, (R)0, Z_Star_MR );
        Z_Star_VR.SumScatterFrom( Z_Star_MR );
 
        blas::internal::LocalTrsm
        ( Left, Upper, Normal, NonUnit, 
          (R)1, SInv_Star_Star, Z_Star_VR );

        Z_Star_MR = Z_Star_VR;
        blas::internal::LocalGemm
        ( Normal, Normal, (R)-1, HPan_MC_Star, Z_Star_MR, (R)1, ABottom );
        //--------------------------------------------------------------------//
        HPan_MC_Star.FreeAlignments();
        Z_Star_MR.FreeAlignments();
        Z_Star_VR.FreeAlignments();

        SlideLockedPartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, /**/ H01, H02,
         /*************/ /******************/
               /**/       H10, /**/ H11, H12,
          HBL, /**/ HBR,  H20, /**/ H21, H22 );

        SlidePartitionUpDiagonal
        ( ATL, /**/ ATR,  A00, /**/ A01, A02,
         /*************/ /******************/
               /**/       A10, /**/ A11, A12,
          ABL, /**/ ABR,  A20, /**/ A21, A22 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

#ifndef WITHOUT_COMPLEX
template<typename R>
void
elemental::lapack::internal::UTLLC
( int offset, 
  const DistMatrix<complex<R>,MC,MR  >& H,
  const DistMatrix<complex<R>,MD,Star>& t,
        DistMatrix<complex<R>,MC,MR  >& A )
{
#ifndef RELEASE
    PushCallStack("lapack::internal::UTLLC");
    if( H.GetGrid() != t.GetGrid() || t.GetGrid() != A.GetGrid() )
        throw logic_error
              ( "H, t, and A must be distributed over the same grid." );
    if( offset > 0 )
        throw logic_error( "Transforms cannot extend above matrix." );
    if( offset < -H.Height() )
        throw logic_error( "Transforms cannot extend below matrix." );
    if( H.Height() != A.Height() )
        throw logic_error
              ( "Height of transforms must equal height of target matrix." );
    if( t.Height() != H.DiagonalLength( offset ) )
        throw logic_error( "t must be the same length as H's 'offset' diag." );
    if( !t.AlignedWithDiag( H, offset ) )
        throw logic_error( "t must be aligned with H's 'offset' diagonal." );
#endif
    typedef complex<R> C;
    const Grid& g = H.GetGrid();

    // Matrix views    
    DistMatrix<C,MC,MR>
        HTL(g), HTR(g),  H00(g), H01(g), H02(g),  HPan(g), HPanCopy(g),
        HBL(g), HBR(g),  H10(g), H11(g), H12(g),
                         H20(g), H21(g), H22(g);
    DistMatrix<C,MC,MR>
        ATL(g), ATR(g),  A00(g), A01(g), A02(g),  ABottom(g),
        ABL(g), ABR(g),  A10(g), A11(g), A12(g),
                         A20(g), A21(g), A22(g);
    DistMatrix<C,MD,Star>
        tT(g),  t0(g),
        tB(g),  t1(g),
                t2(g);

    DistMatrix<C,VC,  Star> HPan_VC_Star(g);
    DistMatrix<C,MC,  Star> HPan_MC_Star(g);
    DistMatrix<C,Star,Star> t1_Star_Star(g);
    DistMatrix<C,Star,Star> SInv_Star_Star(g);
    DistMatrix<C,Star,MR  > Z_Star_MR(g);
    DistMatrix<C,Star,VR  > Z_Star_VR(g);

    LockedPartitionUpDiagonal
    ( H, HTL, HTR,
         HBL, HBR, 0 );
    LockedPartitionUp
    ( t, tT,
         tB, 0 );
    PartitionUpDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    while( HBR.Height() < H.Height() && HBR.Width() < H.Width() )
    {
        LockedRepartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, H01, /**/ H02,
               /**/       H10, H11, /**/ H12,
         /*************/ /******************/
          HBL, /**/ HBR,  H20, H21, /**/ H22 );

        int HPanHeight = H11.Height() + H21.Height();
        int HPanWidth = min( H11.Width(), max(HPanHeight+offset,0) );

        LockedRepartitionUp
        ( tT,  t0,
               t1,
         /**/ /**/
          tB,  t2, HPanWidth );

        RepartitionUpDiagonal
        ( ATL, /**/ ATR,  A00, A01, /**/ A02,
               /**/       A10, A11, /**/ A12,
         /*************/ /******************/
          ABL, /**/ ABR,  A20, A21, /**/ A22 );

        HPan.LockedView( H, H00.Height(), H00.Width(), HPanHeight, HPanWidth );

        ABottom.View( A, A00.Height(), 0, A.Height()-A00.Height(), A.Width() );

        HPan_MC_Star.AlignWith( ABottom );
        Z_Star_MR.AlignWith( ABottom );
        Z_Star_VR.AlignWith( ABottom );
        Z_Star_MR.ResizeTo( HPan.Width(), ABottom.Width() );
        SInv_Star_Star.ResizeTo( HPan.Width(), HPan.Width() );
        //--------------------------------------------------------------------//
        HPanCopy = HPan;
        HPanCopy.MakeTrapezoidal( Left, Lower, offset );
        SetDiagonalToOne( Left, offset, HPanCopy );

        HPan_VC_Star = HPanCopy;
        blas::Herk
        ( Upper, ConjugateTranspose, 
          (C)1, HPan_VC_Star.LockedLocalMatrix(),
          (C)0, SInv_Star_Star.LocalMatrix() );     
        SInv_Star_Star.AllSum();
        t1_Star_Star = t1;
        FixDiagonal( t1_Star_Star, SInv_Star_Star );

        HPan_MC_Star = HPanCopy;
        blas::internal::LocalGemm
        ( ConjugateTranspose, Normal, 
          (C)1, HPan_MC_Star, ABottom, (C)0, Z_Star_MR );
        Z_Star_VR.SumScatterFrom( Z_Star_MR );
 
        blas::internal::LocalTrsm
        ( Left, Upper, Normal, NonUnit, 
          (C)1, SInv_Star_Star, Z_Star_VR );

        Z_Star_MR = Z_Star_VR;
        blas::internal::LocalGemm
        ( Normal, Normal, (C)-1, HPan_MC_Star, Z_Star_MR, (C)1, ABottom );
        //--------------------------------------------------------------------//
        HPan_MC_Star.FreeAlignments();
        Z_Star_MR.FreeAlignments();
        Z_Star_VR.FreeAlignments();

        SlideLockedPartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, /**/ H01, H02,
         /*************/ /******************/
               /**/       H10, /**/ H11, H12,
          HBL, /**/ HBR,  H20, /**/ H21, H22 );

        SlideLockedPartitionUp
        ( tT,  t0,
         /**/ /**/
               t1,
          tB,  t2 );

        SlidePartitionUpDiagonal
        ( ATL, /**/ ATR,  A00, /**/ A01, A02,
         /*************/ /******************/
               /**/       A10, /**/ A11, A12,
          ABL, /**/ ABR,  A20, /**/ A21, A22 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}
#endif

template void elemental::lapack::internal::UTLLC
( int offset,
  const DistMatrix<float,MC,MR>& H,
        DistMatrix<float,MC,MR>& A );

template void elemental::lapack::internal::UTLLC
( int offset,
  const DistMatrix<double,MC,MR>& H,
        DistMatrix<double,MC,MR>& A );

#ifndef WITHOUT_COMPLEX
template void elemental::lapack::internal::UTLLC
( int offset,
  const DistMatrix<scomplex,MC,MR  >& H,
  const DistMatrix<scomplex,MD,Star>& t,
        DistMatrix<scomplex,MC,MR  >& A );

template void elemental::lapack::internal::UTLLC
( int offset,
  const DistMatrix<dcomplex,MC,MR  >& H,
  const DistMatrix<dcomplex,MD,Star>& t,
        DistMatrix<dcomplex,MC,MR  >& A );
#endif
