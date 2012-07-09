// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for myrrh::file::Resize
 *
 * The following situations are tested:
 * -The file does not exist - OK
 * -The file should be resized to empty - OK
 * -Resize operation is made, but scanners show that the resizing shold not
 *  be made (start scanner points to file start and end scanner to file end)
 *  - OK
 * -The file should be resized to half - OK
 * -The file is empty before resize - OK
 * -The file contains only one character - OK
 * -The file contains only one line - OK
 * -The file contains several lines - OK
 *
 * The following situations are not tested, because they should be taken care
 * of by other classes:
 * -Invalid path
 * -The file is very large
 *
 * $Id: TestResize.cpp 362 2007-09-20 17:48:34Z byon $
 */

#include "myrrh/file/Resizer.hpp"
#include "myrrh/file/PositionScanner.hpp"
#include "myrrh/file/ReadOnly.hpp"
#include "myrrh/file/Temporary.hpp"
#include "myrrh/data/test/Files.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/filesystem/operations.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <fstream>

using namespace myrrh::util;
using namespace myrrh::file;
using namespace myrrh::data::test;

typedef boost::unit_test::test_suite TestSuite;

// Declarations of functions that implement test cases
void FileDoesNotExist( );
void FileResizedToEmpty( );
void FileResizeNotNeeded( );
void FileResizeMinimized( );
void FileResizedToHalf( );
void ResizingEmptyFile( );
void FileHasOneCharacter( );
void FileHasOneLine( );
void FileHasSeveralLines( );
void TemporaryFileCannotBeOpened( );

// Declarations of helper functions
std::string GetFileContent(const std::string &fileName);
std::string Path(const std::string &fileName);

class TestCase
{
public:

    virtual ~TestCase( );

    virtual void operator( )( );

protected:

    virtual void DoTest(Resizer &resizer);
    virtual Resizer GetResizer( );
    virtual std::string GetOriginalFileName( );
    virtual std::string GetTargetFileName( );
    virtual std::string GetExpectedResult( );
    virtual PositionScannerPtr GetStartScanner( );
    virtual PositionScannerPtr GetEndScanner( );
};

class BeginningPointVariesCase : public TestCase
{
public:
    BeginningPointVariesCase(PositionScannerPtr beginScanner);

protected:
    virtual PositionScannerPtr GetStartScanner( );

private:
    PositionScannerPtr beginScanner_;
};

class DifferentFileNameCase : public TestCase
{
public:
    DifferentFileNameCase(const std::string fileName);

protected:
    virtual std::string GetOriginalFileName( );

private:
    std::string fileName_;
};

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for Resize");

    test->add(BOOST_TEST_CASE(FileDoesNotExist));
    test->add(BOOST_TEST_CASE(FileResizedToEmpty));
    test->add(BOOST_TEST_CASE(FileResizeNotNeeded));
    test->add(BOOST_TEST_CASE(FileResizeMinimized));
    test->add(BOOST_TEST_CASE(FileResizedToHalf));
    test->add(BOOST_TEST_CASE(ResizingEmptyFile));
    test->add(BOOST_TEST_CASE(FileHasOneCharacter));
    test->add(BOOST_TEST_CASE(FileHasOneLine));
    test->add(BOOST_TEST_CASE(FileHasSeveralLines));
    /// @todo does not work
    /// test->add(BOOST_TEST_CASE(TemporaryFileCannotBeOpened));

    return test;
}

TestCase::~TestCase( )
{
    boost::filesystem::remove(GetTargetFileName( ));
}

void TestCase::operator( )( )
{
    boost::filesystem::copy_file(GetOriginalFileName( ), GetTargetFileName( ));

    Resizer resizer(GetResizer( ));
    DoTest(resizer);

    if (GetFileContent(GetTargetFileName( )) != GetExpectedResult( ))
    {
        BOOST_ERROR("Resize result is not as expected");
    }
}

void TestCase::DoTest(Resizer &resizer)
{
    resizer( );
}

Resizer TestCase::GetResizer( )
{
    return Resizer(GetTargetFileName( ), GetStartScanner( ),
                   GetEndScanner( ));
}

std::string TestCase::GetOriginalFileName( )
{
    return Path(Files::SEVERAL_LINES);
}

std::string TestCase::GetTargetFileName( )
{
    return "Output.txt";
}

