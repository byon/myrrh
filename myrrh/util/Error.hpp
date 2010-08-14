// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * @file myrrh/utility/Error.hpp
 * @author Marko Raatikainen
 *
 * This file contains utility function(s) to handle error situations.
 *
 * $Id: Error.hpp 314 2007-05-06 14:33:12Z Byon $
 */

#ifndef MYRRH_UTILITY_ERROR_HPP_INCLUDED
#define MYRRH_UTILITY_ERROR_HPP_INCLUDED

#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>

namespace myrrh
{

namespace util
{

/**
 * The myrrh::util::CatchExceptions function's default functionality depends
 * of the build target. If the build is for debug target, the function will
 * throw again catched exceptions so that they are easier to notice. On release
 * builds the exceptions are by default just reported to error output and
 * then ignored. Note that the default functionality can be replaced by
 * a function parameter.
 */
#ifdef NDEBUG
    const bool DEFAULT_RETHROW = false;
#else
    const bool DEFAULT_RETHROW = true;
#endif

/**
 * A helper function, which calls the given function and catches all exceptions
 * writing the error into given stream. Then it either rethrows or returns a
 * standard return value for error depending on value of rethrow parameter.
 * If the thrown excpetion is derived from std::exception, additional
 * information will be written (exception name and string returned by what( )).
 * @param function Function to be called
 * @param argument Argument to be passed to function
 * @param os Output target, defaults to std::cerr
 * @param rethrow If true, all exceptions will be rethrown.
 * @see DEFAULT_RETHROW for default value of rethrow parameter.
 * @throws Any exception that can be thrown by given function (if rethrow is
 *         true) or by ostream output.
 * @return EXIT_SUCCESS if all is ok.
 *         EXIT_FAILURE if exception is caught and rethrow is false.
 */
template <typename Func, typename Arg, typename Stream>
int CatchExceptions(Func function, const Arg &argument, Stream &os,
                    bool rethrow = DEFAULT_RETHROW);


// Inline implementations

template <typename Func, typename Arg, typename Stream>
inline int CatchExceptions<Func, Arg, Stream>(Func function,
                                              const Arg &argument,
                                              Stream &os, bool rethrow)
{
    const static std::string ERROR_START_TXT("catchException: ");
    try
    {
        // If the function throws no exceptions, we're successfull
        function(argument);
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        os << ERROR_START_TXT << '\'' << typeid(e).name( ) << "': '"
           << e.what( ) << "'" << std::endl;

        if (rethrow)
        {
            throw;
        }
    }
    catch (...)
    {
        os << ERROR_START_TXT <<  "unknown exception" << std::endl;
        if (rethrow)
        {
            throw;
        }
    }

    // Some exception was thrown, so we'll return an error
    return EXIT_FAILURE;
}

}

}

#endif
