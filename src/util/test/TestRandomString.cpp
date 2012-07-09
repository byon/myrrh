// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for RandomString
 *
 * $Id: TestRandomString.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/util/RandomString.hpp"

#define BOOST_TEST_MODULE TestRandomString
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <vector>

inline void CheckChar(char toCheck)
{
    BOOST_CHECK(toCheck >= 'A');
    BOOST_CHECK(toCheck <= 'z');
}

inline void CheckString(const std::string &toCheck)
{
    std::for_each(toCheck.begin( ), toCheck.end( ), CheckChar);
}

class TestForASize
{
public:

    void operator( )( )
    {
        for (int i = 0; i < MAX_TIMES_TO_TEST_A_LENGTH; ++i)
        {
            std::string lastString(toBeReplaced_);
            GetString(toBeReplaced_);

            BOOST_CHECK_EQUAL(lastString.size( ), toBeReplaced_.size( ));
            CheckString(toBeReplaced_);

            if (toBeReplaced_.size( ) >= MIN_SIZE_TO_CHECK_DIFFERENCE)
            {
                // Even though it is possible in theory to have two alike
                // strings here, I'm quite sure the possibility is too small.
                BOOST_CHECK(lastString != toBeReplaced_);
            }
        }

        TestsDone(toBeReplaced_);
    }

protected:

    virtual void GetString(std::string &toBeReplaced) = 0;

    virtual inline void TestsDone(std::string &)
    {
    }

private:

    std::string toBeReplaced_;
    static const int MAX_TIMES_TO_TEST_A_LENGTH;
    static const std::string::size_type MIN_SIZE_TO_CHECK_DIFFERENCE;
};

const int TestForASize::MAX_TIMES_TO_TEST_A_LENGTH = 10;
const std::string::size_type TestForASize::MIN_SIZE_TO_CHECK_DIFFERENCE = 5;

class TestReplaceWithRandomImpl : public TestForASize
{
protected:

    virtual inline void GetString(std::string &toBeReplaced)
    {
        myrrh::util::ReplaceWithRandom(toBeReplaced);
    }

    virtual inline void TestsDone(std::string &toBeReplaced)
    {
        toBeReplaced += myrrh::util::RandChar( );
    }
};

class TestGetRandomStringImpl : public TestForASize
{
protected:

    virtual inline void GetString(std::string &toBeReplaced)
    {
        toBeReplaced = myrrh::util::GetRandomString(toBeReplaced.size( ));
    }

    virtual inline void TestsDone(std::string &toBeReplaced)
    {
        toBeReplaced += myrrh::util::RandChar( );
    }
};

template <typename Func>
void Test(Func func, int start, int end)
{
    for (int size = start; size < end; ++size)
    {
        func( );
    }
}

BOOST_AUTO_TEST_SUITE(TestRandomString)

BOOST_AUTO_TEST_CASE(TestRandChar)
{
    static const int RANGE_SIZE = 'z' - 'A';

    typedef std::vector<char> Chars;
    Chars notArrived(RANGE_SIZE);

    for (int i = 0; i < RANGE_SIZE; ++i)
    {
        notArrived[i] = 'A' + static_cast<char>(i);
    }

    while (!notArrived.empty( ))
    {
        char current = myrrh::util::RandChar( );

        CheckChar(current);

        Chars::iterator place =
            std::find(notArrived.begin( ), notArrived.end( ), current);
        if (place != notArrived.end( ))
        {
            notArrived.erase(place);
        }
    }
}

BOOST_AUTO_TEST_CASE(TestReplaceWithRandom)
{
    Test(TestReplaceWithRandomImpl( ), 0, 100);
}

BOOST_AUTO_TEST_CASE(TestGetRandomString)
{
    Test(TestGetRandomStringImpl( ), -10, 100);
}

}
