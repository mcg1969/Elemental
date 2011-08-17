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
#ifndef ELEMENTAL_DIST_MATRIX_STAR_MC_HPP
#define ELEMENTAL_DIST_MATRIX_STAR_MC_HPP 1

// Template conventions:
//   G: general datatype
//
//   T: any ring, e.g., the (Gaussian) integers and the real/complex numbers
//   Z: representation of a real ring, e.g., the integers or real numbers
//   std::complex<Z>: representation of a complex ring, e.g. Gaussian integers
//                    or complex numbers
//
//   F: representation of real or complex number
//   R: representation of real number
//   std::complex<R>: representation of complex number

namespace elemental {

// Partial specialization to A[* ,MC] for arbitrary rings.
//
// The columns of these distributed matrices will be replicated on all 
// processes (*), and the rows will be distributed like "Matrix Columns" 
// (MC). Thus the rows will be distributed among columns of the process
// grid.
template<typename T>
class DistMatrixBase<T,STAR,MC> : public AbstractDistMatrix<T>
{
protected:
    typedef AbstractDistMatrix<T> ADM;

    // The basic constructor
    DistMatrixBase
    ( int height, int width, bool constrainedRowAlignment, int rowAlignment,
      const elemental::Grid& g );

    // The basic constructor, but with a supplied leading dimension
    DistMatrixBase
    ( int height, int width, bool constrainedRowAlignment, int rowAlignment,
      int ldim, const elemental::Grid& g );

    // View a constant distributed matrix's buffer
    DistMatrixBase
    ( int height, int width, int rowAlignment,
      const T* buffer, int ldim, const elemental::Grid& g );

    // View a mutable distributed matrix's buffer
    DistMatrixBase
    ( int height, int width, int rowAlignment,
      T* buffer, int ldim, const elemental::Grid& g );

    ~DistMatrixBase();

    virtual void PrintBase( std::ostream& os, const std::string msg="" ) const;

public:
    //------------------------------------------------------------------------//
    // Fulfillments of abstract virtual func's from AbstractDistMatrixBase    //
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
    // i.e.,  A(i,j) += alpha
    virtual void Update( int i, int j, T alpha );

    virtual void MakeTrapezoidal
    ( Side side, Shape shape, int offset = 0 );

    virtual void ScaleTrapezoidal
    ( T alpha, Side side, Shape shape, int offset = 0 );

    virtual void ResizeTo( int height, int width );
    virtual void SetToIdentity();
    virtual void SetToRandom();

    //------------------------------------------------------------------------//
    // Routines specific to [* ,MC] distribution                              //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    // Set the alignments
    void Align( int rowAlignment );
    void AlignRows( int rowAlignment );

    // Aligns all of our DistMatrix's distributions that match a distribution
    // of the argument DistMatrix.
    void AlignWith( const DistMatrixBase<T,MR,  MC  >& A );
    void AlignWith( const DistMatrixBase<T,STAR,MC  >& A );
    void AlignWith( const DistMatrixBase<T,MC,  MR  >& A );
    void AlignWith( const DistMatrixBase<T,MC,  STAR>& A );
    void AlignWith( const DistMatrixBase<T,VC,  STAR>& A );
    void AlignWith( const DistMatrixBase<T,STAR,VC  >& A ); 
    void AlignWith( const DistMatrixBase<T,STAR,MD  >& A ) {}
    void AlignWith( const DistMatrixBase<T,STAR,MR  >& A ) {}
    void AlignWith( const DistMatrixBase<T,STAR,VR  >& A ) {}
    void AlignWith( const DistMatrixBase<T,STAR,STAR>& A ) {}
    void AlignWith( const DistMatrixBase<T,MD,  STAR>& A ) {}
    void AlignWith( const DistMatrixBase<T,MR,  STAR>& A ) {}
    void AlignWith( const DistMatrixBase<T,VR,  STAR>& A ) {}

    // Aligns our column distribution (i.e., Star) with the matching 
    // distribution of the argument. These are all no-ops and exist solely for
    // templating over distribution parameters.
    void AlignColsWith( const DistMatrixBase<T,STAR,MC  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,STAR,MD  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,STAR,MR  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,STAR,VC  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,STAR,VR  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,STAR,STAR>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,MC,  STAR>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,MD,  STAR>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,MR,  STAR>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,VC,  STAR>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,VR,  STAR>& A ) {}

