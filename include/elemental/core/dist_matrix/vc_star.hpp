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
#ifndef ELEMENTAL_DIST_MATRIX_VC_STAR_HPP
#define ELEMENTAL_DIST_MATRIX_VC_STAR_HPP 1

namespace elemental {

// Partial specialization to A[VC,* ].
//
// The columns of these distributed matrices are spread throughout the 
// process grid in a column-major fashion, while the rows are not 
// distributed.
template<typename T>
class DistMatrix<T,VC,STAR> : public AbstractDistMatrix<T>
{
public:
    // Create a 0 x 0 distributed matrix
    DistMatrix( const elemental::Grid& g );

    // Create a height x width distributed matrix
    DistMatrix( int height, int width, const elemental::Grid& g );

    // Create a 0 x 0 distributed matrix with specified alignments
    DistMatrix
    ( bool constrainedColAlignment,
      int colAlignment, const elemental::Grid& g );

    // Create a height x width distributed matrix with specified alignments
    DistMatrix
    ( int height, int width, bool constrainedColAlignment, int colAlignment,
      const elemental::Grid& g );

    // Create a height x width distributed matrix with specified alignments
    // and leading dimension
    DistMatrix
    ( int height, int width, bool constrainedColAlignment, int colAlignment,
      int ldim, const elemental::Grid& g );

    // View a constant distributed matrix's buffer
    DistMatrix
    ( int height, int width, int colAlignment,
      const T* buffer, int ldim, const elemental::Grid& g );

    // View a mutable distributed matrix's buffer
    DistMatrix
    ( int height, int width, int colAlignment,
      T* buffer, int ldim, const elemental::Grid& g );

    // Create a copy of distributed matrix A
    DistMatrix( const DistMatrix<T,VC,STAR>& A );

    ~DistMatrix();

    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,MC,MR>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,MC,STAR>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,STAR,MR>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,MD,STAR>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,STAR,MD>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,MR,MC>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,MR,STAR>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,STAR,MC>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,VC,STAR>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,STAR,VC>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,VR,STAR>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,STAR,VR>& A );
    const DistMatrix<T,VC,STAR>& operator=( const DistMatrix<T,STAR,STAR>& A );

    //------------------------------------------------------------------------//
    // Fulfillments of abstract virtual func's from AbstractDistMatrix        //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    // Every process receives a copy of global entry (i,j)
    virtual T Get( int i, int j ) const;
    // Every process contributes the new value of global entry (i,j)
    virtual void Set( int i, int j, T alpha );
    // Every process contributes the update to global entry (i,j),
    // i.e., A(i,j) += alpha
    virtual void Update( int i, int j, T alpha );

    virtual void MakeTrapezoidal
    ( Side side, Shape shape, int offset = 0 );

    virtual void ScaleTrapezoidal
    ( T alpha, Side side, Shape shape, int offset = 0 );

    virtual void ResizeTo( int height, int width );
    virtual void SetToIdentity();
    virtual void SetToRandom();
    virtual void SetToRandomHermitian();
    virtual void SetToRandomHPD();

    //
    // Routines that are only valid for complex datatypes
    //

    virtual typename RealBase<T>::type GetReal( int i, int j ) const;
    virtual typename RealBase<T>::type GetImag( int i, int j ) const;
    virtual void SetReal( int i, int j, typename RealBase<T>::type u );
    virtual void SetImag( int i, int j, typename RealBase<T>::type u );
    virtual void UpdateReal( int i, int j, typename RealBase<T>::type u );
    virtual void UpdateImag( int i, int j, typename RealBase<T>::type u );

    //------------------------------------------------------------------------//
    // Routines specific to [VC,* ] distribution                              //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    // Set the alignments
    void Align( int colAlignment );
    void AlignCols( int colAlignment );

