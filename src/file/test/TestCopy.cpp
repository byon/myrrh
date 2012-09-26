// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit tests for myrrh::file::Copy class.
 *
 * The following situations are tested:
 * -Output stream is not open
 * -Start point scanner scans to a point after end point scanner
 * -The scanners result in zero length range
 * -The scanners point from start to finish
 * -The scanners point from midway to finish
 * -The scanners result in a very small range
 * -The copying is done from a very large file
 *
 * The following situations are not tested, because they should already be
 * tested in PositionScanner unit tests:
 * -Input stream is not open
 * -Start point scanner scans to a point that is before file start
 * -Start point scanner scans to a point that is after file end
 * -End point scanner scans to a point that is after file end
 *
 *
 * $Id: TestCopy.cpp 360 2007-09-19 18:44:33Z byon $
 */

#include "myrrh/file/Copy.hpp"
#include "myrrh/file/PositionScanner.hpp"
#include "myrrh/file/Eraser.hpp"
#include "myrrh/util/GenerateOutput.hpp"
#include "myrrh/data/test/Files.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/filesystem/path.hpp"
#include "boost/test/unit_test.hpp"
#include "boost/scoped_array.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

typedef boost::unit_test::test_suite TestSuite;

#include <fstream>

using namespace myrrh::data::test;
using namespace myrrh::file;

// Function declarations for separate test cases
void OutputStreamNotOpen( );
void StartPointAfterEndPoint( );
void ZeroLengthRange( );
void CopyFileEntirely( );
void CopyFromMiddleToEnd( );
void CopySmallRange( );
void CopyLargeRange( );
std::string GetFileContent(const boost::filesystem::path &path);

// Declarations for helper function

std::string Path(const std::string &baseName);

/**
 * A tester implementation of PositionScanner. It is used to return a
 * hard-coded value as scanned position. It is usable only for this unit test.
 */
template <int Value>
class HardCodedScanner : public PositionScanner
{
public:

    static const int ScanPoint = Value;

private:

    virtual std::streampos DoScan(std::ifstream &stream) const;
};

class FromMiddleScanner : public PositionScanner
{
    virtual std::streampos DoScan(std::ifstream &stream) const;
};

class TestCase
{
public:

    TestCase( );

    virtual ~TestCase( );

    virtual void operator( )( );

protected:

    virtual Copy GetCopier( );

    virtual PositionScannerPtr StartScanner( ) const = 0;

    virtual PositionScannerPtr EndScanner( ) const = 0;

    virtual void OpenInputStream(std::ifstream &input) const;

    virtual void OpenOutputStream(std::ofstream &output) const;

    virtual std::string InputFileName( ) const;
    virtual std::string OutputFileName( ) const;

    virtual std::string GetExpectedResult( ) const;
};

template <int S, int E>
class HardCodedPointsTestCase : public TestCase
{
public:

    static const int Start = S;
    static const int End = E;

private:

    virtual PositionScannerPtr StartScanner( ) const;

    virtual PositionScannerPtr EndScanner( ) const;
};

template <typename Exception, int S, int E>
class InvalidRangeTestcase : public HardCodedPointsTestCase<S, E>
{
public:

    virtual void operator( )( )
    {
        try
        {
            HardCodedPointsTestCase<S, E>::operator( )( );
            BOOST_ERROR("Should have thrown with invalid range");
        }
        catch (const Exception &)
        {
        }
    }
};

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite *test = BOOST_TEST_SUITE("Test suite for FileCopy");

    test->add(BOOST_TEST_CASE(OutputStreamNotOpen));
    test->add(BOOST_TEST_CASE(StartPointAfterEndPoint));
    test->add(BOOST_TEST_CASE(ZeroLengthRange));
    test->add(BOOST_TEST_CASE(CopyFileEntirely));
    test->add(BOOST_TEST_CASE(CopyFromMiddleToEnd));
    test->add(BOOST_TEST_CASE(CopySmallRange));
    test->add(BOOST_TEST_CASE(CopyLargeRange));

    return test;
}

TestCase::TestCase( )
{
}

TestCase::~TestCase( )
{
}

void TestCase::operator( )( )
{
    std::ifstream input;
    OpenInputStream(input);

    Eraser fileEraser(OutputFileName( ));
    std::ofstream output;
    OpenOutputStream(output);

    GetCopier( )(input, output);

    if (GetFileContent(OutputFileName( )) != GetExpectedResult( ))
    {
        BOOST_ERROR("Result is not as expected");
    }
}

Copy TestCase::GetCopier( )
{
    return Copy(StartScanner( ), EndScanner( ));
}

void TestCase::OpenInputStream(std::ifstream &input) const
{
    input.open(InputFileName( ).c_str( ), std::ios::in | std::ios::binary);
    if (!input.is_open( ))
    {
        BOOST_FAIL("Failed to open " + InputFileName( ));
    }
}

