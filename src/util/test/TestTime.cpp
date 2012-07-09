// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Time
 *
 * $Id: TestTime.cpp 368 2007-09-20 19:08:31Z byon $
 */

#include "myrrh/util/Time.hpp"

#define BOOST_TEST_MODULE TestTime
#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_UNARY_MINUS_TO_UNSIGNED_TYPE
#define DISABLE_COPY_CONSTRUCTOR_COULD_NOT_BE_GENERATED
#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"
#include "boost/thread.hpp"
#include "boost/timer.hpp"
#include "boost/regex.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <cmath>
#include <ctime>
#include <sstream>

typedef boost::unit_test::test_suite TestSuite;

namespace
{

void BoostSleep(int maxSeconds, int maxNSeconds)
{
    boost::xtime toWait;
    boost::xtime_get(&toWait, boost::TIME_UTC_);
    toWait.sec += maxSeconds;
    toWait.nsec += maxNSeconds;
    boost::thread::sleep(toWait);
}

class TestLoop
{
protected:

    struct Stamp
    {
    public:

        Stamp(std::time_t seconds = 0, unsigned int milliSeconds = 0) :
            seconds_(seconds),
            milliSeconds_(milliSeconds)
        {
        }

        Stamp(const Stamp &orig) :
            seconds_(orig.seconds_),
            milliSeconds_(orig.milliSeconds_)
        {
        }

        Stamp &operator+(double elapsed)
        {
            double secondFractionsToAdd = 0.0;
            double secondsToAdd = std::modf(elapsed, &secondFractionsToAdd);
            seconds_ += static_cast<std::time_t>(secondsToAdd);
            double milliSecondsToAdd =
                secondFractionsToAdd * MILLISECONDS_IN_SECOND_;
            milliSeconds_ += static_cast<int>(milliSecondsToAdd);

            return *this;
        }

        std::time_t seconds_;
        unsigned int milliSeconds_;
        static const int MILLISECONDS_IN_SECOND_;
    };

public:

    TestLoop(const Stamp stamp, int maxSecondsToSleep = 0,
             int maxMilliSecondsToSleep = 0) :
        lastStamp_(stamp),
        MAX_SECONDS_TO_SLEEP(maxSecondsToSleep),
        MAX_NANO_SECONDS_TO_SLEEP(maxMilliSecondsToSleep)
    {
        std::time_t now = std::time(0);

        BOOST_CHECK_EQUAL(now, lastStamp_.seconds_);
    }

    void Loop(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            BoostSleep(MAX_SECONDS_TO_SLEEP, MAX_NANO_SECONDS_TO_SLEEP);

            Stamp expectedTime(lastStamp_ + timer_.elapsed( ));
            CheckTime(expectedTime);
        }
    }

protected:

    virtual Stamp GetTimeStamp( ) = 0;

private:

    TestLoop &operator=(const TestLoop &);
    TestLoop(const TestLoop &);

    void CheckTime(const Stamp &expected)
    {
        Stamp now(GetTimeStamp( ));

        /// @todo This checking is basically crap. First, you should not
        /// have randomness in unit tests. Second, you should not sleep.
        /// Third, the tests are likely to fail randomly, not only because
        /// both sleeping and time measurements are not accurate operations,
        /// but also, because the timestamps can be on different seconds.
        /// Commented out for now, will need to check whether this test or the
        /// module makes any sense.
        //BOOST_CHECK_EQUAL(now.seconds_, expected.seconds_);
        //BOOST_CHECK_CLOSE(now.milliSeconds_, expected.milliSeconds_, 0.2);
    }

    boost::timer timer_;
    Stamp lastStamp_;
    const int MAX_SECONDS_TO_SLEEP;
    const int MAX_NANO_SECONDS_TO_SLEEP;
};

const int TestLoop::Stamp::MILLISECONDS_IN_SECOND_ = 1000;

