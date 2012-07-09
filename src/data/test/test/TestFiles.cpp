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

#define BOOST_TEST_MODULE "Unit test suite for myrrh::data::test"
#include "boost/test/unit_test.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <fstream>

namespace
{

void TestFileWithContent(const std::string &name);
const std::string Path(const std::string &name);
void CheckIsOpen(const std::ifstream &stream, const std::string &name);
std::string GetFileContent(std::ifstream &stream);
void TestFileIndex(size_t index, const std::string &name);

}

using namespace myrrh::data::test;

BOOST_AUTO_TEST_SUITE(DynamicSuite)

BOOST_AUTO_TEST_CASE(TestNonExistingFile)
{
    const std::string NAME(Path(Files::NOT_EXISTING));
    std::ifstream stream(NAME.c_str( ));
    BOOST_REQUIRE(!stream.is_open( ));
}

BOOST_AUTO_TEST_CASE(TestEmptyFile)
{
    const std::string NAME(Path(Files::EMPTY));
    std::ifstream stream(NAME.c_str( ));
    CheckIsOpen(stream, NAME);

    BOOST_CHECK_EQUAL(GetFileContent(stream).size( ), 0);
}

BOOST_AUTO_TEST_CASE(TestFilesWithContent)
{
    TestFileWithContent(Path(Files::ONE_CHAR));
    TestFileWithContent(Path(Files::ONE_LINE));
    TestFileWithContent(Path(Files::ONE_LONG_LINE));
    TestFileWithContent(Path(Files::SEVERAL_LINES));
    TestFileWithContent(Path(Files::SEVERAL_LINES_NOT_EQUAL_LENGTH));
}

BOOST_AUTO_TEST_CASE(TestFileIndices)
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

BOOST_AUTO_TEST_SUITE_END( )

namespace
{
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
    static const std::string DIRECTORY(".");
    return DIRECTORY + name;
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

}