    // Aligns our row distribution (i.e., MC) with the matching distribution
    // of the argument. We recognize that a VC distribution can be a subset 
    // of an MC distribution.
    void AlignRowsWith( const DistMatrixBase<T,MR,  MC  >& A );
    void AlignRowsWith( const DistMatrixBase<T,STAR,MC  >& A );
    void AlignRowsWith( const DistMatrixBase<T,MC,  MR  >& A );
    void AlignRowsWith( const DistMatrixBase<T,MC,  STAR>& A );
    void AlignRowsWith( const DistMatrixBase<T,VC,  STAR>& A );
    void AlignRowsWith( const DistMatrixBase<T,STAR,VC  >& A );

    // (Immutable) view of a distributed matrix
    void View( DistMatrixBase<T,STAR,MC>& A );
    void LockedView( const DistMatrixBase<T,STAR,MC>& A );

    // (Immutable) view of a portion of a distributed matrix
    void View
    ( DistMatrixBase<T,STAR,MC>& A,
      int i, int j, int height, int width );

    void LockedView
    ( const DistMatrixBase<T,STAR,MC>& A,
      int i, int j, int height, int width );

    // (Immutable) view of two horizontally contiguous partitions of a
    // distributed matrix
    void View1x2
    ( DistMatrixBase<T,STAR,MC>& AL, DistMatrixBase<T,STAR,MC>& AR );

    void LockedView1x2
    ( const DistMatrixBase<T,STAR,MC>& AL, 
      const DistMatrixBase<T,STAR,MC>& AR );

    // (Immutable) view of two vertically contiguous partitions of a
    // distributed matrix
    void View2x1
    ( DistMatrixBase<T,STAR,MC>& AT,
      DistMatrixBase<T,STAR,MC>& AB );

    void LockedView2x1
    ( const DistMatrixBase<T,STAR,MC>& AT,
      const DistMatrixBase<T,STAR,MC>& AB );

    // (Immutable) view of a contiguous 2x2 set of partitions of a
    // distributed matrix
    void View2x2
    ( DistMatrixBase<T,STAR,MC>& ATL, DistMatrixBase<T,STAR,MC>& ATR,
      DistMatrixBase<T,STAR,MC>& ABL, DistMatrixBase<T,STAR,MC>& ABR );

    void LockedView2x2
    ( const DistMatrixBase<T,STAR,MC>& ATL, 
      const DistMatrixBase<T,STAR,MC>& ATR,
      const DistMatrixBase<T,STAR,MC>& ABL, 
      const DistMatrixBase<T,STAR,MC>& ABR );

    // AllReduce over process row
    void SumOverRow();

    // Routines needed to implement algorithms that avoid using
    // inefficient unpackings of partial matrix distributions
    void AdjointFrom( const DistMatrixBase<T,VC,STAR>& A );
    void TransposeFrom( const DistMatrixBase<T,VC,STAR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,MC,MR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,MC,STAR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,STAR,MR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,MD,STAR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,STAR,MD>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,MR,MC>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,MR,STAR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,STAR,MC>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,VC,STAR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,STAR,VC>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,VR,STAR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,STAR,VR>& A );

    const DistMatrixBase<T,STAR,MC>&
    operator=( const DistMatrixBase<T,STAR,STAR>& A );
};

// Partial specialization to A[* ,MC] for real rings.
//
// The columns of these distributed matrices will be replicated on all 
// processes (*), and the rows will be distributed like "Matrix Columns" 
// (MC). Thus the rows will be distributed among columns of the process
// grid.
template<typename Z>
class DistMatrix<Z,STAR,MC> : public DistMatrixBase<Z,STAR,MC>
{
protected:
    typedef DistMatrixBase<Z,STAR,MC> DMB;

public:
    // Create a 0 x 0 distributed matrix
    DistMatrix
    ( const elemental::Grid& g );

    // Create a height x width distributed matrix
    DistMatrix
    ( int height, int width, const elemental::Grid& g );

    // Create a 0 x 0 distributed matrix with specified alignments
    DistMatrix
    ( bool constrainedRowAlignment, int rowAlignment, const elemental::Grid& g );

