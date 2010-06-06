/*
   This file is part of elemental, a library for distributed-memory dense 
   linear algebra.

   Copyright (C) 2009-2010 Jack Poulson <jack.poulson@gmail.com>

   This program is released under the terms of the license contained in the 
   file LICENSE.
*/
#include "elemental/blas_internal.hpp"
#include "elemental/lapack_internal.hpp"
using namespace std;
using namespace elemental;

template<typename R>
void
elemental::lapack::internal::TridiagL
( DistMatrix<R,MC,MR  >& A,
  DistMatrix<R,MD,Star>& d,
  DistMatrix<R,MD,Star>& e,
  DistMatrix<R,MD,Star>& t )
{
#ifndef RELEASE
    PushCallStack("lapack::internal::TridiagL");
#endif
    const Grid& grid = A.GetGrid();
#ifndef RELEASE
    if( A.GetGrid() != d.GetGrid() ||
        d.GetGrid() != e.GetGrid() ||
        e.GetGrid() != t.GetGrid() )
        throw logic_error
        ( "A, d, e, and t must be distributed over the same grid." );
    if( A.Height() != A.Width() )
        throw logic_error( "A must be square." );
    if( d.Height() != A.Height() || d.Width() != 1 )
        throw logic_error
        ( "d must be a column vector of the same length as A's width." );
    if( e.Height() != A.Height()-1 || e.Width() != 1 )
        throw logic_error
        ( "e must be a column vector of length one less than the width of A." );
    if( t.Height() != A.Height()-1 || t.Width() != 1 )
        throw logic_error
        ( "t must be a column vector of length one less than the width of A." );
    if( d.ColAlignment() != A.ColAlignment() + 
                            A.RowAlignment() * grid.Height() )
        throw logic_error( "d is not aligned with A." );
    if( e.ColAlignment() != ((A.ColAlignment()+1) % grid.Height())
                            + A.RowAlignment() * grid.Height()    )
        throw logic_error( "e ist not aligned with A." );
    if( t.ColAlignment() != A.ColAlignment() + A.RowAlignment()*grid.Height() )
        throw logic_error( "t ist not aligned with A." );
#endif

    // Matrix views 
    DistMatrix<R,MC,MR> 
        ATL(grid), ATR(grid),  A00(grid), A01(grid), A02(grid), 
        ABL(grid), ABR(grid),  A10(grid), A11(grid), A12(grid),
                               A20(grid), A21(grid), A22(grid),
        A11Expanded(grid);
    DistMatrix<R,MD,Star> dT(grid),  d0(grid), 
                          dB(grid),  d1(grid),
                                     d2(grid);
    DistMatrix<R,MD,Star> eT(grid),  e0(grid), 
                          eB(grid),  e1(grid), 
                                     e2(grid);
    DistMatrix<R,MD,Star> tT(grid),  t0(grid),
                          tB(grid),  t1(grid),
                                     t2(grid);

    // Temporary distributions
    DistMatrix<R,Star,Star> A11_Star_Star(grid);
    DistMatrix<R,Star,Star> d1_Star_Star(grid);
    DistMatrix<R,Star,Star> e1_Star_Star(grid);
    DistMatrix<R,Star,Star> t1_Star_Star(grid);
    DistMatrix<R,MC,  MR  > W11(grid),  WPan(grid),
                            W21(grid);

    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    PartitionDown
    ( d, dT,
         dB, 0 );
    PartitionDown
    ( e, eT,
         eB, 0 );
    PartitionDown
    ( t, tT,
         tB, 0 );
    while( ATL.Height() < A.Height() )
    {
        RepartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, /**/ A01, A02,
         /*************/ /******************/
               /**/       A10, /**/ A11, A12,
          ABL, /**/ ABR,  A20, /**/ A21, A22 );

        RepartitionDown
        ( dT,  d0,
         /**/ /**/
               d1,
          dB,  d2 );

        RepartitionDown
        ( eT,  e0,
         /**/ /**/
               e1,
          eB,  e2 );

        RepartitionDown
        ( tT,  t0,
         /**/ /**/
               t1,
          tB,  t2 );

        if( A22.Height() > 0 )
        {
            A11Expanded.View( ABR, 0, 0, A11.Height()+1, A11.Width()+1 );
            WPan.AlignWith( A11 );
            WPan.ResizeTo( ABR.Height(), A11.Width() );
            PartitionDown
            ( WPan, W11,
                    W21, A11.Height() );
            //----------------------------------------------------------------//
            lapack::internal::PanelTridiagL( ABR, WPan, e1, t1 );
            blas::Syr2k( Lower, Normal, (R)-1, A21, W21, (R)1, A22 );
            A11Expanded.SetDiagonal( e1, -1 );
            A11.GetDiagonal( d1 );
            //----------------------------------------------------------------//
            WPan.FreeAlignments();
        }
        else
        {
            A11_Star_Star = A11;
            d1_Star_Star = d1;
            e1_Star_Star = e1;
            t1_Star_Star = t1;

            lapack::Tridiag
            ( Lower, 
              A11_Star_Star.LocalMatrix(),
              d1_Star_Star.LocalMatrix(), 
              e1_Star_Star.LocalMatrix(),
              t1_Star_Star.LocalMatrix() );

            A11 = A11_Star_Star;
            d1 = d1_Star_Star;
            e1 = e1_Star_Star;
            t1 = t1_Star_Star;
        }

        SlidePartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, A01, /**/ A02,
               /**/       A10, A11, /**/ A12,
         /*************/ /******************/
          ABL, /**/ ABR,  A20, A21, /**/ A22 );

        SlidePartitionDown
        ( dT,  d0,
               d1,
         /**/ /**/
          dB,  d2 );

        SlidePartitionDown
        ( eT,  e0,
               e1,
         /**/ /**/
          eB,  e2 );

        SlidePartitionDown
        ( tT,  t0,
               t1,
         /**/ /**/
          tB,  t2 );
    }
        
