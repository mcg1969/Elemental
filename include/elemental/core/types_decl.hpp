/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef CORE_TYPES_DECL_HPP
#define CORE_TYPES_DECL_HPP

namespace elem {

typedef unsigned char byte;
 
typedef Complex<float>  scomplex; 
typedef Complex<double> dcomplex;

namespace data_type_wrapper {

enum ScalarTypes
{
	INTEGRAL,
#ifndef DISABLE_FLOAT
	SINGLE,
#endif	
	DOUBLE,
#ifndef DISABLE_COMPLEX
#ifndef DISABLE_FLOAT	
	SCOMPLEX,
#endif	
	DCOMPLEX,
#endif	
	UNKNOWN
};

template <typename T,enum ScalarTypes S,typename Int=int>
struct ScalarTypeBase {
	typedef T Type;
	typedef T RealType;
	typedef Int IntegralType;
	static const size_t SizeOf = sizeof(T);
	static const ScalarTypes Enum = S;
	static const bool isValid = false;
	static const bool isComplex = false;
	static const bool canBeComplex = false;
};

template <typename T,typename Int=int>
struct ScalarType : public ScalarTypeBase<T,UNKNOWN,Int>
{};

template <enum ScalarTypes S,typename Int=int>
struct ScalarEnum : public ScalarTypeBase<void*,S,Int>
{};

std::string ScalarTypeToString( ScalarTypes stype );

}
using namespace data_type_wrapper;

// For the safe computation of products. The result is given by 
//   product = rho * exp(kappa*n)
// where rho lies in (usually on) the unit circle and kappa is real-valued.
template<typename F,typename Int=int>
struct SafeProduct
{
    F rho;
    typename Base<F>::type kappa;
    Int n;

    SafeProduct( Int numEntries );
};

namespace conjugation_wrapper {
enum Conjugation
{
    UNCONJUGATED,
    CONJUGATED
};
}
using namespace conjugation_wrapper;

namespace distribution_wrapper {
enum Distribution
{
    MC,  // Col of a matrix distribution
    MD,  // Diagonal of a matrix distribution
    MR,  // Row of a matrix distribution
    VC,  // Col-major vector distribution
    VR,  // Row-major vector distribution
    STAR // Do not distribute
};
enum Distribution2D 
{
	MC_MR,
	MC_STAR,
	MD_STAR,
	MR_MC,
	MR_STAR,
	STAR_MC,
	STAR_MD,
	STAR_MR,
	STAR_STAR,
	STAR_VC,
	STAR_VR,
	VC_STAR,
	VR_STAR
};
template <Distribution U,Distribution V,Distribution2D W>
struct DistMapBase {
	static const Distribution RowDist = STAR;
	static const Distribution ColDist = STAR;
	static const Distribution2D Dist2D = STAR_STAR;
	static const bool isValid = false;
};
template <Distribution U,Distribution V>
struct DistMap : public DistMapBase<U,V,STAR_STAR> {};
template <Distribution2D U>
struct Dist2DMap : public DistMapBase<STAR,STAR,U> {};
std::string DistToString( Distribution distribution );
std::string Dist2DToString( Distribution2D distribution );
Distribution StringToDist( std::string s );
Distribution2D StringToDist2D( std::string s );
}
using namespace distribution_wrapper;

namespace forward_or_backward_wrapper {
enum ForwardOrBackward
{
    FORWARD,
    BACKWARD
};
}
using namespace forward_or_backward_wrapper;

namespace grid_order_wrapper {
enum GridOrder
{
    ROW_MAJOR,
    COLUMN_MAJOR
};
}
using namespace grid_order_wrapper;

namespace left_or_right_wrapper {
enum LeftOrRight
{
    LEFT,
    RIGHT
};
char LeftOrRightToChar( LeftOrRight side );
LeftOrRight CharToLeftOrRight( char c );
}
using namespace left_or_right_wrapper;

namespace norm_type_wrapper {
enum NormType
{
    ONE_NORM,           // Operator one norm
    INFINITY_NORM,      // Operator infinity norm
    ENTRYWISE_ONE_NORM, // One-norm of vectorized matrix
    MAX_NORM,           // Maximum entry-wise magnitude
    NUCLEAR_NORM,       // One-norm of the singular values
    FROBENIUS_NORM,     // Two-norm of the singular values
    TWO_NORM            // Infinity-norm of the singular values
};
}
using namespace norm_type_wrapper;

namespace orientation_wrapper {
enum Orientation
{
    NORMAL,
    TRANSPOSE,
    ADJOINT
};
char OrientationToChar( Orientation orientation );
Orientation CharToOrientation( char c );
}
using namespace orientation_wrapper;

namespace unit_or_non_unit_wrapper {
enum UnitOrNonUnit
{
    NON_UNIT,
    UNIT
};
char UnitOrNonUnitToChar( UnitOrNonUnit diag );
UnitOrNonUnit CharToUnitOrNonUnit( char c );
}
using namespace unit_or_non_unit_wrapper;

namespace upper_or_lower_wrapper {
enum UpperOrLower
{
    LOWER,
    UPPER
};
char UpperOrLowerToChar( UpperOrLower uplo );
UpperOrLower CharToUpperOrLower( char c );
}
using namespace upper_or_lower_wrapper;

namespace vertical_or_horizontal_wrapper {
enum VerticalOrHorizontal
{
    VERTICAL,
    HORIZONTAL
};
}
using namespace vertical_or_horizontal_wrapper;

} // namespace elem

#endif // ifndef CORE_TYPES_DECL_HPP