    // Create a height x width distributed matrix with specified alignments
    DistMatrix
    ( int height, int width, bool constrainedRowAlignment, int rowAlignment,
      const elemental::Grid& g );

    // Create a height x width distributed matrix with specified alignments
    // and leading dimension
    DistMatrix
    ( int height, int width, bool constrainedRowAlignment, int rowAlignment,
      int ldim, const elemental::Grid& g );

    // View a constant distributed matrix's buffer
    DistMatrix
    ( int height, int width, int rowAlignment,
      const Z* buffer, int ldim, const elemental::Grid& g );

    // View a mutable distributed matrix's buffer
    DistMatrix
    ( int height, int width, int rowAlignment,
      Z* buffer, int ldim, const elemental::Grid& g );

    // Create a copy of distributed matrix A
    DistMatrix
    ( const DistMatrix<Z,STAR,MC>& A );

    ~DistMatrix();
    
    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,MC,MR>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,MC,STAR>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,STAR,MR>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,MD,STAR>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,STAR,MD>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,MR,MC>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,MR,STAR>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,STAR,MC>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,VC,STAR>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,STAR,VC>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,VR,STAR>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,STAR,VR>& A );

    const DistMatrix<Z,STAR,MC>&
    operator=( const DistMatrix<Z,STAR,STAR>& A );

    //------------------------------------------------------------------------//
    // Fulfillments of abstract virtual func's from AbstractDistMatrixBase    //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    virtual void SetToRandomHermitian();
    virtual void SetToRandomHPD();
};

#ifndef WITHOUT_COMPLEX
// Partial specialization to A[* ,MC] for complex rings.
//
// The columns of these distributed matrices will be replicated on all 
// processes (*), and the rows will be distributed like "Matrix Columns" 
// (MC). Thus the rows will be distributed among columns of the process
// grid.
template<typename Z>
class DistMatrix<std::complex<Z>,STAR,MC> 
: public DistMatrixBase<std::complex<Z>,STAR,MC>
{
protected:
    typedef DistMatrixBase<std::complex<Z>,STAR,MC> DMB;

public:
    // Create a 0 x 0 distributed matrix
    DistMatrix
    ( const elemental::Grid& g );

    // Create a height x width distributed matrix
    DistMatrix
    ( int height, int width, const elemental::Grid& g );

    // Create a 0 x 0 distributed matrix with specified alignments
    DistMatrix
    ( bool constrainedRowAlignment, int rowAlignment, const elemental::Grid& g );

    // Create a height x width distributed matrix with specified alignments
    DistMatrix
    ( int height, int width, bool constrainedRowAlignment, int rowAlignment,
      const elemental::Grid& g );

    // Create a height x width distributed matrix with specified alignments
    // and leading dimension
    DistMatrix
    ( int height, int width, bool constrainedRowAlignment, int rowAlignment,
      int ldim, const elemental::Grid& g );

    // View a constant distributed matrix's buffer
    DistMatrix
    ( int height, int width, int rowAlignment,
      const std::complex<Z>* buffer, int ldim, const elemental::Grid& g );

    // View a mutable distributed matrix's buffer
    DistMatrix
    ( int height, int width, int rowAlignment,
      std::complex<Z>* buffer, int ldim, const elemental::Grid& g );

    // Create a copy of distributed matrix A
    DistMatrix
    ( const DistMatrix<std::complex<Z>,STAR,MC>& A );

    ~DistMatrix();
    
    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,MC,MR>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,MC,STAR>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,STAR,MR>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,MD,STAR>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,STAR,MD>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,MR,MC>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,MR,STAR>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,STAR,MC>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,VC,STAR>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,STAR,VC>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,VR,STAR>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,STAR,VR>& A );

    const DistMatrix<std::complex<Z>,STAR,MC>&
    operator=( const DistMatrix<std::complex<Z>,STAR,STAR>& A );

    //------------------------------------------------------------------------//
    // Fulfillments of abstract virtual func's from AbstractDistMatrixBase    //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    virtual void SetToRandomHermitian();
    virtual void SetToRandomHPD();

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

    // Every process receives the real part of global entry (i,j)
    virtual Z GetReal( int i, int j ) const;
    // Every process receives the imag part of global entry (i,j)
    virtual Z GetImag( int i, int j ) const;
    // Every process contributes the new real part of global entry (i,j)
    virtual void SetReal( int i, int j, Z u );
    // Every process contributes the new imag part of global entry (i,j)
    virtual void SetImag( int i, int j, Z u );
    // Every process contributes the update to the real part of entry (i,j),
    // i.e., real(A(i,j)) += u
    virtual void UpdateReal( int i, int j, Z u );
    // Every process contributes the update to the imag part of entry (i,j),
    // i.e., imag(A(i,j)) += u
    virtual void UpdateImag( int i, int j, Z u );
};
#endif

