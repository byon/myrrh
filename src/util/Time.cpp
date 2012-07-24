// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * File: myrrh/utility/Timer.cpp
 * Author: Marko Raatikainen
 *
 * This file contains timer related helper function implementation(s)
 *
 * $Id: Time.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/util/Time.hpp"
#include <cstdio>
#pragma warning(disable : 4996)

namespace myrrh
{

namespace util
{

const char * const TimeStampAsCString( )
{
    time_t currentTime;
    unsigned int milliSeconds;

    TimeStamp(currentTime, milliSeconds);
    return TimeStampToCString(currentTime, milliSeconds);
}

const char * const TimeStampToCString(time_t seconds, unsigned milliSeconds)
{
    static char header[32] = {0};

    struct tm *nowStamp = localtime(&seconds);
    sprintf(header, "%.4d.%.2d.%.2d %.2d:%.2d:%.2d:%.3d",
            nowStamp->tm_year + 1900, nowStamp->tm_mon +1,
            nowStamp->tm_mday, nowStamp->tm_hour,
            nowStamp->tm_min, nowStamp->tm_sec, milliSeconds);
    return header;
}

}

}
