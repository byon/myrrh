// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * File: myrrh/utility/Timer.hpp
 * Author: Marko Raatikainen
 *
 * This file contains declarations of various timer related helper functions
 *
 * $Id: Time.hpp 368 2007-09-20 19:08:31Z byon $
 */

#ifndef MYRRH_UTILITY_TIME_HPP_INCLUDED
#define MYRRH_UTILITY_TIME_HPP_INCLUDED

#include <ostream>
#include <ctime>

#ifdef WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

namespace myrrh
{

namespace util
{

/**
 * Gets the current timestamp in localtime.
 * @param seconds After the function, this parameter will hold the seconds
 *                from epoch.
 * @param milliSeconds After the function, this parameter will hold the
 *                     milliseconds since the time specified by seconds
 *                     parameter.
 * @warning This function is not thread safe
 */
void TimeStamp(time_t &seconds, unsigned int &milliSeconds);


/**
 * Returns a null terminated c string buffer containing a timestamp of current
 * time. The string follows the format "YYYY.MM.DD HH:mm:SS:sss", where Y is
 * year, M is month, D is day of month, H is hour, m is minutes, S is seconds
 * and s is milliseconds. The time is in localtime.
 * @warning This function is not thread safe
 * @return A pointer to null-terminated buffer containing the timestamp. Note
 *         that this pointer is to a static buffer and therefore the value
 *         should not be modified, nor stored. If the value needs storing, copy
 *         the buffer.
 */
const char * const TimeStampToCString( );

// inline implementations

#ifdef WIN32

#pragma warning(push)
#pragma warning(disable : 4996)

inline void TimeStamp(time_t &seconds, unsigned int &milliSeconds)
{
    static _timeb now;
    _ftime(&now);

    seconds = now.time;
    milliSeconds = now.millitm;
}

#pragma warning(pop)

#else

inline void TimeStamp(time_t &seconds, unsigned int &milliSeconds)
{
    static timeval now;
    gettimeofday(&now, 0);

    seconds = now.tv_sec;
    milliSeconds = now.tv_usec / 1000;
}

#endif

}

}

#endif
