// Contains modified code (ProgressTimer) published with following license:
//  Copyright Beman Dawes 1994-99.  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
// Other code is licensed as follows:
//   Copyright 2007 Marko Raatikainen.
//   Distributed under the Boost Software License, Version 1.0.

/**
 * File: myrrh/utility/ProgressTimer.hpp
 * Author: Marko Raatikainen
 *
 * Contains helper functor for repeating a call to a functor.
 *
 * $Id: Repeat.hpp 358 2007-09-19 16:46:33Z byon $
 */

#ifndef MYRRH_UTILITY_REPEAT_H_INCLUDED
#define MYRRH_UTILITY_REPEAT_H_INCLUDED

namespace myrrh
{

namespace util
{
/**
 * A helper functor for repeating a function a count of times.
 */
template <typename Func>
class Repeat
{
public:

    /**
     * Constructor
     * @param function The function to repeat
     */
    Repeat(Func function);

    /**
     * The functor operator that does the repeating
     * @param count The count of times to repeat
     */
    void operator ( )(size_t count);

private:

    /** The function/functor to be repeatedly called */
    Func function_;
};

// Inline implementations

template <typename Func>
inline Repeat<Func>::Repeat(Func function) :
    function_(function)
{
}

template <typename Func>
inline void Repeat<Func>::operator ( )(size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        function_( );
    }
}

}

}

#endif
