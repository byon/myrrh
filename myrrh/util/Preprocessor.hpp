// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * @file myrrh/utility/Preprocessor.hpp
 * @author Marko Raatikainen
 *
 * This file contains helper functions that have something to do with
 * preprocessor.
 *
 * $Id: Preprocessor.hpp 323 2007-06-13 21:02:57Z byon $
 */

#ifndef MYRRH_UTILITY_PREPROCESSOR_H_INCLUDED
#define MYRRH_UTILITY_PREPROCESSOR_H_INCLUDED

namespace myrrh
{

namespace util
{

/**
 * Tells if the current build is a debug build or not. This function can be
 * used to remove the awkward #ifdef statements from code.
 * @returns true If this is a debug build, false otherwise.
 */
bool IsDebugBuild( );



#ifdef WIN32

// This allows the user to revert the warning levels to normal by calling
// #pragma warning(push) where this header is included
#pragma warning(push)

#ifdef DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#pragma warning(disable: 4127)
#endif

// This disables the 'conversion from 'Type1' to 'Type2', possible loss of
// data' warning that comes plenty of times from boost::date_time
#ifdef DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#pragma warning(disable: 4244 4267)
#endif

#ifdef DISABLE_COPY_CONSTRUCTOR_COULD_NOT_BE_GENERATED
#pragma warning(disable: 4511)
#endif

#ifdef DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#pragma warning(disable: 4512)
#endif

#ifdef DISABLE_UNINITIALIZED_LOCAL_VARIABLE
#pragma warning(disable: 4701)
#endif

#ifdef DISABLE_UNARY_MINUS_TO_UNSIGNED_TYPE
#pragma warning(disable: 4146)
#endif

#ifdef DISABLE_USED_WITHOUT_INITIALIZED
#pragma warning(disable: 4701)
#endif

#ifdef DISABLE_SIGNED_UNSIGNED_MISMATCH
#pragma warning(disable: 4245 4389)
#endif

#ifdef DISABLE_DEPRECATED_FUNCTION_WARNING
#pragma warning(disable: 4996)
#endif

#endif

// Inline implementations

inline bool IsDebugBuild( )
{
#ifdef NDEBUG
    return false;
#else
    return true;
#endif
}

}

}

#endif