    // Aligns all of our DistMatrix's distributions that match a distribution
    // of the argument DistMatrix.
    template<typename S> void AlignWith( const DistMatrix<S,MC,  MR  >& A );
    template<typename S> void AlignWith( const DistMatrix<S,MR,  MC  >& A );
    template<typename S> void AlignWith( const DistMatrix<S,MC,  STAR>& A );
    template<typename S> void AlignWith( const DistMatrix<S,STAR,MC  >& A );
    template<typename S> void AlignWith( const DistMatrix<S,VC,  STAR>& A );
    template<typename S> void AlignWith( const DistMatrix<S,STAR,VC  >& A );
    template<typename S> void AlignWith( const DistMatrix<S,STAR,MD  >& A ) {}
    template<typename S> void AlignWith( const DistMatrix<S,STAR,MR  >& A ) {}
    template<typename S> void AlignWith( const DistMatrix<S,STAR,VR  >& A ) {}
    template<typename S> void AlignWith( const DistMatrix<S,STAR,STAR>& A ) {}
    template<typename S> void AlignWith( const DistMatrix<S,MD,  STAR>& A ) {}
    template<typename S> void AlignWith( const DistMatrix<S,MR,  STAR>& A ) {}
    template<typename S> void AlignWith( const DistMatrix<S,VR,  STAR>& A ) {}

    // Aligns our column distribution (i.e., VC) with the matching distribution
    // of the argument. We recognize that a VC distribution can be a subset of
    // an MC distribution.
    template<typename S> void AlignColsWith( const DistMatrix<S,MC,  MR  >& A );
    template<typename S> void AlignColsWith( const DistMatrix<S,MR,  MC  >& A );
    template<typename S> void AlignColsWith( const DistMatrix<S,MC,  STAR>& A );
    template<typename S> void AlignColsWith( const DistMatrix<S,STAR,MC  >& A );
    template<typename S> void AlignColsWith( const DistMatrix<S,VC,  STAR>& A );
    template<typename S> void AlignColsWith( const DistMatrix<S,STAR,VC  >& A );

    // Aligns our row distribution (i.e., STAR) with the matching distribution
    // of the argument. These are no-ops and exist solely to allow for
    // templating over distribution parameters.
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,STAR,MC  >& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,STAR,MD  >& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,STAR,MR  >& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,STAR,VC  >& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,STAR,VR  >& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,STAR,STAR>& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,MC,  STAR>& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,MD,  STAR>& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,MR,  STAR>& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,VC,  STAR>& A ) {}
    template<typename S>
    void AlignRowsWith( const DistMatrix<S,VR,  STAR>& A ) {}

    // (Immutable) view of a distributed matrix
    void View( DistMatrix<T,VC,STAR>& A );
    void LockedView( const DistMatrix<T,VC,STAR>& A );

    // (Immutable) view of a portion of a distributed matrix
    void View( DistMatrix<T,VC,STAR>& A, int i, int j, int height, int width );
    void LockedView
    ( const DistMatrix<T,VC,STAR>& A, int i, int j, int height, int width );

    // (Immutable) view of two horizontally contiguous partitions of a
    // distributed matrix
    void View1x2
    ( DistMatrix<T,VC,STAR>& AL, DistMatrix<T,VC,STAR>& AR );
    void LockedView1x2
    ( const DistMatrix<T,VC,STAR>& AL, const DistMatrix<T,VC,STAR>& AR );

    // (Immutable) view of two vertically contiguous partitions of a
    // distributed matrix
    void View2x1
    ( DistMatrix<T,VC,STAR>& AT,
      DistMatrix<T,VC,STAR>& AB );
    void LockedView2x1
    ( const DistMatrix<T,VC,STAR>& AT,
      const DistMatrix<T,VC,STAR>& AB );

    // (Immutable) view of a contiguous 2x2 set of partitions of a
    // distributed matrix
    void View2x2
    ( DistMatrix<T,VC,STAR>& ATL, DistMatrix<T,VC,STAR>& ATR,
      DistMatrix<T,VC,STAR>& ABL, DistMatrix<T,VC,STAR>& ABR );
    void LockedView2x2
    ( const DistMatrix<T,VC,STAR>& ATL, const DistMatrix<T,VC,STAR>& ATR,
      const DistMatrix<T,VC,STAR>& ABL, const DistMatrix<T,VC,STAR>& ABR );

    void SumScatterFrom( const DistMatrix<T,MC,STAR>& A );
    void SumScatterUpdate( T alpha, const DistMatrix<T,MC,STAR>& A );