#ifndef RELEASE
    PopCallStack();
#endif
}

#ifndef WITHOUT_COMPLEX
template<typename R>
void
elemental::lapack::internal::TridiagL
( DistMatrix<complex<R>,MC,MR  >& A,
  DistMatrix<R,MD,Star>& d,
  DistMatrix<R,MD,Star>& e,
  DistMatrix<complex<R>,MD,Star>& t )
{
#ifndef RELEASE
    PushCallStack("lapack::internal::TridiagL");
#endif
    const Grid& grid = A.GetGrid();
#ifndef RELEASE
    if( A.GetGrid() != d.GetGrid() ||
        d.GetGrid() != e.GetGrid() ||
        e.GetGrid() != t.GetGrid() )
        throw logic_error
        ( "A, d, e, and t must be distributed over the same grid." );
    if( A.Height() != A.Width() )
        throw logic_error( "A must be square." );
    if( d.Height() != A.Height() || d.Width() != 1 )
        throw logic_error
        ( "d must be a column vector of the same length as A's width." );
    if( e.Height() != A.Height()-1 || e.Width() != 1 )
        throw logic_error
        ( "e must be a column vector of length one less than the width of A." );
    if( t.Height() != A.Height()-1 || t.Width() != 1 )
        throw logic_error
        ( "t must be a column vector of length one less than the width of A." );
    if( d.ColAlignment() != A.ColAlignment() + 
                            A.RowAlignment() * grid.Height() )
        throw logic_error( "d is not aligned with A." );
    if( e.ColAlignment() != ((A.ColAlignment()+1) % grid.Height())
                            + A.RowAlignment() * grid.Height()    )
        throw logic_error( "e ist not aligned with A." );
    if( t.ColAlignment() != A.ColAlignment() + A.RowAlignment()*grid.Height() )
        throw logic_error( "t ist not aligned with A." );
#endif
    typedef complex<R> C;

    // Matrix views 
    DistMatrix<C,MC,MR> 
        ATL(grid), ATR(grid),  A00(grid), A01(grid), A02(grid), 
        ABL(grid), ABR(grid),  A10(grid), A11(grid), A12(grid),
                               A20(grid), A21(grid), A22(grid),
        A11Expanded(grid);
    DistMatrix<R,MD,Star> dT(grid),  d0(grid), 
                          dB(grid),  d1(grid),
                                     d2(grid);
    DistMatrix<R,MD,Star> eT(grid),  e0(grid), 
                          eB(grid),  e1(grid), 
                                     e2(grid);
    DistMatrix<C,MD,Star> tT(grid),  t0(grid),
                          tB(grid),  t1(grid),
                                     t2(grid);

    // Temporary distributions
    DistMatrix<C,Star,Star> A11_Star_Star(grid);
    DistMatrix<R,Star,Star> d1_Star_Star(grid);
    DistMatrix<R,Star,Star> e1_Star_Star(grid);
    DistMatrix<C,Star,Star> t1_Star_Star(grid);
    DistMatrix<C,MC,  MR  > W11(grid),  WPan(grid),
                            W21(grid);

    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    PartitionDown
    ( d, dT,
         dB, 0 );
    PartitionDown
    ( e, eT,
         eB, 0 );
    PartitionDown
    ( t, tT,
         tB, 0 );
    while( ATL.Height() < A.Height() )
    {
        RepartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, /**/ A01, A02,
         /*************/ /******************/
               /**/       A10, /**/ A11, A12,
          ABL, /**/ ABR,  A20, /**/ A21, A22 );

        RepartitionDown
        ( dT,  d0,
         /**/ /**/
               d1,
          dB,  d2 );

        RepartitionDown
        ( eT,  e0,
         /**/ /**/
               e1,
          eB,  e2 );

        RepartitionDown
        ( tT,  t0,
         /**/ /**/
               t1,
          tB,  t2 );

        if( A22.Height() > 0 )
        {
            A11Expanded.View( ABR, 0, 0, A11.Height()+1, A11.Width()+1 );
            WPan.AlignWith( A11 );
            WPan.ResizeTo( ABR.Height(), A11.Width() );
            PartitionDown
            ( WPan, W11,
                    W21, A11.Height() );
            //----------------------------------------------------------------//
            lapack::internal::PanelTridiagL( ABR, WPan, e1, t1 );
            blas::Her2k( Lower, Normal, (C)-1, A21, W21, (C)1, A22 );
            A11Expanded.SetDiagonal( e1, -1 );
            A11.GetRealDiagonal( d1 );
            //----------------------------------------------------------------//
            WPan.FreeAlignments();
        }
        else
        {
            A11_Star_Star = A11;
            d1_Star_Star = d1;
            e1_Star_Star = e1;
            t1_Star_Star = t1;

            lapack::Tridiag
            ( Lower, 
              A11_Star_Star.LocalMatrix(),
              d1_Star_Star.LocalMatrix(), 
              e1_Star_Star.LocalMatrix(),
              t1_Star_Star.LocalMatrix() );

            A11 = A11_Star_Star;
            d1 = d1_Star_Star;
            e1 = e1_Star_Star;
            t1 = t1_Star_Star;
        }

        SlidePartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, A01, /**/ A02,
               /**/       A10, A11, /**/ A12,
         /*************/ /******************/
          ABL, /**/ ABR,  A20, A21, /**/ A22 );

        SlidePartitionDown
        ( dT,  d0,
               d1,
         /**/ /**/
          dB,  d2 );

        SlidePartitionDown
        ( eT,  e0,
               e1,
         /**/ /**/
          eB,  e2 );

        SlidePartitionDown
        ( tT,  t0,
               t1,
         /**/ /**/
          tB,  t2 );
    }
        
#ifndef RELEASE
    PopCallStack();
#endif
}
#endif // WITHOUT_COMPLEX

template void elemental::lapack::internal::TridiagL
( DistMatrix<float,MC,MR  >& A,
  DistMatrix<float,MD,Star>& d,
  DistMatrix<float,MD,Star>& e,
  DistMatrix<float,MD,Star>& t );

template void elemental::lapack::internal::TridiagL
( DistMatrix<double,MC,MR  >& A,
  DistMatrix<double,MD,Star>& d,
  DistMatrix<double,MD,Star>& e,
  DistMatrix<double,MD,Star>& t );

#ifndef WITHOUT_COMPLEX
template void elemental::lapack::internal::TridiagL
( DistMatrix<scomplex,MC,MR  >& A,
  DistMatrix<float,   MD,Star>& d,
  DistMatrix<float,   MD,Star>& e,
  DistMatrix<scomplex,MD,Star>& t );

template void elemental::lapack::internal::TridiagL
( DistMatrix<dcomplex,MC,MR  >& A,
  DistMatrix<double,  MD,Star>& d,
  DistMatrix<double,  MD,Star>& e,
  DistMatrix<dcomplex,MD,Star>& t );
#endif
