// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test for myrrh::util::catchExceptions.
 *
 * $Id: TestCatchExceptions.cpp 367 2007-09-20 18:41:36Z byon $
 */

#include "myrrh/util/Error.hpp"

#define BOOST_TEST_MODULE TestCatchExceptions
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"
#include "boost/test/unit_test.hpp"
#include <sstream>

typedef boost::unit_test::test_suite TestSuite;

namespace
{

class Dummy
{
public:

    Dummy(const std::string &text) :
        text_(text)
    {
    }

    Dummy(const Dummy &orig) :
        text_(orig.text_)
    {
    }

private:

    Dummy &operator=(const Dummy &);

    const std::string text_;
};

class TestException : public std::runtime_error
{
public:

    TestException(const std::string &what) :
        std::runtime_error(what)
    {
    }
};

template <typename T, bool DoesThrow>
void Function(const T &)
{
    if (DoesThrow)
    {
        throw TestException("An expected exception");
    }
}

template <typename T>
void ThrowUnknown(const T &)
{
    class UnknownException
    {
    };
    throw UnknownException( );
}

template <int ExpectedValue>
void CheckArgument(int argument)
{
    BOOST_CHECK_EQUAL(argument, ExpectedValue);
}

template <typename Func, typename T>
void TestSuccess(Func function, const T &argument)
{
    std::ostringstream stream;

    using namespace myrrh::util;

    int result = CatchExceptions(function, argument, stream, false);
    BOOST_CHECK_EQUAL(result, EXIT_SUCCESS);

    BOOST_CHECK(stream.str( ).empty( ));
}

template <typename Func, typename T>
void TestErrors(Func function, const T &argument,
                const std::string& expectedOutput)
{
    std::ostringstream stream;

    using namespace myrrh::util;

    int result = CatchExceptions(function, argument, stream, false);
    BOOST_CHECK_EQUAL(result, EXIT_FAILURE);
    BOOST_CHECK_EQUAL(expectedOutput, stream.str( ));

    stream.str("");
    try
    {
        CatchExceptions(function, argument, stream, true);
        BOOST_ERROR("The previous line should have caused an exception");
    }
    catch(...)
    {
    }

    BOOST_CHECK_EQUAL(expectedOutput, stream.str( ));
}

std::string ExceptionName( )
{
    return typeid(TestException).name( );
}

template <typename T>
void Test(const T &argument)
{
    TestSuccess(Function<T, false>, argument);
    TestErrors(Function<T, true>, argument, "catchException: '" +
               ExceptionName( ) + "': 'An expected exception'\n");
    TestErrors(ThrowUnknown<T>, argument,
        "catchException: unknown exception\n");
}

}

BOOST_AUTO_TEST_SUITE(TestCatchExceptions)

BOOST_AUTO_TEST_CASE(TestInt)
{
    Test(0);
    Test(1234);
    Test(std::numeric_limits<int>::max( ));
    Test(std::numeric_limits<int>::min( ));

    TestSuccess(CheckArgument<1234>, 1234);
}

BOOST_AUTO_TEST_CASE(TestString)
{
    Test(std::string("A string"));
    Test("A string");
    Test("");
    Test(std::string(""));
}

BOOST_AUTO_TEST_CASE(TestClass)
{
    Test(Dummy("Just some string"));
}

BOOST_AUTO_TEST_SUITE_END( )
