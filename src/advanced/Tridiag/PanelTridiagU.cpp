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
#include "elemental/basic_internal.hpp"
#include "elemental/advanced_internal.hpp"
using namespace std;
using namespace elemental;
using namespace elemental::imports;
using namespace elemental::utilities;

template<typename R> // representation of a real number
void
elemental::advanced::internal::PanelTridiagU
( DistMatrix<R,MC,MR  >& A,
  DistMatrix<R,MC,MR  >& W,
  DistMatrix<R,MC,Star>& APan_MC_Star, 
  DistMatrix<R,MR,Star>& APan_MR_Star,
  DistMatrix<R,MC,Star>& W_MC_Star,
  DistMatrix<R,MR,Star>& W_MR_Star )
{
    const int panelSize = W.Width();
    const int topSize = W.Height()-panelSize;
#ifndef RELEASE
    PushCallStack("advanced::internal::PanelTridiagU");
    if( A.Grid() != W.Grid() )
        throw logic_error
        ( "A and W must be distributed over the same grid." );
    if( A.Height() != A.Width() )
        throw logic_error( "A must be square." );
    if( A.Height() != W.Height() )
        throw logic_error( "A and W must be the same height." );
    if( W.Height() < panelSize )
        throw logic_error( "W must be a column panel." );
#endif
    const Grid& g = A.Grid();
    const int r = g.Height();
    const int c = g.Width();
    const int p = g.Size();

    // Create a distributed matrix for storing the superdiagonal
    DistMatrix<R,MD,Star> e(g);
    DistMatrix<R,MC,MR> expandedABR(g);
    expandedABR.View( A, topSize-1, topSize-1, panelSize+1, panelSize+1 );
    e.AlignWithDiag( expandedABR, 1 );
    e.ResizeTo( panelSize, 1 );

    // Matrix views 
    DistMatrix<R,MC,MR> 
        ATL(g), ATR(g),  A00(g), a01(g),     A02(g),  ACol(g), a01Top(g),
        ABL(g), ABR(g),  a10(g), alpha11(g), a12(g),           alpha01Bottom(g),
                         A20(g), a21(g),     A22(g),  A02T(g), A00Pan(g);
    DistMatrix<R,MC,MR> 
        WTL(g), WTR(g),  W00(g), w01(g),     W02(g),  WCol(g),
        WBL(g), WBR(g),  w10(g), omega11(g), w12(g),
                         W20(g), w21(g),     W22(g),  W02T(g), w01Last(g);
    DistMatrix<R,MD,Star> eT(g),  e0(g),
                          eB(g),  epsilon1(g),
                                  e2(g);

    // Temporary distributions
    vector<R> w01LastLocalBuffer(A.Height()/r+1);
    DistMatrix<R,MC,Star> a01_MC_Star(g);
    DistMatrix<R,MC,Star> a01T_MC_Star(g);
    DistMatrix<R,MR,Star> a01_MR_Star(g);
    DistMatrix<R,MC,Star> p01_MC_Star(g);
    DistMatrix<R,MC,Star> p01T_MC_Star(g);
    DistMatrix<R,MR,Star> p01_MR_Star(g);
    DistMatrix<R,MC,Star> q01_MC_Star(g);
    DistMatrix<R,MR,Star> q01_MR_Star(g);
    DistMatrix<R,MR,Star> x21_MR_Star(g);
    DistMatrix<R,MR,Star> y21_MR_Star(g);
    DistMatrix<R,MC,Star> a01Last_MC_Star(g);
    DistMatrix<R,MR,Star> a01Last_MR_Star(g);
    DistMatrix<R,MC,Star> w01Last_MC_Star(g);
    DistMatrix<R,MR,Star> w01Last_MR_Star(g);

    // Push to the blocksize of 1, then pop at the end of the routine
    PushBlocksizeStack( 1 );

    PartitionUpRightDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    PartitionUpRightDiagonal
    ( W, WTL, WTR,
         WBL, WBR, 0 );
    PartitionUp
    ( e, eT,
         eB, 0 );
    bool firstIteration = true;
    R tau = 0;
    R w01LastBottomEntry = 0;
    while( WBR.Width() < panelSize )
    {
        RepartitionUpDiagonal
        ( ATL, /**/ ATR,  A00, a01,     /**/ A02,
               /**/       a10, alpha11, /**/ a12, 
         /*************/ /**********************/
          ABL, /**/ ABR,  A20, a21,     /**/ A22 );
        
        RepartitionUpDiagonal
        ( WTL, /**/ WTR,  W00, w01,     /**/ W02,
               /**/       w10, omega11, /**/ w12,
         /*************/ /**********************/
          WBL, /**/ WBR,  W20, w21,     /**/ W22 );

        RepartitionUp
        ( eT,  e0,
               epsilon1,
         /**/ /********/
          eB,  e2 );

        ACol.View2x1
        ( a01,
          alpha11 );

        WCol.View2x1
        ( w01,
          omega11 );

        // View the portions of A02 and W02 outside of this panel's square
        A02T.View( A02, 0, 0, topSize, A02.Width() );
        W02T.View( W02, 0, 0, topSize, W02.Width() );

        // View the portion of A00 that is inside this panel
        A00Pan.View( A00, 0, topSize, A00.Height(), A00.Width()-topSize );

        if( !firstIteration )
        {
            a01Last_MC_Star.View
            ( APan_MC_Star, 0, WTL.Width(), ACol.Height(), 1 );
            a01Last_MR_Star.View
            ( APan_MR_Star, 0, WTL.Width(), ACol.Height(), 1 );
            w01Last.View
            ( W, 0, WTL.Width(), ACol.Height(), 1 );
        }
            
        PartitionUp
        ( a01, a01Top,
               alpha01Bottom, 1 );

        a01_MC_Star.AlignWith( A00 );
        a01_MR_Star.AlignWith( A00 );
        p01_MC_Star.AlignWith( A00 );
        p01_MR_Star.AlignWith( A00 );
        q01_MC_Star.AlignWith( A00 );
        q01_MR_Star.AlignWith( A00 );
        a01_MC_Star.ResizeTo( a01.Height(), 1 );
        a01_MR_Star.ResizeTo( a01.Height(), 1 );
        p01_MC_Star.ResizeTo( a01.Height(), 1 );
        p01_MR_Star.ResizeTo( a01.Height(), 1 );
        q01_MC_Star.ResizeTo( a01.Height(), 1 );
        q01_MR_Star.ResizeTo( a01.Height(), 1 );
        x21_MR_Star.AlignWith( A02T );
        y21_MR_Star.AlignWith( A02T );
        x21_MR_Star.ResizeTo( A02.Width(), 1 );
        y21_MR_Star.ResizeTo( A02.Width(), 1 );

        // View the portions of a01[MC,* ] and p01[MC,* ] above the current
        // panel's square
        a01T_MC_Star.View( a01_MC_Star, 0, 0, topSize, 1 );
        p01T_MC_Star.View( p01_MC_Star, 0, 0, topSize, 1 );
        //--------------------------------------------------------------------//
        const bool thisIsMyCol = ( g.MRRank() == alpha11.RowAlignment() );
        if( thisIsMyCol )
        {
            if( !firstIteration )
            {
                // Finish updating the current column with two axpy's
                int AColLocalHeight = ACol.LocalHeight();
                R* AColLocalBuffer = ACol.LocalBuffer();
                const R* a01Last_MC_Star_LocalBuffer = 
                    a01Last_MC_Star.LocalBuffer();
                for( int i=0; i<AColLocalHeight; ++i )
                    AColLocalBuffer[i] -=
                        w01LastLocalBuffer[i] + 
                        a01Last_MC_Star_LocalBuffer[i]*w01LastBottomEntry;
            }
        }
        if( thisIsMyCol )
        {
            // Compute the Householder reflector
            tau = advanced::internal::ColReflector( alpha01Bottom, a01Top );
        }
        // Store the subdiagonal value and turn a01 into a proper scaled 
        // reflector by explicitly placing the implicit one in its bottom entry
        alpha01Bottom.GetDiagonal( epsilon1 );
        alpha01Bottom.Set( 0, 0, (R)1 );

        // If this is the first iteration, have each member of the owning 
        // process column broadcast tau and a01 within its process row. 
        // Otherwise, also add w01 into the broadcast.
        if( firstIteration )
        {
            const int a01LocalHeight = a01.LocalHeight();
            vector<R> rowBroadcastBuffer(a01LocalHeight+1);
            if( thisIsMyCol )
            {
                // Pack the broadcast buffer with a01 and tau
                memcpy
                ( &rowBroadcastBuffer[0], 
                  a01.LocalBuffer(), 
                  a01LocalHeight*sizeof(R) );
                rowBroadcastBuffer[a01LocalHeight] = tau;
            }
            // Broadcast a01 and tau across the process row
            mpi::Broadcast
            ( &rowBroadcastBuffer[0], 
              a01LocalHeight+1, a01.RowAlignment(), g.MRComm() );
            // Store a01[MC,* ] into its DistMatrix class and also store a copy
            // for the next iteration
            memcpy
            ( a01_MC_Star.LocalBuffer(), 
              &rowBroadcastBuffer[0],
              a01LocalHeight*sizeof(R) );
            // Store a01[MC,* ] into APan[MC,* ]
            memcpy
            ( APan_MC_Star.LocalBuffer(0,W00.Width()), 
              &rowBroadcastBuffer[0],
              a01LocalHeight*sizeof(R) );
            // Store tau
            tau = rowBroadcastBuffer[a01LocalHeight];
            
            a01_MR_Star = a01_MC_Star;
            // Store a01[MR,* ]
            memcpy
            ( APan_MR_Star.LocalBuffer(0,W00.Width()),
              a01_MR_Star.LocalBuffer(),
              a01_MR_Star.LocalHeight()*sizeof(R) );
        }
        else
        {
            const int a01LocalHeight = a01.LocalHeight();
            const int w01LastLocalHeight = ACol.LocalHeight();
            vector<R> rowBroadcastBuffer(a01LocalHeight+w01LastLocalHeight+1);
            if( thisIsMyCol ) 
            {
                // Pack the broadcast buffer with a01, w01Last, and tau
                memcpy
                ( &rowBroadcastBuffer[0], 
                  a01.LocalBuffer(),
                  a01LocalHeight*sizeof(R) );
                memcpy
                ( &rowBroadcastBuffer[a01LocalHeight], 
                  &w01LastLocalBuffer[0],
                  w01LastLocalHeight*sizeof(R) );
                rowBroadcastBuffer[a01LocalHeight+w01LastLocalHeight] = tau;
            }
            // Broadcast a01, w01Last, and tau across the process row
            mpi::Broadcast
            ( &rowBroadcastBuffer[0], 
              a01LocalHeight+w01LastLocalHeight+1, 
              a01.RowAlignment(), g.MRComm() );
            // Store a01[MC,* ] into its DistMatrix class 
            memcpy
            ( a01_MC_Star.LocalBuffer(), 
              &rowBroadcastBuffer[0],
              a01LocalHeight*sizeof(R) );
            // Store a01[MC,* ] into APan[MC,* ]
            memcpy
            ( APan_MC_Star.LocalBuffer(0,W00.Width()),
              &rowBroadcastBuffer[0],
              a01LocalHeight*sizeof(R) );
            // Store w01Last[MC,* ] into its DistMatrix class
            w01Last_MC_Star.AlignWith( A00 );
            w01Last_MC_Star.ResizeTo( a01.Height()+1, 1 );
            memcpy
            ( w01Last_MC_Star.LocalBuffer(), 
              &rowBroadcastBuffer[a01LocalHeight], 
              w01LastLocalHeight*sizeof(R) );
            // Store the bottom part of w01Last[MC,* ] into WB[MC,* ] and, 
            // if necessary, w01.
            memcpy
            ( W_MC_Star.LocalBuffer(0,W00.Width()+1),
              &rowBroadcastBuffer[a01LocalHeight],
              w01LastLocalHeight*sizeof(R) );
            if( g.MRRank() == w01Last.RowAlignment() )
            {
                memcpy
                ( w01Last.LocalBuffer(),
                  &rowBroadcastBuffer[a01LocalHeight],
                  w01LastLocalHeight*sizeof(R) );
            }
            // Store tau
            tau = rowBroadcastBuffer[a01LocalHeight+w01LastLocalHeight];

            // Form a01[MR,* ] and w01Last[MR,* ] by combining the 
            // communications needed for taking a vector from 
            // [MC,* ] -> [MR,* ]: 
            //   local copy to [VC,* ], 
            //   Send/Recv to [VR,* ], 
            //   AllGather to [MR,* ]
            // We can combine the two by treating a01 as [a01; 0].

            int colAlignSource = A00.ColAlignment();
            int colAlignDest = A00.RowAlignment();
            int colShiftSource = A00.ColShift();
            int colShiftDest = A00.RowShift();

            int height = a01.Height()+1;
            int portionSize = max(2*MaxLocalLength(height,p),mpi::MIN_COLL_MSG);

            int colShiftVRDest = Shift(g.VRRank(),colAlignDest,p);
            int colShiftVCSource = Shift(g.VCRank(),colAlignSource,p);
            int sendRankRM = (g.VRRank()+(p+colShiftVCSource-colShiftVRDest))%p;
            int recvRankCM = (g.VCRank()+(p+colShiftVRDest-colShiftVCSource))%p;
            int recvRankRM = (recvRankCM/r)+c*(recvRankCM%r);

            vector<R> transposeBuffer( (r+1)*portionSize );
            R* sendBuf = &transposeBuffer[0];
            R* recvBuf = &transposeBuffer[r*portionSize];

            // (w01Last[VC,* ] <- w01Last[MC,* ]) and
            // ([a01; 0][VC,* ] <- [a01; 0][MC,* ])
            {
                // Pack the necessary portion of w01Last[MC,* ]
                int shift = Shift(g.VCRank(),colAlignSource,p);
                int offset = (shift-colShiftSource)/r;
                int w01LocalHeight = LocalLength(height,shift,p);
                const R* w01LastBuffer = w01Last_MC_Star.LocalBuffer(offset,0);
                for( int i=0; i<w01LocalHeight; ++i )
                    sendBuf[i] = w01LastBuffer[i*c];
                
                // Pack the necessary portion of a01[MC,* ]
                int a01LocalHeight = LocalLength(height-1,shift,p);
                const R* a01Buffer = a01_MC_Star.LocalBuffer(offset,0);
                for( int i=0; i<a01LocalHeight; ++i )
                    sendBuf[w01LocalHeight+i] = a01Buffer[i*c];
            }

            // [VR,* ] <- [VC,* ]
            mpi::SendRecv
            ( sendBuf, portionSize, sendRankRM, 0,
              recvBuf, portionSize, recvRankRM, mpi::ANY_TAG, g.VRComm() );

            // [MR,* ] <- [VR,* ]
            mpi::AllGather
            ( recvBuf, portionSize,
              sendBuf, portionSize, g.MCComm() );

            // Unpack
            w01Last_MR_Star.AlignWith( A00 );
            w01Last_MR_Star.ResizeTo( a01.Height()+1, 1 );
            for( int k=0; k<r; ++k )
            {
                // Unpack into w01Last[MR,* ]
                const R* w01Data = &sendBuf[k*portionSize];
                int shift = Shift(g.MRRank()+c*k,colAlignDest,p);
                int offset = (shift-colShiftDest) / c;
                int w01LocalHeight = LocalLength(height,shift,p);
                R* w01LastBuffer = w01Last_MR_Star.LocalBuffer(offset,0);
                for( int i=0; i<w01LocalHeight; ++i )
                    w01LastBuffer[i*r] = w01Data[i];

                // Unpack into a01[MR,* ]
                const R* a01Data = &sendBuf[k*portionSize+w01LocalHeight];
                int a01LocalHeight = LocalLength(height-1,shift,p);
                R* a01Buffer = a01_MR_Star.LocalBuffer(offset,0);
                for( int i=0; i<a01LocalHeight; ++i )
                    a01Buffer[i*r] = a01Data[i];
            }
            // Store w01Last[MR,* ]
            memcpy
            ( W_MR_Star.LocalBuffer(0,W00.Width()+1),
              w01Last_MR_Star.LocalBuffer(),
              w01Last_MR_Star.LocalHeight()*sizeof(R) );
            // Store a01[MR,* ]
            memcpy
            ( APan_MR_Star.LocalBuffer(0,W00.Width()),
              a01_MR_Star.LocalBuffer(),
              a01_MR_Star.LocalHeight()*sizeof(R) );

            // Update the portion of A00 that is in our current panel with 
            // w01Last and a01Last using two gers. We do not need their bottom
            // entries. We trash the lower triangle of our panel of A since we 
            // are only doing slightly more work and we can replace it
            // afterwards.
            DistMatrix<R,MC,Star> a01Last_MC_Star_Top(g);
            DistMatrix<R,MR,Star> a01Last_MR_Star_TopPan(g);
            DistMatrix<R,MC,Star> w01Last_MC_Star_Top(g);
            DistMatrix<R,MR,Star> w01Last_MR_Star_TopPan(g);
            a01Last_MC_Star_Top.View
            ( a01Last_MC_Star, 0, 0, a01.Height(), 1 );
            a01Last_MR_Star_TopPan.View
            ( a01Last_MR_Star, topSize, 0, a01.Height()-topSize, 1 );
            w01Last_MC_Star_Top.View
            ( w01Last_MC_Star, 0, 0, a01.Height(), 1 );
            w01Last_MR_Star_TopPan.View
            ( w01Last_MR_Star, topSize, 0, a01.Height()-topSize, 1 );
            const R* a01_MC_Star_Buffer = a01Last_MC_Star_Top.LocalBuffer();
            const R* a01_MR_Star_Buffer = a01Last_MR_Star_TopPan.LocalBuffer();
            const R* w01_MC_Star_Buffer = w01Last_MC_Star_Top.LocalBuffer();
            const R* w01_MR_Star_Buffer = w01Last_MR_Star_TopPan.LocalBuffer();
            R* A00PanBuffer = A00Pan.LocalBuffer();
            int localHeight = A00Pan.LocalHeight();
            int localWidth = A00Pan.LocalWidth();
            int lDim = A00Pan.LocalLDim();
            for( int jLocal=0; jLocal<localWidth; ++jLocal )
                for( int iLocal=0; iLocal<localHeight; ++iLocal )
                    A00PanBuffer[iLocal+jLocal*lDim] -=
                        w01_MC_Star_Buffer[iLocal]*a01_MR_Star_Buffer[jLocal] +
                        a01_MC_Star_Buffer[iLocal]*w01_MR_Star_Buffer[jLocal];

            // We are through with the last iteration's w01
            w01Last_MC_Star.FreeAlignments();
            w01Last_MR_Star.FreeAlignments();
        }

        // Form the local portions of (A00 a01) into p01[MC,* ] and q01[MR,* ]:
        //   p01[MC,* ] := triu(A00)[MC,MR] a01[MR,* ]
        //   q01[MR,* ] := triu(A00,+1)'[MR,MC] a01[MC,* ]
        PopBlocksizeStack();
        p01_MC_Star.SetToZero();
        q01_MR_Star.SetToZero();
        basic::internal::LocalSymvColAccumulateU
        ( (R)1, A00, a01_MC_Star, a01_MR_Star, p01_MC_Star, q01_MR_Star );
        PushBlocksizeStack( 1 );

        basic::Gemv
        ( Transpose, 
          (R)1, W02T.LockedLocalMatrix(),
                a01T_MC_Star.LockedLocalMatrix(),
          (R)0, x21_MR_Star.LocalMatrix() );
        basic::Gemv
        ( Transpose, 
          (R)1, A02T.LockedLocalMatrix(),
                a01T_MC_Star.LockedLocalMatrix(),
          (R)0, y21_MR_Star.LocalMatrix() );
        // Combine the AllReduce column summations of x21[MR,* ], y21[MR,* ],
        // and q01[MR,* ]
        {
            int x21LocalHeight = x21_MR_Star.LocalHeight();
            int y21LocalHeight = y21_MR_Star.LocalHeight();
            int q01LocalHeight = q01_MR_Star.LocalHeight();
            int reduceSize = x21LocalHeight+y21LocalHeight+q01LocalHeight;
            vector<R> colSummationSendBuffer(reduceSize);
            vector<R> colSummationRecvBuffer(reduceSize);
            memcpy
            ( &colSummationSendBuffer[0], 
              x21_MR_Star.LocalBuffer(), 
              x21LocalHeight*sizeof(R) );
            memcpy
            ( &colSummationSendBuffer[x21LocalHeight],
              y21_MR_Star.LocalBuffer(), 
              y21LocalHeight*sizeof(R) );
            memcpy
            ( &colSummationSendBuffer[x21LocalHeight+y21LocalHeight],
              q01_MR_Star.LocalBuffer(), 
              q01LocalHeight*sizeof(R) );
            mpi::AllReduce
            ( &colSummationSendBuffer[0], 
              &colSummationRecvBuffer[0],
              reduceSize, mpi::SUM, g.MCComm() );
            memcpy
            ( x21_MR_Star.LocalBuffer(), 
              &colSummationRecvBuffer[0], 
              x21LocalHeight*sizeof(R) );
            memcpy
            ( y21_MR_Star.LocalBuffer(), 
              &colSummationRecvBuffer[x21LocalHeight], 
              y21LocalHeight*sizeof(R) );
            memcpy
            ( q01_MR_Star.LocalBuffer(), 
              &colSummationRecvBuffer[x21LocalHeight+y21LocalHeight], 
              q01LocalHeight*sizeof(R) );
        }

        basic::Gemv
        ( Normal, 
          (R)-1, A02T.LockedLocalMatrix(),
                 x21_MR_Star.LockedLocalMatrix(),
          (R)+1, p01T_MC_Star.LocalMatrix() );
        basic::Gemv
        ( Normal, 
          (R)-1, W02T.LockedLocalMatrix(),
                 y21_MR_Star.LockedLocalMatrix(),
          (R)+1, p01T_MC_Star.LocalMatrix() );

        if( W00.Width() > 0 )
        {
            // This is not the last iteration of the panel factorization, 
            // combine the Reduce to one of p01[MC,* ] with the redistribution 
            // of q01[MR,* ] -> q01[MC,MR] to the next process column.
            int localHeight = p01_MC_Star.LocalHeight();
            vector<R> reduceToOneSendBuffer(2*localHeight);
            vector<R> reduceToOneRecvBuffer(2*localHeight);

            // Pack p01[MC,* ]
            memcpy
            ( &reduceToOneSendBuffer[0], 
              p01_MC_Star.LocalBuffer(),
              localHeight*sizeof(R) );

            // Fill in contributions to q01[MC,MR] from q01[MR,* ]
            bool contributing = 
                ( q01_MR_Star.ColShift() % g.GCD() ==
                  p01_MC_Star.ColShift() % g.GCD() );
            if( contributing )
            {
                if( r == c )
                {
                    memcpy
                    ( &reduceToOneSendBuffer[localHeight],
                      q01_MR_Star.LocalBuffer(), 
                      localHeight*sizeof(R) );
                }
                else
                {
                    // Zero the entire buffer first
                    memset
                    ( &reduceToOneSendBuffer[localHeight], 0,
                      localHeight*sizeof(R) );
                    // Fill in the entries that we contribute to.
                    // We seek to find the minimum s in N such that
                    //   s*c = a0-b0 (mod r)
                    // where a0 is the column shift of MC, b0 is the row shift
                    // of MR, and s is our first local entry of MR that will 
                    // contribute to MC. I cannot think of an O(1) method, so
                    // I will instead use the worst-case O(lcm(c,r)/c) method.
                    int sourcePeriod = g.LCM() / c;
                    int targetPeriod = g.LCM() / r;
                    int a0 = p01_MC_Star.ColShift();
                    int b0 = q01_MR_Star.ColShift();

                    int sourceStart = 0;
                    int f = (r+a0-b0) % r;
                    for( int s=0; s<sourcePeriod; ++s )
                    {
                        if( (s*c) % r == f )
                        {
                            sourceStart = s;
                            break;
                        }
                    }

                    int globalShift = b0+sourceStart*c;
                    int targetStart = (globalShift-a0)/r;
                    int localLength =
                        LocalLength(localHeight,targetStart,targetPeriod);
                    const R* q01_MR_Star_LocalBuffer = 
                        q01_MR_Star.LocalBuffer();
                    int offset = localHeight + targetStart;
                    for( int i=0; i<localLength; ++i )                        
                        reduceToOneSendBuffer[offset+i*targetPeriod] = 
                            q01_MR_Star_LocalBuffer[sourceStart+i*sourcePeriod];
                }
            }
            else
            {
                memset
                ( &reduceToOneSendBuffer[localHeight], 0, 
                  localHeight*sizeof(R) );
            }

            int nextProcessRow = (alpha11.ColAlignment()+r-1) % r;
            int nextProcessCol = (alpha11.RowAlignment()+c-1) % c;
            mpi::Reduce
            ( &reduceToOneSendBuffer[0], 
              &reduceToOneRecvBuffer[0],
              2*localHeight, mpi::SUM, nextProcessCol, g.MRComm() );
            if( g.MRRank() == nextProcessCol )
            {
                // Combine the second half into the first half        
                for( int i=0; i<localHeight; ++i )
                    reduceToOneRecvBuffer[i] +=
                        reduceToOneRecvBuffer[i+localHeight];

                // Finish computing w01. During its computation, ensure that 
                // every process has a copy of the bottom element of the w01.
                // We know a priori that the bottom element of a01 is one.
                const R* a01_MC_Star_LocalBuffer = a01_MC_Star.LocalBuffer();
                R myDotProduct = blas::Dot
                    ( localHeight, &reduceToOneRecvBuffer[0], 1, 
                                   a01_MC_Star_LocalBuffer,   1 );
                R sendBuffer[2];
                R recvBuffer[2];
                sendBuffer[0] = myDotProduct;
                sendBuffer[1] = ( g.MCRank()==nextProcessRow ? 
                                  reduceToOneRecvBuffer[localHeight-1] : 0 );
                mpi::AllReduce
                ( sendBuffer, recvBuffer, 2, mpi::SUM, g.MCComm() );
                R dotProduct = recvBuffer[0];

                // Set up for the next iteration by filling in the values for:
                // - w01LastLocalBuffer
                // - w01LastBottomEntry
                R scale = 0.5*dotProduct*tau;
                for( int i=0; i<localHeight; ++i )
                    w01LastLocalBuffer[i] = tau*
                        ( reduceToOneRecvBuffer[i]-
                          scale*a01_MC_Star_LocalBuffer[i] );
                w01LastBottomEntry = tau*( recvBuffer[1]-scale );
            }
        }
        else
        {
            // This is the last iteration, our last task is to finish forming
            // w01[MC,* ] and w01[MR,* ] so that we may place them into W[MC,* ]
            // and W[MR,* ]
            int localHeight = p01_MC_Star.LocalHeight();
            vector<R> allReduceSendBuffer(2*localHeight);
            vector<R> allReduceRecvBuffer(2*localHeight);

            // Pack p01[MC,* ]
            memcpy
            ( &allReduceSendBuffer[0], 
              p01_MC_Star.LocalBuffer(),
              localHeight*sizeof(R) );

            // Fill in contributions to q01[MC,* ] from q01[MR,* ]
            bool contributing = 
                ( q01_MR_Star.ColShift() % g.GCD() ==
                  p01_MC_Star.ColShift() % g.GCD() );
            if( contributing )
            {
                if( r == c )
                {
                    memcpy
                    ( &allReduceSendBuffer[localHeight],
                      q01_MR_Star.LocalBuffer(), 
                      localHeight*sizeof(R) );
                }
                else
                {
                    // Zero the entire buffer first
                    memset
                    ( &allReduceSendBuffer[localHeight], 0, 
                      localHeight*sizeof(R) );
                    // Fill in the entries that we contribute to.
                    // We seek to find the minimum s in N such that
                    //   s*c = a0-b0 (mod r)
                    // where a0 is the column shift of MC, b0 is the row shift
                    // of MR, and s is our first local entry of MR that will 
                    // contribute to MC. I cannot think of an O(1) method, so
                    // I will instead use the worst-case O(lcm(c,r)/c) method.
                    int sourcePeriod = g.LCM() / c;
                    int targetPeriod = g.LCM() / r;
                    int a0 = p01_MC_Star.ColShift();
                    int b0 = q01_MR_Star.ColShift();

                    int sourceStart = 0;
                    int f = (r+a0-b0) % r;
                    for( int s=0; s<sourcePeriod; ++s )
                    {
                        if( (s*c) % r == f )
                        {
                            sourceStart = s;
                            break;
                        }
                    }

                    int globalShift = b0+sourceStart*c;
                    int targetStart = (globalShift-a0)/r;
                    int localLength = 
                        LocalLength(localHeight,targetStart,targetPeriod);
                    const R* q01_MR_Star_LocalBuffer = 
                        q01_MR_Star.LocalBuffer();
                    int offset = localHeight + targetStart;
                    for( int i=0; i<localLength; ++i )
                        allReduceSendBuffer[offset+i*targetPeriod] = 
                            q01_MR_Star_LocalBuffer[sourceStart+i*sourcePeriod];
                }
            }
            else
            {
                memset
                ( &allReduceSendBuffer[localHeight], 0, 
                  localHeight*sizeof(R) );
            }

            mpi::AllReduce
            ( &allReduceSendBuffer[0], 
              &allReduceRecvBuffer[0],
              2*localHeight, mpi::SUM, g.MRComm() );

            // Combine the second half into the first half        
            for( int i=0; i<localHeight; ++i )
                allReduceRecvBuffer[i] += allReduceRecvBuffer[i+localHeight];
 
            // Finish computing w01. 
            const R* a01_MC_Star_LocalBuffer = a01_MC_Star.LocalBuffer();
            R myDotProduct = blas::Dot
                ( localHeight, &allReduceRecvBuffer[0], 1, 
                               a01_MC_Star_LocalBuffer, 1 );
            R dotProduct;
            mpi::AllReduce
            ( &myDotProduct, &dotProduct, 1, mpi::SUM, g.MCComm() );

            // Grab views into W[MC,* ] and W[MR,* ]
            DistMatrix<R,MC,Star> w01_MC_Star(g);
            DistMatrix<R,MR,Star> w01_MR_Star(g);
            w01_MC_Star.View( W_MC_Star, 0, W00.Width(), w01.Height(), 1 );
            w01_MR_Star.View( W_MR_Star, 0, W00.Width(), w01.Height(), 1 );

            // Store w01[MC,* ]
            R scale = 0.5*dotProduct*tau;
            R* w01_MC_Star_LocalBuffer = w01_MC_Star.LocalBuffer();
            for( int i=0; i<localHeight; ++i )
                w01_MC_Star_LocalBuffer[i] = tau*
                    ( allReduceRecvBuffer[i]-
                      scale*a01_MC_Star_LocalBuffer[i] );

            // Form w01[MR,* ]
            w01_MR_Star = w01_MC_Star;
        }
        //--------------------------------------------------------------------//
        a01_MC_Star.FreeAlignments();
        a01_MR_Star.FreeAlignments();
        p01_MC_Star.FreeAlignments();
        p01_MR_Star.FreeAlignments();
        q01_MC_Star.FreeAlignments();
        q01_MR_Star.FreeAlignments();
        x21_MR_Star.FreeAlignments();
        y21_MR_Star.FreeAlignments();

        SlidePartitionUpDiagonal
        ( ATL, /**/ ATR,  A00, /**/ a01,     A02,
         /*************/ /**********************/
               /**/       a10, /**/ alpha11, a12,
          ABL, /**/ ABR,  A20, /**/ a21,     A22 );

        SlidePartitionUpDiagonal
        ( WTL, /**/ WTR,  W00, /**/ w01,     W02,
         /*************/ /**********************/
               /**/       w10, /**/ omega11, w12,
          WBL, /**/ WBR,  W20, /**/ w21,     W22 );
        
        SlidePartitionUp
        ( eT,  e0,
         /**/ /********/
               epsilon1,
          eB,  e2 );
        firstIteration = false;
    }
    PopBlocksizeStack();

    expandedABR.SetDiagonal( e, 1 );
#ifndef RELEASE
    PopCallStack();
#endif
}