private:
    virtual void PrintBase( std::ostream& os, const std::string msg="" ) const;

    // The remainder of this class definition makes use of an idiom that allows
    // for implementing certain routines for (potentially) only complex
    // datatypes.

    template<typename Z>
    struct SetToRandomHermitianHelper
    {
        static void Func( DistMatrix<Z,VC,STAR>& parent );
    };
    template<typename Z>
    struct SetToRandomHermitianHelper<std::complex<Z> >
    {
        static void Func( DistMatrix<std::complex<Z>,VC,STAR>& parent );
    };
    template<typename Z> friend struct SetToRandomHermitianHelper;

    template<typename Z>
    struct SetToRandomHPDHelper
    {
        static void Func( DistMatrix<Z,VC,STAR>& parent );
    };
    template<typename Z>
    struct SetToRandomHPDHelper<std::complex<Z> >
    {
        static void Func( DistMatrix<std::complex<Z>,VC,STAR>& parent );
    };
    template<typename Z> friend struct SetToRandomHPDHelper;

    template<typename Z>
    struct GetRealHelper
    {
        static Z Func( const DistMatrix<Z,VC,STAR>& parent, int i, int j );
    };
    template<typename Z>
    struct GetRealHelper<std::complex<Z> >
    {
        static Z Func
        ( const DistMatrix<std::complex<Z>,VC,STAR>& parent, int i, int j );
    };
    template<typename Z> friend struct GetRealHelper;

    template<typename Z>
    struct GetImagHelper
    {
        static Z Func( const DistMatrix<Z,VC,STAR>& parent, int i, int j );
    };
    template<typename Z>
    struct GetImagHelper<std::complex<Z> >
    {
        static Z Func
        ( const DistMatrix<std::complex<Z>,VC,STAR>& parent, int i, int j );
    };
    template<typename Z> friend struct GetImagHelper;

    template<typename Z>
    struct SetRealHelper
    {
        static void Func
        ( DistMatrix<Z,VC,STAR>& parent, int i, int j, Z alpha );
    };
    template<typename Z>
    struct SetRealHelper<std::complex<Z> >
    {
        static void Func
        ( DistMatrix<std::complex<Z>,VC,STAR>& parent, int i, int j, Z alpha );
    };
    template<typename Z> friend struct SetRealHelper;

    template<typename Z>
    struct SetImagHelper
    {
        static void Func
        ( DistMatrix<Z,VC,STAR>& parent, int i, int j, Z alpha );
    };
    template<typename Z>
    struct SetImagHelper<std::complex<Z> >
    {
        static void Func
        ( DistMatrix<std::complex<Z>,VC,STAR>& parent, int i, int j, Z alpha );
    };
    template<typename Z> friend struct SetImagHelper;

    template<typename Z>
    struct UpdateRealHelper
    {
        static void Func
        ( DistMatrix<Z,VC,STAR>& parent, int i, int j, Z alpha );
    };
    template<typename Z>
    struct UpdateRealHelper<std::complex<Z> >
    {
        static void Func
        ( DistMatrix<std::complex<Z>,VC,STAR>& parent, int i, int j, Z alpha );
    };
    template<typename Z> friend struct UpdateRealHelper;

    template<typename Z>
    struct UpdateImagHelper
    {
        static void Func
        ( DistMatrix<Z,VC,STAR>& parent, int i, int j, Z alpha );
    };
    template<typename Z>
    struct UpdateImagHelper<std::complex<Z> >
    {
        static void Func
        ( DistMatrix<std::complex<Z>,VC,STAR>& parent, int i, int j, Z alpha );
    };
    template<typename Z> friend struct UpdateImagHelper;
};

} // namespace elemental

