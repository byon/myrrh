// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * Contains tests for myrrh::util::ProgressTimer
 *
 * $Id: TestProgressTimer.cpp 358 2007-09-19 16:46:33Z byon $
 */

#include "myrrh/util/ProgressTimer.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"
#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"
#include "boost/scoped_array.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/xtime.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <cstdlib>
#include <string>
#include <sstream>

namespace
{

void SleepRandomTime(int maxSeconds, int maxNSeconds)
{
    /// @todo Remove this. Sleeping and randomness in unit test. Not good.
    boost::xtime toWait;
    boost::xtime_get(&toWait, boost::TIME_UTC);
    toWait.sec += (std::rand( ) % maxSeconds);
    toWait.nsec += (std::rand( ) % maxNSeconds);
    boost::thread::sleep(toWait);
}

void CheckTimerOutput(std::istringstream &input, const std::string &id,
                      double elapsedTime)
{
    const size_t ID_SIZE = id.size( );
    boost::scoped_array<char> buffer(new char[ID_SIZE +1]);
    input.read(buffer.get( ), static_cast<std::streamsize>(ID_SIZE));
    buffer[ID_SIZE] = 0;

    std::string readId(buffer.get( ));
    BOOST_CHECK_EQUAL(id, readId);

    char colon;
    double spentTime = 0.0;
    std::string secondId;

    input >> colon >> spentTime >> secondId;

    BOOST_CHECK_EQUAL(colon, ':');
    BOOST_CHECK_CLOSE(elapsedTime, spentTime, 0.001);
    BOOST_CHECK_EQUAL(secondId, "s");
}

void DoTestProgressTimer(const std::string id)
{
    std::ostringstream stream;

    boost::timer timer;

    {
        myrrh::util::ProgressTimer timer(id, stream);

        // Spend some random time
        SleepRandomTime(2, 100000000);
    }

    double elapsedTime = timer.elapsed( );

    const std::string OUTPUT(stream.str( ));
    BOOST_CHECK(OUTPUT.length( ));

    std::istringstream input(OUTPUT);
    CheckTimerOutput(input, id, elapsedTime);
}

}

BOOST_AUTO_TEST_SUITE(TestProgressTimer)

BOOST_AUTO_TEST_CASE(TestProgressTimer)
{
    DoTestProgressTimer("Test1");
    DoTestProgressTimer("");
    DoTestProgressTimer("Test3 with some extraordinary and uncommonly long "
                        "identifier, which goes on and on without stopping "
                        "even if there were some dramatically dressed lady in "
                        "red shouting in a wavering but emotional voice to "
                        "stop, while theatre workers are moving the sceneries "
                        "and all such theatre things around in the backgound. "
                        "No, the identifier is an ever-lasting entity that "
                        "will never ever st...");
}

BOOST_AUTO_TEST_SUITE_END( )

namespace
{

class RandomSleep
{
public:

    RandomSleep(int maxSeconds, int maxNSeconds, bool willThrow) :
        mMaxSeconds(maxSeconds),
        mMaxNSeconds(maxNSeconds),
        mWillThrow(willThrow)
    {
    }

    RandomSleep(const RandomSleep &orig) :
        mMaxSeconds(orig.mMaxSeconds),
        mMaxNSeconds(orig.mMaxNSeconds),
        mWillThrow(orig.mWillThrow)
    {
    }

    void operator( )( )
    {
        SleepRandomTime(mMaxSeconds, mMaxNSeconds);

        if (mWillThrow)
        {
            throw std::runtime_error("Expected exception from RandomSleep");
        }
    }

private:

    RandomSleep &operator=(const RandomSleep &);

    const int mMaxSeconds;
    const int mMaxNSeconds;
    bool mWillThrow;
};

template <typename Func>
void DoTestTimePerformance(const std::string &id, int count, Func function,
                           bool expectedResult)
{
    using namespace myrrh::util;

    std::ostringstream stream;

    boost::timer timer;

    bool result = TimePerformance(id, count, function, stream);
    double elapsedTime = timer.elapsed( );

    BOOST_CHECK_EQUAL(result, expectedResult);

    const std::string OUTPUT(stream.str( ));
    BOOST_CHECK(OUTPUT.length( ));

    std::istringstream input(OUTPUT);

    if (expectedResult)
    {
        CheckTimerOutput(input, id, elapsedTime);
    }
    else
    {
        const size_t MAX_SIZE = 1024;
        boost::scoped_array<char> buffer(new char[MAX_SIZE]);
        memset(buffer.get( ), 0, MAX_SIZE);
        input.getline(buffer.get( ), MAX_SIZE);

        const std::string ERROR_LINE(buffer.get( ));
        const std::string EXPECTED_LINE(
            "catchException: 'class std::runtime_error': 'Expected exception "
            "from RandomSleep'");
        BOOST_CHECK_EQUAL(ERROR_LINE, EXPECTED_LINE);
    }
}

void TestTimePerformance( )
{
    RandomSleep randomSleep(1, 100000000, false);
    DoTestTimePerformance("Test1", 1234, randomSleep, true);
    DoTestTimePerformance("Test 2", 4321, randomSleep, true);

    RandomSleep failingSleep(1, 100000000, true);
    DoTestTimePerformance("Failing sleep", 100000, failingSleep, false);
}

}