class TimeTestLoop : public TestLoop
{
public:

    TimeTestLoop( ) :
        TestLoop(GetTimeStampImpl( ))
    {
    }

protected:

    virtual Stamp GetTimeStamp( )
    {
        return GetTimeStampImpl( );
    }

private:

    Stamp GetTimeStampImpl( )
    {
        Stamp stamp;
        myrrh::util::TimeStamp(stamp.seconds_, stamp.milliSeconds_);

        return stamp;
    }
};

class TestTimeToCString : public TestLoop
{
public:

    TestTimeToCString( ) :
        TestLoop(GetTimeStampImpl( ))
    {
    }

protected:

    virtual Stamp GetTimeStamp( )
    {
        return GetTimeStampImpl( );
    }

private:

    Stamp GetTimeStampImpl( )
    {
        const char * const timeString = myrrh::util::TimeStampToCString( );
        BOOST_CHECK_EQUAL(size_t(23), strlen(timeString));

        // Format of the string is YYYY.MM.DD HH:mm:SS:sss
        const boost::regex EXPRESSION(
            "\\d{4}\\.\\d{2}\\.\\d{2} (\\d{2}:){3}\\d{3}");
        BOOST_CHECK(boost::regex_match(timeString, EXPRESSION));

        std::istringstream reader((std::string(timeString)));

        // Sets the stream not to skip whitespace
        reader >> std::noskipws;

        tm timeStamp = {0};

        // Replace the original values by the ones read from the string
        timeStamp.tm_year = GetNextNumber(reader, '.');
        timeStamp.tm_mon = GetNextNumber(reader, '.');
        timeStamp.tm_mday = GetNextNumber(reader, ' ');
        timeStamp.tm_hour = GetNextNumber(reader, ':');
        timeStamp.tm_min = GetNextNumber(reader, ':');
        timeStamp.tm_sec = GetNextNumber(reader, ':');
        int milliSeconds = GetNextNumber(reader, 0);

        BOOST_CHECK(timeStamp.tm_year > 0);
        BOOST_CHECK(timeStamp.tm_mon > 0 && timeStamp.tm_mon < 13);
        BOOST_CHECK(timeStamp.tm_mday > 0 && timeStamp.tm_mday < 32);
        BOOST_CHECK(timeStamp.tm_hour >= 0 && timeStamp.tm_hour < 25);
        BOOST_CHECK(timeStamp.tm_min >= 0 && timeStamp.tm_min < 60);
        BOOST_CHECK(timeStamp.tm_sec >= 0 && timeStamp.tm_sec < 60);
        BOOST_CHECK(milliSeconds >= 0 && milliSeconds < 1000);

        // The checking if the time values are correct, is done in parent class

        // The following changes need to be done for the std::mktime to
        // succeed, because std::tm's base year is 1900 and base month is 0
        timeStamp.tm_year -= 1900;
        --timeStamp.tm_mon;

        // By setting this field to negative, we signal to mktime that we want
        // it to figure out if it DST is in effect or not.
        timeStamp.tm_isdst = -1;

        std::time_t now = std::mktime(&timeStamp);
        BOOST_REQUIRE(now >= 0);

        return Stamp(now, milliSeconds);
    }

    int GetNextNumber(std::istringstream &stream, char expectedCharacter)
    {
        int value = 0;
        stream >> value;

        if (expectedCharacter)
        {
            char separator = 0;
            stream >> separator;
            BOOST_CHECK_EQUAL(separator, expectedCharacter);
        }

        return value;
    }
};

}

BOOST_AUTO_TEST_SUITE(TestTime)

BOOST_AUTO_TEST_CASE(TestTimeStamp)
{
    TimeTestLoop( ).Loop(10);
}

BOOST_AUTO_TEST_CASE(TestTimeStampToCString)
{
    TestTimeToCString( ).Loop(10);
}

BOOST_AUTO_TEST_SUITE_END( )
