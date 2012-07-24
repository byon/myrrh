// Copyright 2007-2012 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "myrrh/util/Time.hpp"

#define BOOST_TEST_MODULE TestTime
#include "boost/test/unit_test.hpp"

BOOST_AUTO_TEST_SUITE(TestTime)

BOOST_AUTO_TEST_CASE(TimeStampFillsSeconds)
{
    time_t seconds = 0;
    unsigned unused = 0;
    myrrh::util::TimeStamp(seconds, unused);
    BOOST_CHECK(0 != seconds);
}

BOOST_AUTO_TEST_CASE(TimeStampFillsMilliseconds)
{
    time_t unused = 0;
    unsigned milliSeconds = 1000;
    myrrh::util::TimeStamp(unused, milliSeconds);
    BOOST_CHECK(0 != 1000);
}

BOOST_AUTO_TEST_SUITE_END( )