#ifndef WITHOUT_COMPLEX
template<typename R> // representation of a real number
void
elemental::advanced::internal::PanelTridiagU
( DistMatrix<complex<R>,MC,MR  >& A,
  DistMatrix<complex<R>,MC,MR  >& W,
  DistMatrix<complex<R>,MD,Star>& t,
  DistMatrix<complex<R>,MC,Star>& APan_MC_Star, 
  DistMatrix<complex<R>,MR,Star>& APan_MR_Star,
  DistMatrix<complex<R>,MC,Star>& W_MC_Star,
  DistMatrix<complex<R>,MR,Star>& W_MR_Star )
{
    const int panelSize = W.Width();
    const int topSize = W.Height()-panelSize;
#ifndef RELEASE
    PushCallStack("advanced::internal::PanelTridiagU");
    if( A.Grid() != W.Grid() || W.Grid() != t.Grid() )
        throw logic_error
        ("A, W, and t must be distributed over the same grid.");
    if( A.Height() != A.Width() )
        throw logic_error("A must be square.");
    if( A.Height() != W.Height() )
        throw logic_error( "A and W must be the same height.");
    if( W.Height() < panelSize )
        throw logic_error("W must be a column panel.");
    if( t.Height() != W.Width() || t.Width() != 1 )
        throw logic_error
        ("t must be a column vector of the same length as W's width.");
#endif
    typedef complex<R> C;

    const Grid& g = A.Grid();
    const int r = g.Height();
    const int c = g.Width();
    const int p = g.Size();

    // Create a distributed matrix for storing the superdiagonal
    DistMatrix<R,MD,Star> e(g);
    DistMatrix<C,MC,MR> expandedABR(g);
    expandedABR.View( A, topSize-1, topSize-1, panelSize+1, panelSize+1 );
    e.AlignWithDiag( expandedABR, 1 );
    e.ResizeTo( panelSize, 1 );

    // Matrix views 
    DistMatrix<C,MC,MR> 
        ATL(g), ATR(g),  A00(g), a01(g),     A02(g),  ACol(g), a01Top(g),
        ABL(g), ABR(g),  a10(g), alpha11(g), a12(g),           alpha01Bottom(g),
                         A20(g), a21(g),     A22(g),  A02T(g), A00Pan(g);
    DistMatrix<C,MC,MR> 
        WTL(g), WTR(g),  W00(g), w01(g),     W02(g),  WCol(g),
        WBL(g), WBR(g),  w10(g), omega11(g), w12(g),
                         W20(g), w21(g),     W22(g),  W02T(g), w01Last(g);
    DistMatrix<R,MD,Star> eT(g),  e0(g),
                          eB(g),  epsilon1(g),
                                  e2(g);
    DistMatrix<C,MD,Star>
        tT(g),  t0(g),
        tB(g),  tau1(g),
                t2(g);

    // Temporary distributions
    vector<C> w01LastLocalBuffer(A.Height()/r+1);
    DistMatrix<C,MC,Star> a01_MC_Star(g);
    DistMatrix<C,MC,Star> a01T_MC_Star(g);
    DistMatrix<C,MR,Star> a01_MR_Star(g);
    DistMatrix<C,MC,Star> p01_MC_Star(g);
    DistMatrix<C,MC,Star> p01T_MC_Star(g);
    DistMatrix<C,MR,Star> p01_MR_Star(g);
    DistMatrix<C,MC,Star> q01_MC_Star(g);
    DistMatrix<C,MR,Star> q01_MR_Star(g);
    DistMatrix<C,MR,Star> x21_MR_Star(g);
    DistMatrix<C,MR,Star> y21_MR_Star(g);
    DistMatrix<C,MC,Star> a01Last_MC_Star(g);
    DistMatrix<C,MR,Star> a01Last_MR_Star(g);
    DistMatrix<C,MC,Star> w01Last_MC_Star(g);
    DistMatrix<C,MR,Star> w01Last_MR_Star(g);

    // Push to the blocksize of 1, then pop at the end of the routine
    PushBlocksizeStack( 1 );

    PartitionUpRightDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    PartitionUpRightDiagonal
    ( W, WTL, WTR,
         WBL, WBR, 0 );
    PartitionUp
    ( e, eT,
         eB, 0 );
    PartitionUp
    ( t, tT,
         tB, 0 );
    bool firstIteration = true;
    C tau = 0;
    C w01LastBottomEntry = 0;
    while( WBR.Width() < panelSize )
    {
        RepartitionUpDiagonal
        ( ATL, /**/ ATR,  A00, a01,     /**/ A02,
               /**/       a10, alpha11, /**/ a12, 
         /*************/ /**********************/
          ABL, /**/ ABR,  A20, a21,     /**/ A22 );
        
        RepartitionUpDiagonal
        ( WTL, /**/ WTR,  W00, w01,     /**/ W02,
               /**/       w10, omega11, /**/ w12,
         /*************/ /**********************/
          WBL, /**/ WBR,  W20, w21,     /**/ W22 );

        RepartitionUp
        ( eT,  e0,
               epsilon1,
         /**/ /********/
          eB,  e2 );

        RepartitionUp
        ( tT,  t0,
               tau1,
         /**/ /****/
          tB,  t2 );

        ACol.View2x1
        ( a01,
          alpha11 );

        WCol.View2x1
        ( w01,
          omega11 );

        // View the portions of A02 and W0T outside of this panel's square
        A02T.View( A02, 0, 0, topSize, A02.Width() );
        W02T.View( W02, 0, 0, topSize, W02.Width() );

        // View the portion of A00 inside the current panel
        A00Pan.View( A00, 0, topSize, A00.Height(), A00.Width()-topSize );

        if( !firstIteration )
        {
            a01Last_MC_Star.View
            ( APan_MC_Star, 0, WTL.Width(), ACol.Height(), 1 );
            a01Last_MR_Star.View
            ( APan_MR_Star, 0, WTL.Width(), ACol.Height(), 1 );
            w01Last.View
            ( W, 0, WTL.Width(), ACol.Height(), 1 );
        }
            
        PartitionUp
        ( a01, a01Top,
               alpha01Bottom, 1 );

        a01_MC_Star.AlignWith( A00 );
        a01_MR_Star.AlignWith( A00 );
        p01_MC_Star.AlignWith( A00 );
        p01_MR_Star.AlignWith( A00 );
        q01_MC_Star.AlignWith( A00 );
        q01_MR_Star.AlignWith( A00 );
        a01_MC_Star.ResizeTo( a01.Height(), 1 );
        a01_MR_Star.ResizeTo( a01.Height(), 1 );
        p01_MC_Star.ResizeTo( a01.Height(), 1 );
        p01_MR_Star.ResizeTo( a01.Height(), 1 );
        q01_MC_Star.ResizeTo( a01.Height(), 1 );
        q01_MR_Star.ResizeTo( a01.Height(), 1 );
        x21_MR_Star.AlignWith( A02T );
        y21_MR_Star.AlignWith( A02T );
        x21_MR_Star.ResizeTo( A02.Width(), 1 );
        y21_MR_Star.ResizeTo( A02.Width(), 1 );

        // View the portions of a01[MC,* ] and p01[MC,* ] above the current
        // panel's square
        a01T_MC_Star.View( a01_MC_Star, 0, 0, topSize, 1 );
        p01T_MC_Star.View( p01_MC_Star, 0, 0, topSize, 1 );
        //--------------------------------------------------------------------//
        const bool thisIsMyCol = ( g.MRRank() == alpha11.RowAlignment() );
        if( thisIsMyCol )
        {
            if( !firstIteration )
            {
                // Finish updating the current column with two axpy's
                int AColLocalHeight = ACol.LocalHeight();
                C* AColLocalBuffer = ACol.LocalBuffer();
                const C* a01Last_MC_Star_LocalBuffer = 
                    a01Last_MC_Star.LocalBuffer();
                for( int i=0; i<AColLocalHeight; ++i )
                    AColLocalBuffer[i] -=
                        w01LastLocalBuffer[i] + 
                        a01Last_MC_Star_LocalBuffer[i]*Conj(w01LastBottomEntry);
            }
        }
        if( thisIsMyCol )
        {
            // Compute the Householder reflector
            tau = advanced::internal::ColReflector( alpha01Bottom, a01Top );
            if( g.MCRank() == alpha01Bottom.ColAlignment() )
                tau1.SetLocalEntry(0,0,tau);
        }
        // Store the subdiagonal value and turn a01 into a proper scaled 
        // reflector by explicitly placing the implicit one in its first entry.
        alpha01Bottom.GetRealDiagonal( epsilon1 );
        alpha01Bottom.Set( 0, 0, (C)1 );

        // If this is the first iteration, have each member of the owning 
        // process column broadcast tau and a01 within its process row. 
        // Otherwise, also add w01 into the broadcast.
        if( firstIteration )
        {
            const int a01LocalHeight = a01.LocalHeight();
            vector<C> rowBroadcastBuffer(a01LocalHeight+1);
            if( thisIsMyCol )
            {
                // Pack the broadcast buffer with a01 and tau
                memcpy
                ( &rowBroadcastBuffer[0], 
                  a01.LocalBuffer(), 
                  a01LocalHeight*sizeof(C) );
                rowBroadcastBuffer[a01LocalHeight] = tau;
            }
            // Broadcast a01 and tau across the process row
            mpi::Broadcast
            ( &rowBroadcastBuffer[0], 
              a01LocalHeight+1, a01.RowAlignment(), g.MRComm() );
            // Store a01[MC,* ] into its DistMatrix class and also store a copy
            // for the next iteration
            memcpy
            ( a01_MC_Star.LocalBuffer(), 
              &rowBroadcastBuffer[0],
              a01LocalHeight*sizeof(C) );
            // Store a01[MC,* ] into APan[MC,* ]
            memcpy
            ( APan_MC_Star.LocalBuffer(0,W00.Width()), 
              &rowBroadcastBuffer[0],
              a01LocalHeight*sizeof(C) );
            // Store tau
            tau = rowBroadcastBuffer[a01LocalHeight];
            
            a01_MR_Star = a01_MC_Star;
            // Store a01[MR,* ]
            memcpy
            ( APan_MR_Star.LocalBuffer(0,W00.Width()),
              a01_MR_Star.LocalBuffer(),
              a01_MR_Star.LocalHeight()*sizeof(C) );
        }
        else
        {
            const int a01LocalHeight = a01.LocalHeight();
            const int w01LastLocalHeight = ACol.LocalHeight();
            vector<C> rowBroadcastBuffer(a01LocalHeight+w01LastLocalHeight+1);
            if( thisIsMyCol ) 
            {
                // Pack the broadcast buffer with a01, w01Last, and tau
                memcpy
                ( &rowBroadcastBuffer[0], 
                  a01.LocalBuffer(),
                  a01LocalHeight*sizeof(C) );
                memcpy
                ( &rowBroadcastBuffer[a01LocalHeight], 
                  &w01LastLocalBuffer[0],
                  w01LastLocalHeight*sizeof(C) );
                rowBroadcastBuffer[a01LocalHeight+w01LastLocalHeight] = tau;
            }
            // Broadcast a01, w01Last, and tau across the process row
            mpi::Broadcast
            ( &rowBroadcastBuffer[0], 
              a01LocalHeight+w01LastLocalHeight+1, 
              a01.RowAlignment(), g.MRComm() );
            // Store a01[MC,* ] into its DistMatrix class 
            memcpy
            ( a01_MC_Star.LocalBuffer(), 
              &rowBroadcastBuffer[0],
              a01LocalHeight*sizeof(C) );
            // Store a01[MC,* ] into APan[MC,* ]
            memcpy
            ( APan_MC_Star.LocalBuffer(0,W00.Width()), 
              &rowBroadcastBuffer[0],
              a01LocalHeight*sizeof(C) );
            // Store w01Last[MC,* ] into its DistMatrix class
            w01Last_MC_Star.AlignWith( A00 );
            w01Last_MC_Star.ResizeTo( a01.Height()+1, 1 );
            memcpy
            ( w01Last_MC_Star.LocalBuffer(), 
              &rowBroadcastBuffer[a01LocalHeight], 
              w01LastLocalHeight*sizeof(C) );
            // Store the bottom part of w01Last[MC,* ] into WB[MC,* ] and, 
            // if necessary, w01.
            memcpy
            ( W_MC_Star.LocalBuffer(0,W00.Width()+1),
              &rowBroadcastBuffer[a01LocalHeight],
              w01LastLocalHeight*sizeof(C) );
            if( g.MRRank() == w01Last.RowAlignment() )
            {
                memcpy
                ( w01Last.LocalBuffer(),
                  &rowBroadcastBuffer[a01LocalHeight],
                  w01LastLocalHeight*sizeof(C) );
            }
            // Store tau
            tau = rowBroadcastBuffer[a01LocalHeight+w01LastLocalHeight];

            // Form a01[MR,* ] and w01Last[MR,* ] by combining the 
            // communications needed for taking a vector from 
            // [MC,* ] -> [MR,* ]: 
            //   local copy to [VC,* ], 
            //   Send/Recv to [VR,* ], 
            //   AllGather to [MR,* ]
            // We can combine the two by treating a01 as [a01; 0] 

            int colAlignSource = A00.ColAlignment();
            int colAlignDest = A00.RowAlignment();
            int colShiftSource = A00.ColShift();
            int colShiftDest = A00.RowShift();

            int height = a01.Height()+1;
            int portionSize = max(2*MaxLocalLength(height,p),mpi::MIN_COLL_MSG);

            int colShiftVRDest = Shift(g.VRRank(),colAlignDest,p);
            int colShiftVCSource = Shift(g.VCRank(),colAlignSource,p);
            int sendRankRM = (g.VRRank()+(p+colShiftVCSource-colShiftVRDest))%p;
            int recvRankCM = (g.VCRank()+(p+colShiftVRDest-colShiftVCSource))%p;
            int recvRankRM = (recvRankCM/r)+c*(recvRankCM%r);

            vector<C> transposeBuffer( (r+1)*portionSize );
            C* sendBuf = &transposeBuffer[0];
            C* recvBuf = &transposeBuffer[r*portionSize];

            // (w01Last[VC,* ] <- w01Last[MC,* ]) and
            // ([a01; 0][VC,* ] <- [a01; 0][MC,* ])
            {
                // Pack the necessary portion of w01Last[MC,* ]
                int shift = Shift(g.VCRank(),colAlignSource,p);
                int offset = (shift-colShiftSource)/r;
                int w01LocalHeight = LocalLength(height,shift,p);
                const C* w01LastBuffer = w01Last_MC_Star.LocalBuffer(offset,0);
                for( int i=0; i<w01LocalHeight; ++i )
                    sendBuf[i] = w01LastBuffer[i*c];
                
                // Pack the necessary portion of a01[MC,* ]
                int a01LocalHeight = LocalLength(height-1,shift,p);
                const C* a01Buffer = a01_MC_Star.LocalBuffer(offset,0);
                for( int i=0; i<a01LocalHeight; ++i )
                    sendBuf[w01LocalHeight+i] = a01Buffer[i*c];
            }

            // [VR,* ] <- [VC,* ]
            mpi::SendRecv
            ( sendBuf, portionSize, sendRankRM, 0,
              recvBuf, portionSize, recvRankRM, mpi::ANY_TAG, g.VRComm() );

            // [MR,* ] <- [VR,* ]
            mpi::AllGather
            ( recvBuf, portionSize,
              sendBuf, portionSize, g.MCComm() );

            // Unpack
            w01Last_MR_Star.AlignWith( A00 );
            w01Last_MR_Star.ResizeTo( a01.Height()+1, 1 );
            for( int k=0; k<r; ++k )
            {
                // Unpack into w01Last[MR,* ]
                const C* w01Data = &sendBuf[k*portionSize];
                int shift = Shift(g.MRRank()+c*k,colAlignDest,p);
                int offset = (shift-colShiftDest) / c;
                int w01LocalHeight = LocalLength(height,shift,p);
                C* w01LastBuffer = w01Last_MR_Star.LocalBuffer(offset,0);
                for( int i=0; i<w01LocalHeight; ++i )
                    w01LastBuffer[i*r] = w01Data[i];

                // Unpack into a01[MR,* ]
                const C* a01Data = &sendBuf[k*portionSize+w01LocalHeight];
                int a01LocalHeight = LocalLength(height-1,shift,p);
                C* a01Buffer = a01_MR_Star.LocalBuffer(offset,0);
                for( int i=0; i<a01LocalHeight; ++i )
                    a01Buffer[i*r] = a01Data[i];
            }
            // Store w01Last[MR,* ]
            memcpy
            ( W_MR_Star.LocalBuffer(0,W00.Width()+1),
              w01Last_MR_Star.LocalBuffer(),
              w01Last_MR_Star.LocalHeight()*sizeof(C) );
            // Store a01[MR,* ]
            memcpy
            ( APan_MR_Star.LocalBuffer(0,W00.Width()),
              a01_MR_Star.LocalBuffer(),
              a01_MR_Star.LocalHeight()*sizeof(C) );

            // Update the portion of A00 that is in our current panel with 
            // w01Last and a01Last using two gers. We do not need their bottom
            // entries. We trash the lower triangle of our panel of A since we 
            // are only doing slightly more work and we can replace it
            // afterwards.
            DistMatrix<C,MC,Star> a01Last_MC_Star_Top(g);
            DistMatrix<C,MR,Star> a01Last_MR_Star_TopPan(g);
            DistMatrix<C,MC,Star> w01Last_MC_Star_Top(g);
            DistMatrix<C,MR,Star> w01Last_MR_Star_TopPan(g);
            a01Last_MC_Star_Top.View
            ( a01Last_MC_Star, 0, 0, a01.Height(), 1 );            
            a01Last_MR_Star_TopPan.View
            ( a01Last_MR_Star, topSize, 0, a01.Height()-topSize, 1 );
            w01Last_MC_Star_Top.View
            ( w01Last_MC_Star, 0, 0, a01.Height(), 1 );
            w01Last_MR_Star_TopPan.View
            ( w01Last_MR_Star, topSize, 0, a01.Height()-topSize, 1 );
            const C* a01_MC_Star_Buffer = a01Last_MC_Star_Top.LocalBuffer();
            const C* a01_MR_Star_Buffer = a01Last_MR_Star_TopPan.LocalBuffer();
            const C* w01_MC_Star_Buffer = w01Last_MC_Star_Top.LocalBuffer();
            const C* w01_MR_Star_Buffer = w01Last_MR_Star_TopPan.LocalBuffer();
            C* A00PanBuffer = A00Pan.LocalBuffer();
            int localHeight = A00Pan.LocalHeight();
            int localWidth = A00Pan.LocalWidth();
            int lDim = A00Pan.LocalLDim();
            for( int jLocal=0; jLocal<localWidth; ++jLocal )
                for( int iLocal=0; iLocal<localHeight; ++iLocal )
                    A00PanBuffer[iLocal+jLocal*lDim] -=
                        w01_MC_Star_Buffer[iLocal]*
                        Conj(a01_MR_Star_Buffer[jLocal]) +
                        a01_MC_Star_Buffer[iLocal]*
                        Conj(w01_MR_Star_Buffer[jLocal]);

            // We are through with the last iteration's w01
            w01Last_MC_Star.FreeAlignments();
            w01Last_MR_Star.FreeAlignments();
        }

        // Form the local portions of (A00 a01) into p01[MC,* ] and q01[MR,* ]:
        //   p01[MC,* ] := triu(A00)[MC,MR] a01[MR,* ]
        //   q01[MR,* ] := triu(A00,+1)'[MR,MC] a01[MC,* ]
        PopBlocksizeStack();
        p01_MC_Star.SetToZero();
        q01_MR_Star.SetToZero();
        basic::internal::LocalHemvColAccumulateU
        ( (C)1, A00, a01_MC_Star, a01_MR_Star, p01_MC_Star, q01_MR_Star );
        PushBlocksizeStack( 1 );

        basic::Gemv
        ( ConjugateTranspose, 
          (C)1, W02T.LockedLocalMatrix(),
                a01T_MC_Star.LockedLocalMatrix(),
          (C)0, x21_MR_Star.LocalMatrix() );
        basic::Gemv
        ( ConjugateTranspose, 
          (C)1, A02T.LockedLocalMatrix(),
                a01T_MC_Star.LockedLocalMatrix(),
          (C)0, y21_MR_Star.LocalMatrix() );
        // Combine the AllReduce column summations of x21[MR,* ], y21[MR,* ],
        // and q01[MR,* ]
        {
            int x21LocalHeight = x21_MR_Star.LocalHeight();
            int y21LocalHeight = y21_MR_Star.LocalHeight();
            int q01LocalHeight = q01_MR_Star.LocalHeight();
            int reduceSize = x21LocalHeight+y21LocalHeight+q01LocalHeight;
            vector<C> colSummationSendBuffer(reduceSize);
            vector<C> colSummationRecvBuffer(reduceSize);
            memcpy
            ( &colSummationSendBuffer[0], 
              x21_MR_Star.LocalBuffer(), 
              x21LocalHeight*sizeof(C) );
            memcpy
            ( &colSummationSendBuffer[x21LocalHeight],
              y21_MR_Star.LocalBuffer(), 
              y21LocalHeight*sizeof(C) );
            memcpy
            ( &colSummationSendBuffer[x21LocalHeight+y21LocalHeight],
              q01_MR_Star.LocalBuffer(), 
              q01LocalHeight*sizeof(C) );
            mpi::AllReduce
            ( &colSummationSendBuffer[0], 
              &colSummationRecvBuffer[0],
              reduceSize, mpi::SUM, g.MCComm() );
            memcpy
            ( x21_MR_Star.LocalBuffer(), 
              &colSummationRecvBuffer[0], 
              x21LocalHeight*sizeof(C) );
            memcpy
            ( y21_MR_Star.LocalBuffer(), 
              &colSummationRecvBuffer[x21LocalHeight], 
              y21LocalHeight*sizeof(C) );
            memcpy
            ( q01_MR_Star.LocalBuffer(), 
              &colSummationRecvBuffer[x21LocalHeight+y21LocalHeight], 
              q01LocalHeight*sizeof(C) );
        }

        basic::Gemv
        ( Normal, 
          (C)-1, A02T.LockedLocalMatrix(),
                 x21_MR_Star.LockedLocalMatrix(),
          (C)+1, p01T_MC_Star.LocalMatrix() );
        basic::Gemv
        ( Normal, 
          (C)-1, W02T.LockedLocalMatrix(),
                 y21_MR_Star.LockedLocalMatrix(),
          (C)+1, p01T_MC_Star.LocalMatrix() );

        if( W00.Width() > 0 )
        {
            // This is not the last iteration of the panel factorization, 
            // combine the Reduce to one of p01[MC,* ] with the redistribution 
            // of q01[MR,* ] -> q01[MC,MR] to the next process column.
            int localHeight = p01_MC_Star.LocalHeight();
            vector<C> reduceToOneSendBuffer(2*localHeight);
            vector<C> reduceToOneRecvBuffer(2*localHeight);

            // Pack p01[MC,* ]
            memcpy
            ( &reduceToOneSendBuffer[0], 
              p01_MC_Star.LocalBuffer(),
              localHeight*sizeof(C) );

            // Fill in contributions to q01[MC,MR] from q01[MR,* ]
            bool contributing = 
                ( q01_MR_Star.ColShift() % g.GCD() ==
                  p01_MC_Star.ColShift() % g.GCD() );
            if( contributing )
            {
                if( r == c )
                {
                    memcpy
                    ( &reduceToOneSendBuffer[localHeight],
                      q01_MR_Star.LocalBuffer(), 
                      localHeight*sizeof(C) );
                }
                else
                {
                    // Zero the entire buffer first
                    memset
                    ( &reduceToOneSendBuffer[localHeight], 0,
                      localHeight*sizeof(C) );
                    // Fill in the entries that we contribute to.
                    // We seek to find the minimum s in N such that
                    //   s*c = a0-b0 (mod r)
                    // where a0 is the column shift of MC, b0 is the row shift
                    // of MR, and s is our first local entry of MR that will 
                    // contribute to MC. I cannot think of an O(1) method, so
                    // I will instead use the worst-case O(lcm(c,r)/c) method.
                    int sourcePeriod = g.LCM() / c;
                    int targetPeriod = g.LCM() / r;
                    int a0 = p01_MC_Star.ColShift();
                    int b0 = q01_MR_Star.ColShift();

                    int sourceStart = 0;
                    int f = (r+a0-b0) % r;
                    for( int s=0; s<sourcePeriod; ++s )
                    {
                        if( (s*c) % r == f )
                        {
                            sourceStart = s;
                            break;
                        }
                    }

                    int globalShift = b0+sourceStart*c;
                    int targetStart = (globalShift-a0)/r;
                    int localLength =
                        LocalLength(localHeight,targetStart,targetPeriod);
                    const C* q01_MR_Star_LocalBuffer = 
                        q01_MR_Star.LocalBuffer();
                    int offset = localHeight + targetStart;
                    for( int i=0; i<localLength; ++i )                        
                        reduceToOneSendBuffer[offset+i*targetPeriod] = 
                            q01_MR_Star_LocalBuffer[sourceStart+i*sourcePeriod];
                }
            }
            else
            {
                memset
                ( &reduceToOneSendBuffer[localHeight], 0, 
                  localHeight*sizeof(C) );
            }

            int nextProcessRow = (alpha11.ColAlignment()+r-1) % r;
            int nextProcessCol = (alpha11.RowAlignment()+c-1) % c;
            mpi::Reduce
            ( &reduceToOneSendBuffer[0], 
              &reduceToOneRecvBuffer[0],
              2*localHeight, mpi::SUM, nextProcessCol, g.MRComm() );
            if( g.MRRank() == nextProcessCol )
            {
                // Combine the second half into the first half        
                for( int i=0; i<localHeight; ++i )
                    reduceToOneRecvBuffer[i] +=
                        reduceToOneRecvBuffer[i+localHeight];

                // Finish computing w01. During its computation, ensure that 
                // every process has a copy of the last element of the w01.
                // We know a priori that the last element of a01 is one.
                const C* a01_MC_Star_LocalBuffer = a01_MC_Star.LocalBuffer();
                C myDotProduct = blas::Dot
                    ( localHeight, &reduceToOneRecvBuffer[0], 1, 
                                   &a01_MC_Star_LocalBuffer[0], 1 );
                C sendBuffer[2];
                C recvBuffer[2];
                sendBuffer[0] = myDotProduct;
                sendBuffer[1] = ( g.MCRank()==nextProcessRow ? 
                                  reduceToOneRecvBuffer[localHeight-1] : 0 );
                mpi::AllReduce
                ( sendBuffer, recvBuffer, 2, mpi::SUM, g.MCComm() );
                C dotProduct = recvBuffer[0];

                // Set up for the next iteration by filling in the values for:
                // - w01LastLocalBuffer
                // - w01LastBottomEntry
                C scale = static_cast<C>(0.5)*dotProduct*Conj(tau);
                for( int i=0; i<localHeight; ++i )
                    w01LastLocalBuffer[i] = tau*
                        ( reduceToOneRecvBuffer[i]-
                          scale*a01_MC_Star_LocalBuffer[i] );
                w01LastBottomEntry = tau*( recvBuffer[1]-scale );
            }
        }
        else
        {
            // This is the last iteration, our last task is to finish forming
            // w01[MC,* ] and w01[MR,* ] so that we may place them into W[MC,* ]
            // and W[MR,* ]
            int localHeight = p01_MC_Star.LocalHeight();
            vector<C> allReduceSendBuffer(2*localHeight);
            vector<C> allReduceRecvBuffer(2*localHeight);

            // Pack p01[MC,* ]
            memcpy
            ( &allReduceSendBuffer[0], 
              p01_MC_Star.LocalBuffer(),
              localHeight*sizeof(C) );

            // Fill in contributions to q01[MC,* ] from q01[MR,* ]
            bool contributing = 
                ( q01_MR_Star.ColShift() % g.GCD() ==
                  p01_MC_Star.ColShift() % g.GCD() );
            if( contributing )
            {
                if( r == c )
                {
                    memcpy
                    ( &allReduceSendBuffer[localHeight],
                      q01_MR_Star.LocalBuffer(), 
                      localHeight*sizeof(C) );
                }
                else
                {
                    // Zero the entire buffer first
                    memset
                    ( &allReduceSendBuffer[localHeight], 0, 
                      localHeight*sizeof(C) );
                    // Fill in the entries that we contribute to.
                    // We seek to find the minimum s in N such that
                    //   s*c = a0-b0 (mod r)
                    // where a0 is the column shift of MC, b0 is the row shift
                    // of MR, and s is our first local entry of MR that will 
                    // contribute to MC. I cannot think of an O(1) method, so
                    // I will instead use the worst-case O(lcm(c,r)/c) method.
                    int sourcePeriod = g.LCM() / c;
                    int targetPeriod = g.LCM() / r;
                    int a0 = p01_MC_Star.ColShift();
                    int b0 = q01_MR_Star.ColShift();

                    int sourceStart = 0;
                    int f = (r+a0-b0) % r;
                    for( int s=0; s<sourcePeriod; ++s )
                    {
                        if( (s*c) % r == f )
                        {
                            sourceStart = s;
                            break;
                        }
                    }

                    int globalShift = b0+sourceStart*c;
                    int targetStart = (globalShift-a0)/r;
                    int localLength = 
                        LocalLength(localHeight,targetStart,targetPeriod);
                    const C* q01_MR_Star_LocalBuffer = 
                        q01_MR_Star.LocalBuffer();
                    int offset = localHeight + targetStart;
                    for( int i=0; i<localLength; ++i )
                        allReduceSendBuffer[offset+i*targetPeriod] = 
                            q01_MR_Star_LocalBuffer[sourceStart+i*sourcePeriod];
                }
            }
            else
            {
                memset
                ( &allReduceSendBuffer[localHeight], 0, 
                  localHeight*sizeof(C) );
            }

            mpi::AllReduce
            ( &allReduceSendBuffer[0], 
              &allReduceRecvBuffer[0],
              2*localHeight, mpi::SUM, g.MRComm() );

            // Combine the second half into the first half        
            for( int i=0; i<localHeight; ++i )
                allReduceRecvBuffer[i] += allReduceRecvBuffer[i+localHeight];
 
            // Finish computing w01. During its computation, ensure that 
            // every process has a copy of the last element of the w01.
            // We know a priori that the last element of a01 is one.
            const C* a01_MC_Star_LocalBuffer = a01_MC_Star.LocalBuffer();
            C myDotProduct = blas::Dot
                ( localHeight, &allReduceRecvBuffer[0], 1, 
                               a01_MC_Star_LocalBuffer, 1 );
            C dotProduct;
            mpi::AllReduce
            ( &myDotProduct, &dotProduct, 1, mpi::SUM, g.MCComm() );

            // Grab views into W[MC,* ] and W[MR,* ]
            DistMatrix<C,MC,Star> w01_MC_Star(g);
            DistMatrix<C,MR,Star> w01_MR_Star(g);
            w01_MC_Star.View( W_MC_Star, 0, W00.Width(), w01.Height(), 1 );
            w01_MR_Star.View( W_MR_Star, 0, W00.Width(), w01.Height(), 1 );

            // Store w01[MC,* ]
            C scale = static_cast<C>(0.5)*dotProduct*Conj(tau);
            C* w01_MC_Star_LocalBuffer = w01_MC_Star.LocalBuffer();
            for( int i=0; i<localHeight; ++i )
                w01_MC_Star_LocalBuffer[i] = tau*
                    ( allReduceRecvBuffer[i]-
                      scale*a01_MC_Star_LocalBuffer[i] );

            // Form w01[MR,* ]
            w01_MR_Star = w01_MC_Star;
        }
        //--------------------------------------------------------------------//
        a01_MC_Star.FreeAlignments();
        a01_MR_Star.FreeAlignments();
        p01_MC_Star.FreeAlignments();
        p01_MR_Star.FreeAlignments();
        q01_MC_Star.FreeAlignments();
        q01_MR_Star.FreeAlignments();
        x21_MR_Star.FreeAlignments();
        y21_MR_Star.FreeAlignments();

        SlidePartitionUpDiagonal
        ( ATL, /**/ ATR,  A00, /**/ a01,     A02,
         /*************/ /**********************/
               /**/       a10, /**/ alpha11, a12,
          ABL, /**/ ABR,  A20, /**/ a21,     A22 );

        SlidePartitionUpDiagonal
        ( WTL, /**/ WTR,  W00, /**/ w01,     W02,
         /*************/ /**********************/
               /**/       w10, /**/ omega11, w12,
          WBL, /**/ WBR,  W20, /**/ w21,     W22 );
        
        SlidePartitionUp
        ( eT,  e0,
         /**/ /********/
               epsilon1,
          eB,  e2 );

        SlidePartitionUp
        ( tT,  t0,
         /**/ /****/
               tau1,
          tB,  t2 );
        firstIteration = false;
    }
    PopBlocksizeStack();

    expandedABR.SetRealDiagonal( e, 1 );
#ifndef RELEASE
    PopCallStack();
#endif
}
#endif // WITHOUT_COMPLEX