//----------------------------------------------------------------------------//
// Implementation begins here                                                 //
//----------------------------------------------------------------------------//

//
// DistMatrixBase[* ,MC]
//

template<typename T>
inline
DistMatrixBase<T,STAR,MC>::DistMatrixBase
( int height, int width, bool constrainedRowAlignment, int rowAlignment,
  const elemental::Grid& g )
: ADM(height,width,false,constrainedRowAlignment,0,rowAlignment,
      // column shift
      0,
      // row shift
      ( g.InGrid() ? utilities::Shift(g.MCRank(),rowAlignment,g.Height()) : 0 ),
      // local height
      ( g.InGrid() ? height : 0 ),
      // local width
      ( g.InGrid() ? 
        utilities::LocalLength(width,g.MCRank(),rowAlignment,g.Height()) : 0 ),
      g)
{ }

template<typename T>
inline
DistMatrixBase<T,STAR,MC>::DistMatrixBase
( int height, int width, bool constrainedRowAlignment, int rowAlignment,
  int ldim, const elemental::Grid& g )
: ADM(height,width,false,constrainedRowAlignment,0,rowAlignment,
      // column shift
      0,
      // row shift
      ( g.InGrid() ? utilities::Shift(g.MCRank(),rowAlignment,g.Height()) : 0 ),
      // local height
      ( g.InGrid() ? height : 0 ),
      // local width
      ( g.InGrid() ? 
        utilities::LocalLength(width,g.MCRank(),rowAlignment,g.Height()) : 0 ),
      ldim,g)
{ }

template<typename T>
inline
DistMatrixBase<T,STAR,MC>::DistMatrixBase
( int height, int width, int rowAlignment,
  const T* buffer, int ldim, const elemental::Grid& g )
: ADM(height,width,0,rowAlignment,
      // column shift
      0,
      // row shift
      ( g.InGrid() ? utilities::Shift(g.MCRank(),rowAlignment,g.Height()) : 0 ),
      // local height
      ( g.InGrid() ? height : 0 ),
      // local width
      ( g.InGrid() ? 
        utilities::LocalLength(width,g.MCRank(),rowAlignment,g.Height()) : 0 ),
      buffer,ldim,g)
{ }

template<typename T>
inline
DistMatrixBase<T,STAR,MC>::DistMatrixBase
( int height, int width, int rowAlignment,
  T* buffer, int ldim, const elemental::Grid& g )
: ADM(height,width,0,rowAlignment,
      // column shift
      0,
      // row shift
      ( g.InGrid() ? utilities::Shift(g.MCRank(),rowAlignment,g.Height()) : 0 ),
      // local height
      ( g.InGrid() ? height : 0 ),
      // local width
      ( g.InGrid() ? 
        utilities::LocalLength(width,g.MCRank(),rowAlignment,g.Height()) : 0 ),
      buffer,ldim,g)
{ }

template<typename T>
inline
DistMatrixBase<T,STAR,MC>::~DistMatrixBase()
{ }

//
// Real DistMatrix[* ,MC]
//

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::DistMatrix
( const elemental::Grid& g )
: DMB(0,0,false,0,g)
{ }

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::DistMatrix
( int height, int width, const elemental::Grid& g )
: DMB(height,width,false,0,g)
{ }

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::DistMatrix
( bool constrainedRowAlignment, int rowAlignment, const elemental::Grid& g )
: DMB(0,0,constrainedRowAlignment,rowAlignment,g)
{ }

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::DistMatrix
( int height, int width, bool constrainedRowAlignment, int rowAlignment, 
  const elemental::Grid& g )
