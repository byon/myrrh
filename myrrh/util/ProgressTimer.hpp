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
 * A modified version of boost::progress_timer, which takes an additional
 * constructor parameter, id, which will be added to the timing output in
 * destructor.
 * Also contains a helper function for timing performance tests.
 *
 * $Id: ProgressTimer.hpp 358 2007-09-19 16:46:33Z byon $
 */

#ifndef MYRRH_UTILITY_PROGRESS_TIMER_H_INCLUDED
#define MYRRH_UTILITY_PROGRESS_TIMER_H_INCLUDED

#include "myrrh/util/Error.hpp"
#include "myrrh/util/Repeat.hpp"
#include <boost/timer.hpp>
#include <boost/utility.hpp>  // for noncopyable
#include <boost/cstdint.hpp>  // for uintmax_t
#include <iostream>           // for ostream, cout, etc
#include <string>             // for string


namespace myrrh
{

namespace util
{

/**
 * This class is a modified version of boost::progress_timer, which takes
 * an additional constructor parameter, which can be used to identify the
 * output of the class. The purpose of the class is to time how long a certain
 * functionality of a program lasts. When the class is constructed, a timestamp
 * is stored. When the class is destroyed, the passed time will be printed to
 * specified output stream.
 * The current precision of the timing is milliseconds.
 * @note Destructor will silently ignore any possible exceptions thrown in the
 *       process of writing into output.
 */
class ProgressTimer : public boost::timer, private boost::noncopyable
{
public:

    /**
     * Constructor
     * @param id Additional string identifier written to start of output
     * @param os The output stream, which will be used for writing. According
     *           to original boost documentation: "os is hint; implementation
     *           may ignore, particularly in embedded systems"
     */
    explicit ProgressTimer( const std::string id = "",
                            std::ostream & os = std::cout );

    /**
     * Destructor, writes the passed time into output. All exceptions are
     * silently ignored
     */
    ~ProgressTimer( );

private:

    /** The output stream into which we'll write */
    std::ostream & os_;
    /** Identifies the output from output of other timers */
    const std::string id_;
};

/**
 * A helper function for timing performance test of a single function. The
 * given function will be called by given count of times.
 * If any of the function calls throw exception, an error string will be output
 * to the given stream and the test is stopped with returnvalue of false.
 * @param id Identifies the timer, printed to timer output
 * @param count Count of times function will be called
 * @param function Function that is tested
 * @param os Stream where any output will be written, defaults to std::cout
 * @return true if all of the test calls were successfull, false if one caused
 *         an exception.
 */
template <typename Func>
bool TimePerformance(const std::string &id, const size_t count, Func function,
                     std::ostream & os = std::cout);

// Inline implementations

inline ProgressTimer::ProgressTimer(const std::string id, std::ostream &os) :
    os_(os),
        id_(id)
{
}

inline ProgressTimer::~ProgressTimer( )
{
    //  A) Throwing an exception from a destructor is a Bad Thing.
    //  B) The ProgressTimer destructor does output which may throw.
    //  C) A ProgressTimer is usually not critical to the application.
    //  Therefore, wrap the I/O in a try block, catch and ignore all
    // exceptions.
    try
    {
        // use istream instead of ios_base to workaround GNU problem
        // (Greg Chicares)
        std::istream::fmtflags old_flags =
            os_.setf( std::istream::fixed, std::istream::floatfield );
        std::streamsize old_prec = os_.precision( 3 );
        os_ << id_ << ": " << elapsed() << " s\n" << std::endl;
        os_.flags( old_flags );
        os_.precision( old_prec );
    }

    catch (...) {} // eat any exceptions
}

template <typename Func>
inline bool TimePerformance(const std::string &id, const size_t count,
                            Func function, std::ostream &os)
{
    ProgressTimer timer(id, os);

    // We'll introduce a functor object that will do the actual work and then
    // call myrrh::util::CatchExceptions, which already knows how to handle
    // exceptions
    Repeat<Func> repeater(function);
    return (EXIT_SUCCESS ==
            myrrh::util::CatchExceptions(repeater, count, os, false));
}

}

}

#endif
