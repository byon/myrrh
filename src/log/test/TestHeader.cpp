// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for myrrh::log::TimestampHeader and
 * myrrh::log::Header.
 *
 * The following tests are made:
 * -Given stream is not open.
 * -Given verbosity id gets written to output
 * -A valid timestamp gets written to output
 * -Same header object can write to different streams
 *
 * The following tests are not made, because the functionality is already
 * tested elsewhere:
 * -Checking if the timestamp shows correct time
 *
 * $Id: TestHeader.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/log/Header.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/regex.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

typedef boost::unit_test::test_suite TestSuite;

using namespace myrrh::log;
using namespace myrrh::util;

// Declarations of test case functions
void WriteOneLine( );
void WriteSeveralLines( );
void UseSameHeaderForDifferentStreams( );

class TestCase
{
public:

    virtual ~TestCase( );

    virtual void operator( )( );

protected:

    virtual void DoTest(Header &header, std::ostream &stream);
    virtual HeaderPtr GetHeader( );
    virtual char GetVerbosityId( );
    virtual bool DoesMatchExpected(const std::string &result);
};

// Declarations of helper functions

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for myrrh::log::Header");

    test->add(BOOST_TEST_CASE(WriteOneLine));
    test->add(BOOST_TEST_CASE(WriteSeveralLines));
    test->add(BOOST_TEST_CASE(UseSameHeaderForDifferentStreams));

    return test;
}

TestCase::~TestCase( )
{
}

void TestCase::operator( )( )
{
    HeaderPtr header(GetHeader( ));

    std::ostringstream stream;

    DoTest(*header, stream);

    DoesMatchExpected(stream.str( ));
}

void TestCase::DoTest(Header &header, std::ostream &stream)
{
    header.Write(stream, GetVerbosityId( ));
}

HeaderPtr TestCase::GetHeader( )
{
    return HeaderPtr(new TimestampHeader);
}

char TestCase::GetVerbosityId( )
{
    return 'G';
}

bool TestCase::DoesMatchExpected(const std::string &result)
{
    const std::string ID(1, GetVerbosityId( ));
    boost::regex expression("^\\d{4}\\.\\d{2}\\.\\d{2} " // date
                            "\\d{2}:\\d{2}:\\d{2}.\\d{6} " // time
                            + ID + " $"); // verbosity id

    if (!boost::regex_match(result, expression))
    {
        BOOST_ERROR("Result '" + result + "' not expected with expression " +
                    expression.str( ));
        return false;
    }

    return true;
}

void WriteOneLine( )
{
    TestCase( )( );
}

void WriteSeveralLines( )
{
    class Case : public TestCase
    {
        void DoTest(Header &header, std::ostream &stream)
        {
            TestCase::DoTest(header, stream);
            TestCase::DoTest(header, stream);
        }

        bool DoesMatchExpected(const std::string &result)
        {
            const std::size_t HALF_WAY = result.size( ) / 2;
            const std::string FIRST_HALF(result.substr(0, HALF_WAY));
            const std::string SECOND_HALF(result.substr(HALF_WAY));

            return TestCase::DoesMatchExpected(FIRST_HALF) &&
                   TestCase::DoesMatchExpected(SECOND_HALF);
        }
    };

    Case( )( );
}

void UseSameHeaderForDifferentStreams( )
{
    TimestampHeader header;

    std::ostringstream first;
    header.Write(first, '-');

    std::ostringstream second;
    header.Write(second, '-');

    const std::string EXAMPLE("1234.12.12 12:12:12:123456 - ");

    typedef unsigned int uint;
    const uint EXPECTED_SIZE = static_cast<uint>(EXAMPLE.size( ));
    const uint FIRST_SIZE = static_cast<uint>(first.str( ).size( ));
    const uint SECOND_SIZE = static_cast<uint>(second.str( ).size( ));

    BOOST_CHECK_EQUAL(FIRST_SIZE, EXPECTED_SIZE);
    BOOST_CHECK_EQUAL(SECOND_SIZE, EXPECTED_SIZE);
}
