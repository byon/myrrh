// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Stream
 *
 * $Id: TestStream.cpp 369 2007-09-20 20:08:51Z byon $
 */

#include "myrrh/log/policy/Stream.hpp"
#include "myrrh/log/policy/Restriction.hpp"
#include "myrrh/log/policy/Creator.hpp"
#include "myrrh/log/policy/Path.hpp"
#include "myrrh/log/policy/PathPart.hpp"

#include "myrrh/file/Eraser.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/filesystem/path.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

typedef boost::unit_test::test_suite TestSuite;

using namespace myrrh::log::policy;

// Test case function declarations
void TestStream( );

// Helper function declarations
std::string GetFileContent(const std::string &path);

// Use automatic test initialization
TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for Stream");
    test->add(BOOST_TEST_CASE(TestStream));
    return test;
}

// Just one test? And even that is commented out...
void TestStream( )
{
    const std::string FILE_NAME("myrrh.log");
    myrrh::file::Eraser eraser(FILE_NAME);

    Path path;
    path += FILE_NAME;
    InitialOpenerPtr opener(new Creator);
    PolicyPtr policy(new Policy(path, opener, opener));
    Stream stream(policy);

    stream << "A string followed by end of line" << std::endl;
    stream << 1234 << " string after an integer and before a float " << 123.12;
    stream << ". Yet another string.\n";
    stream.flush( );

    /** @todo Find out why the file is not there
    BOOST_CHECK_EQUAL(GetFileContent(FILE_NAME),
                      "A string followed by end of line\n"
                      "1234 string after an integer and before a float 123.12"
                      ". Yet another string.\n");
    */
}

std::string GetFileContent(const std::string &path)
{
    std::ifstream file(path.c_str( ));
    BOOST_REQUIRE(file.is_open( ));

    std::ostringstream stream;
    stream << file.rdbuf( );

    return stream.str( );
}