void TestCase::OpenOutputStream(std::ofstream &output) const
{
    output.open(OutputFileName( ).c_str( ));
    if (!output.is_open( ))
    {
        BOOST_FAIL("Failed to open " + OutputFileName( ));
    }
}

std::string TestCase::InputFileName( ) const
{
    return Path(Files::SEVERAL_LINES);
}

std::string TestCase::OutputFileName( ) const
{
    return "output.txt";
}

std::string TestCase::GetExpectedResult( ) const
{
    std::ifstream input(InputFileName( ).c_str( ),
                        std::ios::in | std::ios::binary);
    BOOST_REQUIRE(input.is_open( ));

    PositionScannerPtr startScanner(StartScanner( ));
    PositionScannerPtr endScanner(EndScanner( ));

    const std::streamoff START = startScanner->Scan(input);
    const std::streamoff END = endScanner->Scan(input);
    const std::streamoff SIZE = END - START;
    assert(SIZE >= 0);
    if (SIZE == 0)
    {
        return "";
    }

    std::ostringstream stream;
    stream << input.rdbuf( );

    return stream.str( ).substr(START, SIZE);
}

template <int S, int E>
PositionScannerPtr HardCodedPointsTestCase<S, E>::StartScanner( ) const
{
    return PositionScannerPtr(new HardCodedScanner<Start>( ));
}

template <int S, int E>
PositionScannerPtr HardCodedPointsTestCase<S, E>::EndScanner( ) const
{
    return PositionScannerPtr(new HardCodedScanner<End>( ));
}

void OutputStreamNotOpen( )
{
    struct Case : public HardCodedPointsTestCase<0, 0>
    {
        virtual void operator( )( )
        {
            try
            {
                HardCodedPointsTestCase<0, 0>::operator( )( );
                BOOST_ERROR("Should have thrown with unopened output stream");
            }
            catch (const Copy::StreamNotOpen &)
            {
            }
        }

        virtual void OpenOutputStream(std::ofstream &) const
        {
        }
    };

    Case( )( );
}

void StartPointAfterEndPoint( )
{
    InvalidRangeTestcase<Copy::OutOfBounds, 10, 6>( )( );
}

void ZeroLengthRange( )
{
    HardCodedPointsTestCase<10, 10>( )( );
}

void CopyFileEntirely( )
{
    class Case : public TestCase
    {
        virtual PositionScannerPtr StartScanner( ) const
        {
            return PositionScannerPtr(new myrrh::file::StartScanner);
        }

        virtual PositionScannerPtr EndScanner( ) const
        {
            return PositionScannerPtr(new myrrh::file::EndScanner);
        }
    };

    Case( )( );
}

void CopyFromMiddleToEnd( )
{
    class Case : public TestCase
    {
        virtual PositionScannerPtr StartScanner( ) const
        {
            return PositionScannerPtr(new FromMiddleScanner);
        }

        virtual PositionScannerPtr EndScanner( ) const
        {
            return PositionScannerPtr(new myrrh::file::EndScanner);
        }
    };

    Case( )( );
}

void CopySmallRange( )
{
    HardCodedPointsTestCase<6, 7>( )( );
}

void CopyLargeRange( )
{
    std::ifstream input(Path(Files::SEVERAL_LINES_NOT_EQUAL_LENGTH).c_str( ));
    BOOST_REQUIRE(input.is_open( ));

    const std::string FILE_NAME("LargeFile.txt");
    Eraser fileDeleter(FILE_NAME.c_str( ));
    std::ofstream output(FILE_NAME.c_str( ));
    BOOST_REQUIRE(output.is_open( ));

    myrrh::util::GenerateOutput(input, output, 1000000);

    class Case : public HardCodedPointsTestCase<123456, 654321>
    {
    public:
        Case(const std::string &fileName) :
            fileName_(fileName)
        {
        }

    protected:
        virtual std::string InputFileName( ) const
        {
            return fileName_;
        }

        std::string fileName_;
    };

    Case testCase(FILE_NAME);
    testCase( );
}

template <int Value>
std::streampos HardCodedScanner<Value>::DoScan(std::ifstream &) const
{
    return ScanPoint;
}

std::streampos FromMiddleScanner::DoScan(std::ifstream &stream) const
{
    const std::streampos ORIG = stream.tellg( );

    stream.seekg(0, std::ios::end);
    const std::streampos END = stream.tellg( );

    stream.seekg(ORIG);

    return END / 2;
}

std::string Path(const std::string &baseName)
{
    const std::string PATH("../../../../../..");
    return PATH + baseName;
}

std::string GetFileContent(const boost::filesystem::path &path)
{
    std::ifstream file(path.string( ).c_str( ));
    BOOST_REQUIRE(file.is_open( ));

    std::ostringstream stream;
    stream << file.rdbuf( );

    return stream.str( );
}