: DMB(height,width,constrainedRowAlignment,rowAlignment,g)
{ }

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::DistMatrix
( int height, int width, bool constrainedRowAlignment, int rowAlignment, 
  int ldim, const elemental::Grid& g )
: DMB(height,width,constrainedRowAlignment,rowAlignment,ldim,g)
{ }

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::DistMatrix
( int height, int width, int rowAlignment,
  const Z* buffer, int ldim, const elemental::Grid& g )
: DMB(height,width,rowAlignment,buffer,ldim,g)
{ }

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::DistMatrix
( int height, int width, int rowAlignment,
  Z* buffer, int ldim, const elemental::Grid& g )
: DMB(height,width,rowAlignment,buffer,ldim,g)
{ }

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::DistMatrix
( const DistMatrix<Z,STAR,MC>& A )
: DMB(0,0,false,0,A.Grid())
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MC]::DistMatrix");
#endif
    if( &A != this )
        *this = A;
    else
        throw std::logic_error
        ( "Attempted to construct a [* ,MC] with itself." );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename Z>
inline
DistMatrix<Z,STAR,MC>::~DistMatrix()
{ }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,MC,MR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,MC,STAR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,STAR,MR>& A)
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,MD,STAR>& A)
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,STAR,MD>& A)
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,MR,MC>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,MR,STAR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,STAR,MC>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,VC,STAR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,STAR,VC>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,VR,STAR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,STAR,VR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<Z,STAR,MC>&
DistMatrix<Z,STAR,MC>::operator=
( const DistMatrix<Z,STAR,STAR>& A )
{ DMB::operator=( A ); return *this; }

//
// Complex DistMatrix[* ,MC]
//

#ifndef WITHOUT_COMPLEX
template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::DistMatrix
( const elemental::Grid& g )
: DMB(0,0,false,0,g)
{ }

template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::DistMatrix
( int height, int width, const elemental::Grid& g )
: DMB(height,width,false,0,g)
{ }

template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::DistMatrix
( bool constrainedRowAlignment, int rowAlignment, const elemental::Grid& g )
: DMB(0,0,constrainedRowAlignment,rowAlignment,g)
{ }

template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::DistMatrix
( int height, int width, bool constrainedRowAlignment, int rowAlignment, 
  const elemental::Grid& g )
: DMB(height,width,constrainedRowAlignment,rowAlignment,g)
{ }

template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::DistMatrix
( int height, int width, bool constrainedRowAlignment, int rowAlignment, 
  int ldim, const elemental::Grid& g )
: DMB(height,width,constrainedRowAlignment,rowAlignment,ldim,g)
{ }

template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::DistMatrix
( int height, int width, int rowAlignment,
  const std::complex<Z>* buffer, int ldim, const elemental::Grid& g )
: DMB(height,width,rowAlignment,buffer,ldim,g)
{ }

template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::DistMatrix
( int height, int width, int rowAlignment,
  std::complex<Z>* buffer, int ldim, const elemental::Grid& g )
: DMB(height,width,rowAlignment,buffer,ldim,g)
{ }

template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::DistMatrix
( const DistMatrix<std::complex<Z>,STAR,MC>& A )
: DMB(0,0,false,0,A.Grid())
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MC]::DistMatrix");
#endif
    if( &A != this )
        *this = A;
    else
        throw std::logic_error
        ( "Attempted to construct a [* ,MC] with itself." );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename Z>
inline
DistMatrix<std::complex<Z>,STAR,MC>::~DistMatrix()
{ }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,MC,MR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,MC,STAR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,STAR,MR>& A)
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,MD,STAR>& A)
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,STAR,MD>& A)
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,MR,MC>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,MR,STAR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,STAR,MC>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,VC,STAR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,STAR,VC>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,VR,STAR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,STAR,VR>& A )
{ DMB::operator=( A ); return *this; }

template<typename Z>
inline const DistMatrix<std::complex<Z>,STAR,MC>&
DistMatrix<std::complex<Z>,STAR,MC>::operator=
( const DistMatrix<std::complex<Z>,STAR,STAR>& A )
{ DMB::operator=( A ); return *this; }
#endif // WITHOUT_COMPLEX

} // elemental

#endif /* ELEMENTAL_DIST_MATRIX_STAR_MC_HPP */