std::string TestCase::GetExpectedResult( )
{
    std::ifstream file(GetOriginalFileName( ).c_str( ),
                       std::ios::in | std::ios::binary);
    BOOST_REQUIRE(file.is_open( ));

    PositionScannerPtr startScanner(GetStartScanner( ));
    PositionScannerPtr endScanner(GetEndScanner( ));

    const std::streamoff START = startScanner->Scan(file);
    const std::streamoff END = endScanner->Scan(file);
    const std::streamoff SIZE = END - START;
    assert(SIZE >= 0);

    if (!SIZE)
    {
        return "";
    }

    std::ostringstream stream;
    stream << file.rdbuf( );

    return stream.str( ).substr(START, SIZE);
}

PositionScannerPtr TestCase::GetStartScanner( )
{
    const boost::filesystem::path NAME(GetOriginalFileName( ));
    const std::streamsize SIZE =
        static_cast<std::streamsize>(boost::filesystem::file_size(NAME));
    return PositionScannerPtr(new ScanFromStart(SIZE / 2));
}

PositionScannerPtr TestCase::GetEndScanner( )
{
    return PositionScannerPtr(new EndScanner);
}

BeginningPointVariesCase::
BeginningPointVariesCase(PositionScannerPtr beginScanner) :
    beginScanner_(beginScanner)
{
}

PositionScannerPtr BeginningPointVariesCase::GetStartScanner( )
{
    return beginScanner_;
}

DifferentFileNameCase::DifferentFileNameCase(const std::string fileName) :
    fileName_(fileName)
{
}

std::string DifferentFileNameCase::GetOriginalFileName( )
{
    return Path(fileName_);
}

void FileDoesNotExist( )
{
    PositionScannerPtr startScanner(new ScanFromStart(1024));
    PositionScannerPtr endScanner(new EndScanner);
    Resizer resizer(Files::NOT_EXISTING, startScanner, endScanner);
    BOOST_CHECK_THROW(resizer( ), Resizer::NoFile);

    BOOST_CHECK(!boost::filesystem::exists(Files::NOT_EXISTING));
}

void FileResizedToEmpty( )
{
    PositionScannerPtr beginScanner(new EndScanner);
    BeginningPointVariesCase testCase(beginScanner);
    testCase( );
}

void FileResizeNotNeeded( )
{
    PositionScannerPtr beginScanner(new StartScanner);
    BeginningPointVariesCase testCase(beginScanner);
    testCase( );
}

void FileResizeMinimized( )
{
    PositionScannerPtr beginScanner(new StartScanner);
    BeginningPointVariesCase testCase(beginScanner);
    testCase( );
}

void FileResizedToHalf( )
{
    TestCase( )( );
}

void ResizingEmptyFile( )
{
    DifferentFileNameCase testCase(Files::EMPTY);
    testCase( );
}

void FileHasOneCharacter( )
{
    DifferentFileNameCase testCase(Files::ONE_CHAR);
    testCase( );
}

void FileHasOneLine( )
{
    DifferentFileNameCase testCase(Files::ONE_LONG_LINE);
    testCase( );
}

void TemporaryFileCannotBeOpened( )
{
    myrrh::file::ReadOnly original("TestFile.txt.tmp", "SomeContent");
    myrrh::file::Eraser eraser("TestFile.txt");
    {
        std::ofstream file("TestFile.txt");
        file << "SomeOtherContent" << std::endl;
    }

    PositionScannerPtr startScanner(new ScanFromStart(1024));
    PositionScannerPtr endScanner(new EndScanner);
    Resizer resizer("TestFile.txt", startScanner, endScanner);
    BOOST_CHECK_THROW(resizer( ), boost::filesystem::filesystem_error);
}

void FileHasSeveralLines( )
{
    DifferentFileNameCase testCase(Files::SEVERAL_LINES_NOT_EQUAL_LENGTH);
    testCase( );
}

std::string GetFileContent(const std::string &fileName)
{
    std::ifstream file(fileName.c_str( ),
                       std::ios::in | std::ios::binary);
    if (!file.is_open( ))
    {
        BOOST_ERROR("Failed to open file " + fileName);
    }

    std::ostringstream stream;
    stream << file.rdbuf( );

    return stream.str( );
}

std::string Path(const std::string &fileName)
{
    const std::string PATH_TO_ROOT(".");
    return PATH_TO_ROOT + fileName;
}
