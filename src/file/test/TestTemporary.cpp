// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Temporary
 *
 * $Id: TestTemporary.cpp 356 2007-09-18 19:55:21Z byon $
 */

#include "myrrh/file/Temporary.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/filesystem/exception.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <fstream>
#include <string>

typedef boost::unit_test::test_suite TestSuite;

const std::string FILE_NAME("file.txt");
const std::string DIRECTORY_NAME("TestDirectory");
const std::string OUTPUT("Just some output to test");

// utility function declarations
template <typename Error>
void CatchConstructError(const boost::filesystem::path &path,
                         const std::string &error);
void GenerateFile(const boost::filesystem::path &path,
                  const std::string &content);
void CheckFileContent(const boost::filesystem::path &path,
                      const std::string &expectedContent);
void UseTemporary(const boost::filesystem::path &path,
                  const std::string &content, bool doThrow = false);

/**
 * Class used for intentional exceptions
 */
class TestError
{
};

/**
 * Class used for ensuring that test files are deleted.
 */
class FileCleanup
{
public:

    FileCleanup( )
    {
        Remove( );
    }

    ~FileCleanup( )
    {
        Remove( );
    }

private:

    void Remove( )
    {
        boost::filesystem::remove(FILE_NAME);
        boost::filesystem::remove(DIRECTORY_NAME);
    }
};

void EmptyPath( )
{
    boost::filesystem::path path;
    CatchConstructError<myrrh::file::Temporary::PathError>(
        path, "Empty path should cause exception");
}

void UsingDirectoryPath( )
{
    FileCleanup cleaner;
    BOOST_REQUIRE(boost::filesystem::create_directory(DIRECTORY_NAME));

    CatchConstructError<myrrh::file::Temporary::PathError>(
        DIRECTORY_NAME, "Directory path should cause exception");
}

void PathToAlreadyExistingFile( )
{
    FileCleanup cleaner;
    GenerateFile(FILE_NAME, OUTPUT);

    UseTemporary(FILE_NAME, OUTPUT + OUTPUT);

    BOOST_CHECK(!boost::filesystem::exists(FILE_NAME));
}

void PathToAlreadyOpenFile( )
{
    FileCleanup cleaner;
    GenerateFile(FILE_NAME, OUTPUT);
    std::ifstream file(FILE_NAME.c_str( ));
    BOOST_REQUIRE(file.is_open( ));

    UseTemporary(FILE_NAME, OUTPUT + OUTPUT);

    // Cannot delete the file, if it is still open
    BOOST_CHECK(boost::filesystem::exists(FILE_NAME));
}

void NoOutput( )
{
    FileCleanup cleaner;
    {
        myrrh::file::Temporary tmp(FILE_NAME);
    }
    BOOST_CHECK(!boost::filesystem::exists(FILE_NAME));
}

void ExceptionAfterConstruction( )
{
    FileCleanup cleaner;

    try
    {
        myrrh::file::Temporary tmp(FILE_NAME);
        throw TestError( );
    }
    catch (const TestError &)
    {
    }

    BOOST_CHECK(!boost::filesystem::exists(FILE_NAME));
}

void PrintingToTemporary( )
{
    FileCleanup cleaner;

    UseTemporary(FILE_NAME, OUTPUT + OUTPUT);

    BOOST_CHECK(!boost::filesystem::exists(FILE_NAME));
}

void ExceptionAfterOutput( )
{
    FileCleanup cleaner;

    try
    {
        UseTemporary(FILE_NAME, OUTPUT + OUTPUT, true);
    }
    catch (const TestError &)
    {
    }

    BOOST_CHECK(!boost::filesystem::exists(FILE_NAME));
}

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test =
        BOOST_TEST_SUITE("Test suite for myrrh::file::Temporary");

#ifdef NDEBUG
    // The empty path can only be tested in release builds, because it causes
    // an assertion and the test stops there.
    test->add(BOOST_TEST_CASE(EmptyPath));
#endif

    test->add(BOOST_TEST_CASE(UsingDirectoryPath));
    test->add(BOOST_TEST_CASE(PathToAlreadyExistingFile));
    test->add(BOOST_TEST_CASE(PathToAlreadyOpenFile));
    test->add(BOOST_TEST_CASE(NoOutput));
    test->add(BOOST_TEST_CASE(ExceptionAfterConstruction));
    test->add(BOOST_TEST_CASE(PrintingToTemporary));
    test->add(BOOST_TEST_CASE(ExceptionAfterOutput));
    return test;
}

template <typename Error>
void CatchConstructError(const boost::filesystem::path &path,
                         const std::string &error)
{
    try
    {
        myrrh::file::Temporary tmp(path);
        BOOST_ERROR(error);
    }
    catch (const Error &e)
    {
        const std::string REASON(e.what( ));
        BOOST_CHECK(!REASON.empty( ));
        BOOST_CHECK(std::string::npos != REASON.find(path.string( )));
    }
}

void GenerateFile(const boost::filesystem::path &path,
                  const std::string &content)
{
    std::ofstream file(path.string( ).c_str( ));
    BOOST_REQUIRE(file.is_open( ));
    file << content << std::endl;
}

void CheckFileContent(const boost::filesystem::path &path,
                      const std::string &expectedContent)
{
    std::ifstream file(path.string( ).c_str( ));
    BOOST_REQUIRE(file.is_open( ));

    std::ostringstream stream;
    stream << file.rdbuf( );

    BOOST_CHECK_EQUAL(stream.str( ), expectedContent);
}

void UseTemporary(const boost::filesystem::path &path,
                  const std::string &content, bool doThrow)
{
    myrrh::file::Temporary tmp(path);
    tmp.Stream( ) << content << std::endl;
    CheckFileContent(path, content + '\n');

    if (doThrow)
    {
        throw TestError( );
    }
}