//----------------------------------------------------------------------------//
// Implementation begins here                                                 //
//----------------------------------------------------------------------------//

#include "./vc_star_main.hpp"
#include "./vc_star_helpers.hpp"

namespace elemental {

template<typename T>
inline
DistMatrix<T,VC,STAR>::DistMatrix( const elemental::Grid& g )
: AbstractDistMatrix<T>
  (0,0,false,false,0,0,
   (g.InGrid() ? g.VCRank() : 0 ),0,
   0,0,g)
{ }

template<typename T>
inline
DistMatrix<T,VC,STAR>::DistMatrix
( int height, int width, const elemental::Grid& g )
: AbstractDistMatrix<T>
  (height,width,false,false,0,0,
   (g.InGrid() ? g.VCRank() : 0),0,
   (g.InGrid() ? LocalLength(height,g.VCRank(),0,g.Size()) : 0),width,
   g)
{ }

template<typename T>
inline
DistMatrix<T,VC,STAR>::DistMatrix
( bool constrainedColAlignment, int colAlignment, const elemental::Grid& g )
: AbstractDistMatrix<T>
  (0,0,constrainedColAlignment,false,colAlignment,0,
   (g.InGrid() ? Shift(g.VCRank(),colAlignment,g.Size()) : 0),0,
   0,0,g)
{ }

template<typename T>
inline
DistMatrix<T,VC,STAR>::DistMatrix
( int height, int width, bool constrainedColAlignment, int colAlignment,
  const elemental::Grid& g )
: AbstractDistMatrix<T>
  (height,width,constrainedColAlignment,false,colAlignment,0,
   (g.InGrid() ? Shift(g.VCRank(),colAlignment,g.Size()) : 0),0,
   (g.InGrid() ? LocalLength(height,g.VCRank(),colAlignment,g.Size()) : 0),
   width,g)
{ }

template<typename T>
inline
DistMatrix<T,VC,STAR>::DistMatrix
( int height, int width, bool constrainedColAlignment, int colAlignment,
  int ldim, const elemental::Grid& g )
: AbstractDistMatrix<T>
  (height,width,constrainedColAlignment,false,colAlignment,0,
   (g.InGrid() ? Shift(g.VCRank(),colAlignment,g.Size()) : 0),0,
   (g.InGrid() ? LocalLength(height,g.VCRank(),colAlignment,g.Size()) : 0),
   width,ldim,g)
{ }

template<typename T>
inline
DistMatrix<T,VC,STAR>::DistMatrix
( int height, int width, int colAlignment, const T* buffer, int ldim,
  const elemental::Grid& g )
: AbstractDistMatrix<T>
  (height,width,colAlignment,0,
   (g.InGrid() ? Shift(g.VCRank(),colAlignment,g.Size()) : 0),0,
   (g.InGrid() ? LocalLength(height,g.VCRank(),colAlignment,g.Size()) : 0),
   width,buffer,ldim,g)
{ }

template<typename T>
inline
DistMatrix<T,VC,STAR>::DistMatrix
( int height, int width, int colAlignment, T* buffer, int ldim,
  const elemental::Grid& g )
: AbstractDistMatrix<T>
  (height,width,colAlignment,0,
   (g.InGrid() ? Shift(g.VCRank(),colAlignment,g.Size()) : 0),0,
   (g.InGrid() ? LocalLength(height,g.VCRank(),colAlignment,g.Size()) : 0),
   width,buffer,ldim,g)
{ }

template<typename T>
inline
DistMatrix<T,VC,STAR>::DistMatrix( const DistMatrix<T,VC,STAR>& A )
: AbstractDistMatrix<T>(0,0,false,false,0,0,0,0,0,0,A.Grid())
{
#ifndef RELEASE
    PushCallStack("DistMatrix[VC,* ]::DistMatrix");
#endif
    if( &A != this )
        *this = A;
    else
        throw std::logic_error("Tried to construct [VC,* ] with itself");
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline
DistMatrix<T,VC,STAR>::~DistMatrix()
{ }

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignWith( const DistMatrix<S,MC,MR>& A )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::AlignWith([MC,MR])");
    this->AssertFreeColAlignment();
    this->AssertSameGrid( A );
#endif
    const elemental::Grid& g = this->Grid();
    this->_colAlignment = A.ColAlignment();
    this->_colShift = Shift( g.VCRank(), this->ColAlignment(), g.Size() );
    this->_constrainedColAlignment = true;
    this->_height = 0;
    this->_width = 0;
    this->_localMatrix.ResizeTo( 0, 0 );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignWith( const DistMatrix<S,MR,MC>& A )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::AlignWith([MR,MC])");
    this->AssertFreeColAlignment();
    this->AssertSameGrid( A );
#endif
    const elemental::Grid& g = this->Grid();
    this->_colAlignment = A.RowAlignment();
    this->_colShift = Shift( g.VCRank(), this->ColAlignment(), g.Size() );
    this->_constrainedColAlignment = true;
    this->_height = 0;
    this->_width = 0;
    this->_localMatrix.ResizeTo( 0, 0 );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignWith( const DistMatrix<S,MC,STAR>& A )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::AlignWith([MC,* ])");
    this->AssertFreeColAlignment();
    this->AssertSameGrid( A );
#endif
    const elemental::Grid& g = this->Grid();
    this->_colAlignment = A.ColAlignment();
    this->_colShift = Shift( g.VCRank(), this->ColAlignment(), g.Size() );
    this->_constrainedColAlignment = true;
    this->_height = 0;
    this->_width = 0;
    this->_localMatrix.ResizeTo( 0, 0 );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignWith( const DistMatrix<S,STAR,MC>& A )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::AlignWith([* ,MC])");
    this->AssertFreeColAlignment();
    this->AssertSameGrid( A );
#endif
    const elemental::Grid& g = this->Grid();
    this->_colAlignment = A.RowAlignment();
    this->_colShift = Shift( g.VCRank(), this->ColAlignment(), g.Size() );
    this->_constrainedColAlignment = true;
    this->_height = 0;
    this->_width = 0;
    this->_localMatrix.ResizeTo( 0, 0 );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignWith( const DistMatrix<S,VC,STAR>& A )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::AlignWith([VC,* ])");
    this->AssertFreeColAlignment();
    this->AssertSameGrid( A );
#endif
    this->_colAlignment = A.ColAlignment();
    this->_colShift = A.ColShift();
    this->_constrainedColAlignment = true;
    this->_height = 0;
    this->_width = 0;
    this->_localMatrix.ResizeTo( 0, 0 );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignWith( const DistMatrix<S,STAR,VC>& A )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::AlignWith([* ,VC])");
    this->AssertFreeColAlignment();
    this->AssertSameGrid( A );
#endif
    this->_colAlignment = A.RowAlignment();
    this->_colShift = A.RowShift();
    this->_constrainedColAlignment = true;
    this->_height = 0;
    this->_width = 0;
    this->_localMatrix.ResizeTo( 0, 0 );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignColsWith( const DistMatrix<S,MC,MR>& A )
{ AlignWith( A ); }

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignColsWith( const DistMatrix<S,MR,MC>& A )
{ AlignWith( A ); }

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignColsWith( const DistMatrix<S,MC,STAR>& A )
{ AlignWith( A ); }

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignColsWith( const DistMatrix<S,STAR,MC>& A )
{ AlignWith( A ); }

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignColsWith( const DistMatrix<S,VC,STAR>& A )
{ AlignWith( A ); }

template<typename T>
template<typename S>
inline void
DistMatrix<T,VC,STAR>::AlignColsWith( const DistMatrix<S,STAR,VC>& A )
{ AlignWith( A ); }

//
// The remainder of the file is for implementing the helpers
//

template<typename T>
inline void
DistMatrix<T,VC,STAR>::SetToRandomHermitian()
{ SetToRandomHermitianHelper<T>::Func( *this ); }

template<typename T>
inline void
DistMatrix<T,VC,STAR>::SetToRandomHPD()
{ SetToRandomHPDHelper<T>::Func( *this ); }

template<typename T>
inline typename RealBase<T>::type
DistMatrix<T,VC,STAR>::GetReal( int i, int j ) const
{ return GetRealHelper<T>::Func( *this, i, j ); }

template<typename T>
template<typename Z>
inline Z
DistMatrix<T,VC,STAR>::GetRealHelper<Z>::Func
( const DistMatrix<Z,VC,STAR>& parent, int i, int j )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::GetRealHelper");
#endif
    throw std::logic_error("Called complex-only routine with real datatype");
}

template<typename T>
inline typename RealBase<T>::type
DistMatrix<T,VC,STAR>::GetImag( int i, int j ) const
{ return GetImagHelper<T>::Func( *this, i, j ); }

template<typename T>
template<typename Z>
inline Z
DistMatrix<T,VC,STAR>::GetImagHelper<Z>::Func
( const DistMatrix<Z,VC,STAR>& parent, int i, int j )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::GetImag");
#endif
    throw std::logic_error("Called complex-only routine with real datatype");
}

