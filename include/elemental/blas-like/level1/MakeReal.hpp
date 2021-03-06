/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef BLAS_MAKEREAL_HPP
#define BLAS_MAKEREAL_HPP

namespace elem {

template <typename Int>
void MakeReal( AutoMatrix<Int>& A );

template<typename T>
inline void
MakeReal( Matrix<T>& A )
{
#ifndef RELEASE
    PushCallStack("MakeReal");
#endif
    T* ABuffer = A.Buffer();
    const int height = A.Height();
    const int width = A.Width();
    const int ldim = A.LDim();
    for( int j=0; j<width; ++j )
        for( int i=0; i<height; ++i )
            ABuffer[i+j*ldim] = RealPart(ABuffer[i+j*ldim]);
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T,Distribution U,Distribution V>
inline void
MakeReal( DistMatrix<T,U,V>& A )
{
#ifndef RELEASE
    PushCallStack("MakeReal");
#endif
    MakeReal( A.Matrix() );
#ifndef RELEASE
    PopCallStack();
#endif
}

} // namespace elem

#endif // ifndef BLAS_MAKEREAL_HPP