template void elemental::advanced::internal::PanelTridiagU
( DistMatrix<float,MC,MR  >& A,
  DistMatrix<float,MC,MR  >& W,
  DistMatrix<float,MC,Star>& APan_MC_Star,
  DistMatrix<float,MR,Star>& APan_MR_Star,
  DistMatrix<float,MC,Star>& W_MC_Star,
  DistMatrix<float,MR,Star>& W_MR_Star );

template void elemental::advanced::internal::PanelTridiagU
( DistMatrix<double,MC,MR  >& A,
  DistMatrix<double,MC,MR  >& W,
  DistMatrix<double,MC,Star>& APan_MC_Star,
  DistMatrix<double,MR,Star>& APan_MR_Star,
  DistMatrix<double,MC,Star>& W_MC_Star,
  DistMatrix<double,MR,Star>& W_MR_Star );

#ifndef WITHOUT_COMPLEX
template void elemental::advanced::internal::PanelTridiagU
( DistMatrix<scomplex,MC,MR  >& A,
  DistMatrix<scomplex,MC,MR  >& W,
  DistMatrix<scomplex,MD,Star>& t,
  DistMatrix<scomplex,MC,Star>& APan_MC_Star,
  DistMatrix<scomplex,MR,Star>& APan_MR_Star,
  DistMatrix<scomplex,MC,Star>& W_MC_Star,
  DistMatrix<scomplex,MR,Star>& W_MR_Star );

template void elemental::advanced::internal::PanelTridiagU
( DistMatrix<dcomplex,MC,MR  >& A,
  DistMatrix<dcomplex,MC,MR  >& W,
  DistMatrix<dcomplex,MD,Star>& t,
  DistMatrix<dcomplex,MC,Star>& APan_MC_Star,
  DistMatrix<dcomplex,MR,Star>& APan_MR_Star,
  DistMatrix<dcomplex,MC,Star>& W_MC_Star,
  DistMatrix<dcomplex,MR,Star>& W_MR_Star );
#endif