template<typename T>
inline void
DistMatrix<T,VC,STAR>::SetReal( int i, int j, typename RealBase<T>::type alpha )
{ SetRealHelper<T>::Func( *this, i, j, alpha ); }

template<typename T>
template<typename Z>
inline void
DistMatrix<T,VC,STAR>::SetRealHelper<Z>::Func
( DistMatrix<Z,VC,STAR>& parent, int i, int j, Z alpha )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::SetReal");
#endif
    throw std::logic_error("Called complex-only routine with real datatype");
}

template<typename T>
inline void
DistMatrix<T,VC,STAR>::SetImag( int i, int j, typename RealBase<T>::type alpha )
{ SetImagHelper<T>::Func( *this, i, j, alpha ); }

template<typename T>
template<typename Z>
inline void
DistMatrix<T,VC,STAR>::SetImagHelper<Z>::Func
( DistMatrix<Z,VC,STAR>& parent, int i, int j, Z alpha )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::SetImag");
#endif
    throw std::logic_error("Called complex-only routine with real datatype");
}

template<typename T>
inline void
DistMatrix<T,VC,STAR>::UpdateReal
( int i, int j, typename RealBase<T>::type alpha )
{ UpdateRealHelper<T>::Func( *this, i, j, alpha ); }

template<typename T>
template<typename Z>
inline void
DistMatrix<T,VC,STAR>::UpdateRealHelper<Z>::Func
( DistMatrix<Z,VC,STAR>& parent, int i, int j, Z alpha )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::UpdateReal");
#endif
    throw std::logic_error("Called complex-only routine with real datatype");
}

template<typename T>
inline void
DistMatrix<T,VC,STAR>::UpdateImag
( int i, int j, typename RealBase<T>::type alpha )
{ UpdateImagHelper<T>::Func( *this, i, j, alpha ); }

template<typename T>
template<typename Z>
inline void
DistMatrix<T,VC,STAR>::UpdateImagHelper<Z>::Func
( DistMatrix<Z,VC,STAR>& parent, int i, int j, Z alpha )
{
#ifndef RELEASE
    PushCallStack("[VC,* ]::UpdateImag");
#endif
    throw std::logic_error("Called complex-only routine with real datatype");
}

} // elemental

#endif /* ELEMENTAL_DIST_MATRIX_VC_STAR_HPP */
