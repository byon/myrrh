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

#include "myrrh/util/time.hpp"
#pragma warning(disable : 4996)

namespace myrrh
{

namespace util
{

const char * const TimeStampToCString( )
{
    static char header[32] = {0};
    time_t currentTime;
    unsigned int milliSeconds;

    TimeStamp(currentTime, milliSeconds);

    struct tm *nowStamp = localtime(&currentTime);
    sprintf(header, "%.4d.%.2d.%.2d %.2d:%.2d:%.2d:%.3d",
            nowStamp->tm_year + 1900, nowStamp->tm_mon +1,
            nowStamp->tm_mday, nowStamp->tm_hour,
            nowStamp->tm_min, nowStamp->tm_sec, milliSeconds);

    return header;
}

}

}

