// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Files
 *
 * $Id: TestFiles.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/data/test/Files.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#define DISABLE_SIGNED_UNSIGNED_MISMATCH
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <fstream>

typedef boost::unit_test::test_suite TestSuite;

// Test case function declarations
void TestFilesWithContent( );
void TestNonExistingFile( );
void TestEmptyFile( );
void TestFileIndices( );

// Helper function declarations
void TestFileWithContent(const std::string &name);
const std::string Path(const std::string &name);
void CheckIsOpen(const std::ifstream &stream, const std::string &name);
std::string GetFileContent(std::ifstream &stream);
void TestFileIndex(size_t index, const std::string &name);

using namespace myrrh::data::test;

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite *test(BOOST_TEST_SUITE("Test suite for Files"));
    test->add(BOOST_TEST_CASE(TestNonExistingFile));
    test->add(BOOST_TEST_CASE(TestEmptyFile));
    test->add(BOOST_TEST_CASE(TestFilesWithContent));
    test->add(BOOST_TEST_CASE(TestFileIndices));

    return test;
}

void TestNonExistingFile( )
{
    const std::string NAME(Path(Files::NOT_EXISTING));
    std::ifstream stream(NAME.c_str( ));
    BOOST_REQUIRE(!stream.is_open( ));
}

void TestEmptyFile( )
{
    const std::string NAME(Path(Files::EMPTY));
    std::ifstream stream(NAME.c_str( ));
    CheckIsOpen(stream, NAME);

    BOOST_CHECK_EQUAL(GetFileContent(stream).size( ), 0);
}

void TestFilesWithContent( )
{
    TestFileWithContent(Path(Files::ONE_CHAR));
    TestFileWithContent(Path(Files::ONE_LINE));
    TestFileWithContent(Path(Files::ONE_LONG_LINE));
    TestFileWithContent(Path(Files::SEVERAL_LINES));
    TestFileWithContent(Path(Files::SEVERAL_LINES_NOT_EQUAL_LENGTH));
}

void TestFileIndices( )
{
    TestFileIndex(Files::Index::NOT_EXISTING, Files::NOT_EXISTING);
    TestFileIndex(Files::Index::EMPTY, Files::EMPTY);
    TestFileIndex(Files::Index::ONE_CHAR, Files::ONE_CHAR);
    TestFileIndex(Files::Index::ONE_LINE, Files::ONE_LINE);
    TestFileIndex(Files::Index::ONE_LONG_LINE, Files::ONE_LONG_LINE);
    TestFileIndex(Files::Index::SEVERAL_LINES, Files::SEVERAL_LINES);
    TestFileIndex(Files::Index::SEVERAL_LINES_NOT_EQUAL_LENGTH,
                  Files::SEVERAL_LINES_NOT_EQUAL_LENGTH);
    TestFileIndex(Files::Index::LARGE_FILE, Files::LARGE_FILE);
}

void TestFileIndex(size_t index, const std::string &name)
{
    BOOST_CHECK_EQUAL(Files::GetAll( )[index], name);
}

void TestFileWithContent(const std::string &name)
{
    std::ifstream stream(name.c_str( ));
    CheckIsOpen(stream, name);

    BOOST_CHECK(GetFileContent(stream).size( ) > 0);
}

inline const std::string Path(const std::string &name)
{
    static const std::string START("../../../..");
    return START + name;
}

inline void CheckIsOpen(const std::ifstream &stream, const std::string &name)
{
    if (!stream.is_open( ))
    {
        BOOST_ERROR("Cannot open " + name);
    }
}

std::string GetFileContent(std::ifstream &stream)
{
    std::ostringstream stringStream;
    stringStream << stream.rdbuf( );

    return stringStream.str( );
}
