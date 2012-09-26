// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Eraser.
 *
 * The following situations are tested:
 * -Giving path to file in current folder
 * -Giving relative path to file in a subfolder
 * -Giving absolute path to file
 * -Giving path to a directory
 * -No call to Release
 * -Exception before Release
 *
 * The following situations are not tested, because they are considered user
 * errors:
 * -Giving empty path
 *
 * $Id: TestEraser.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/file/Eraser.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/test/unit_test.hpp"

#include <fstream>

#ifdef WIN32
#pragma warning(pop)
#endif

typedef boost::unit_test::test_suite TestSuite;

// Test case function declarations
void PathToFileInCurrentFolder( );
void RelativePathToAFileInSubfolder( );
void AbsolutePathToAFile( );
void PathToAnEmptyDirectory( );
void PathToADirectory( );

class TestCase
{
public:

    TestCase(const boost::filesystem::path &path);

    virtual ~TestCase( );

    void operator( )( );

protected:

    virtual void TestSuccessfullHandling( );
    virtual void TestForgottenErase( );
    virtual void TestExceptionThrown( );
    virtual void PutOutputToFile(const boost::filesystem::path &path);
    const boost::filesystem::path &GetPath( ) const;

private:

    TestCase(const TestCase &);
    TestCase &operator=(const TestCase &);

    const boost::filesystem::path PATH_;
};

class DirectoryTestCase : public TestCase
{
public:

    DirectoryTestCase(const boost::filesystem::path &path);

protected:

    virtual void TestSuccessfullHandling( );
    virtual void TestForgottenErase( );
    virtual void TestExceptionThrown( );
    virtual void PutOutputToFile(const boost::filesystem::path &path);
};

class TestException : public std::runtime_error
{
public:

    TestException( ) :
        std::runtime_error("Simulated exception to test exception handling")
    {
    }
};

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for Eraser");
    test->add(BOOST_TEST_CASE(PathToFileInCurrentFolder));
    test->add(BOOST_TEST_CASE(RelativePathToAFileInSubfolder));
    test->add(BOOST_TEST_CASE(AbsolutePathToAFile));
    test->add(BOOST_TEST_CASE(PathToAnEmptyDirectory));
    test->add(BOOST_TEST_CASE(PathToADirectory));
    return test;
}

TestCase::TestCase(const boost::filesystem::path &path) :
    PATH_(path)
{
}

TestCase::~TestCase( )
{
}

void TestCase::operator( )( )
{
    TestSuccessfullHandling( );

    BOOST_CHECK_EQUAL(true, boost::filesystem::exists(PATH_));

    TestForgottenErase( );
    BOOST_CHECK_EQUAL(false, boost::filesystem::exists(PATH_));

    try
    {
        TestExceptionThrown( );
        BOOST_ERROR("The function should have thrown");
    }
    catch (const TestException &)
    {
    }

    BOOST_CHECK_EQUAL(false, boost::filesystem::exists(PATH_));
}

void TestCase::TestSuccessfullHandling( )
{
    myrrh::file::Eraser eraser(GetPath( ));
    PutOutputToFile(GetPath( ));
    eraser.Release( );
}

void TestCase::TestForgottenErase( )
{
    myrrh::file::Eraser eraser(GetPath( ));
    PutOutputToFile(GetPath( ));
}

void TestCase::TestExceptionThrown( )
{
    myrrh::file::Eraser eraser(GetPath( ));
    PutOutputToFile(GetPath( ));
    throw TestException( );
}

void TestCase::PutOutputToFile(const boost::filesystem::path &path)
{
    std::ofstream file(path.string( ).c_str( ));
    if (!file.is_open( ))
    {
        BOOST_FAIL("Failed to open file " + GetPath( ).string( ));
    }
    file << "Just some output" << std::endl;
    BOOST_CHECK_EQUAL(true, boost::filesystem::exists(PATH_));
}

const boost::filesystem::path &TestCase::GetPath( ) const
{
    return PATH_;
}

DirectoryTestCase::DirectoryTestCase(const boost::filesystem::path &path) :
    TestCase(path)
{
}

void DirectoryTestCase::TestSuccessfullHandling( )
{
    boost::filesystem::create_directory(GetPath( ));
    TestCase::TestSuccessfullHandling( );
}

void DirectoryTestCase::TestForgottenErase( )
{
    boost::filesystem::create_directory(GetPath( ));
    TestCase::TestForgottenErase( );
}

void DirectoryTestCase::TestExceptionThrown( )
{
    boost::filesystem::create_directory(GetPath( ));
    TestCase::TestExceptionThrown( );
}

void DirectoryTestCase::PutOutputToFile(const boost::filesystem::path &path)
{
    TestCase::PutOutputToFile(path / "File.txt");
}

void PathToFileInCurrentFolder( )
{
    TestCase("File.txt")( );
}

void RelativePathToAFileInSubfolder( )
{
    const boost::filesystem::path DIRECTORY("SubFolderThatShouldNotExist");
    const boost::filesystem::path FILE(DIRECTORY / "File.txt");
    boost::filesystem::create_directory(DIRECTORY);
    TestCase testCase(FILE);
    testCase( );
    boost::filesystem::remove(DIRECTORY);
}

void AbsolutePathToAFile( )
{
    TestCase(boost::filesystem::current_path( ) / "File.txt")( );
}

void PathToAnEmptyDirectory( )
{
    class Case : public DirectoryTestCase
    {
    public:
        Case( ) :
            DirectoryTestCase("SubFolderThatShouldNotExist")
        {
        }

    protected:

        virtual void PutOutputToFile(const boost::filesystem::path &)
        {
        }

    private:

        Case(const Case &) : DirectoryTestCase(""){ }
        Case &operator=(const Case &){ return *this; }
    };

    Case ( )( );
}

void PathToADirectory( )
{
    DirectoryTestCase testCase("SubFolderThatShouldNotExist");
    testCase( );
}
