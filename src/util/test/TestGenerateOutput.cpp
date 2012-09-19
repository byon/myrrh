// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit tests for myrrh::util::GenerateOutput function
 *
 * The following situations are tested:
 * -Output stream not open - OK
 * -Input stream not open - OK
 * -Zero output size - OK
 * -Output size is smaller than input data size - OK
 * -Output size is exactly input data size - OK
 * -Output size is larger than input size - OK
 * -Output size is several times larger than input size - OK
 * -Input stream contains no data - OK
 * -Input stream contains one character - OK
 * -Input stream contains several lines of data - OK
 * -Input stream contains data, but the position is in midway - OK
 * -Input stream contains data, but the position is at stream's end - OK
 *
 * $Id: TestGenerateOutput.cpp 354 2007-09-17 17:39:58Z byon $
 */

#include "myrrh/util/GenerateOutput.hpp"
#include "myrrh/file/Temporary.hpp"
#include "myrrh/data/test/Files.hpp"

#define BOOST_TEST_MODULE TestGenerateOutput
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <fstream>
#include <iostream>

using namespace myrrh::util;
using namespace myrrh::data::test;

namespace
{

std::string Path(const std::string &restOfPath);
template <typename Stream>
void CheckPath(const Stream &stream, const std::string &path);
std::string ReadRangeFromFile(const std::string &fileName,
                              const std::streampos start,
                              const std::streampos end);

class TestCase
{
public:

    virtual ~TestCase( );

    virtual void operator( )( );

protected:

    virtual std::string OutputStreamName( );

    virtual std::string InputStreamName( );

    virtual void OpenInputStream(std::ifstream &input);

    virtual void OpenOutputStream(std::ofstream &output);

    virtual std::streampos GetOutputSize( );

    virtual void DoTest(std::istream &input, std::ostream &output,
                        std::streamsize outputSize);

    virtual std::string GetExpectedResult( ) = 0;
};

class NoInputCase : public TestCase
{
private:

    virtual std::streampos GetOutputSize( );

    virtual std::string GetExpectedResult( );
};

class SuccessfullCase : public TestCase
{
private:

    virtual std::string GetExpectedResult( );
};

class ResultFromFileCase : public SuccessfullCase
{
public:

    ResultFromFileCase(const std::string &fileName);

private:

    virtual std::string InputStreamName( );

    ResultFromFileCase(const ResultFromFileCase &);
    ResultFromFileCase &operator=(const ResultFromFileCase &);

    const std::string FILE_NAME_;
};

}

BOOST_AUTO_TEST_SUITE(TestGenerateOutput)

BOOST_AUTO_TEST_CASE(OutputStreamNotOpen)
{
    class Case : public NoInputCase
    {
    protected:
        virtual void OpenOutputStream(std::ofstream &)
        {
        }

        virtual void DoTest(std::istream &input, std::ostream &output,
                            std::streamsize outputSize)
        {
            try
            {
                NoInputCase::DoTest(input, output, outputSize);
                BOOST_ERROR("Trying to generate output to non open file did "
                            "not cause exception");
            }
            catch (const WriteFailed &)
            {
            }
        }
    };

    Case( )( );
}

BOOST_AUTO_TEST_CASE(InputStreamNotOpen)
{
    class Case : public NoInputCase
    {
    protected:

        virtual void OpenInputStream(std::ifstream &)
        {
        }
    };

    Case( )( );
}

BOOST_AUTO_TEST_CASE(ZeroOutputSize)
{
    class Case : public NoInputCase
    {
    protected:

        virtual std::streampos GetOutputSize( )
        {
            return 0;
        }
    };

    Case( )( );
}

BOOST_AUTO_TEST_CASE(OutputSizeSmaller)
{
    class Case : public SuccessfullCase
    {
        virtual std::streampos GetOutputSize( )
        {
            return SuccessfullCase::GetOutputSize( ) / 2;
        }
    };

    Case( )( );
}

BOOST_AUTO_TEST_CASE(OutputSizeEqual)
{
    SuccessfullCase( )( );
}

BOOST_AUTO_TEST_CASE(OutputSizeLarger)
{
    class Case : public SuccessfullCase
    {
        virtual std::streampos GetOutputSize( )
        {
            const std::streampos ORIG(SuccessfullCase::GetOutputSize( ));
            return static_cast<std::streamoff>(ORIG * 1.5);
        }
    };

    Case( )( );
}

BOOST_AUTO_TEST_CASE(OutputSizeSeveralTimesLarger)
{
    class Case : public SuccessfullCase
    {
        virtual std::streampos GetOutputSize( )
        {
            return SuccessfullCase::GetOutputSize( ) * 9876;
        }
    };

    Case( )( );
}

BOOST_AUTO_TEST_CASE(NoInputData)
{
    ResultFromFileCase testCase(Files::EMPTY);
    testCase( );
}

BOOST_AUTO_TEST_CASE(InputDataOfOneChar)
{
    ResultFromFileCase testCase(Files::ONE_CHAR);
    testCase( );
}

BOOST_AUTO_TEST_CASE(InputDataOfSeveralLines)
{
    ResultFromFileCase testCase(Files::SEVERAL_LINES);
    testCase( );
}

BOOST_AUTO_TEST_CASE(InputStreamInMidway)
{
    class Case : public SuccessfullCase
    {
        virtual void OpenInputStream(std::ifstream &input)
        {
            TestCase::OpenInputStream(input);

            const std::streampos START(input.tellg( ));
            input.seekg(0, std::ios::end);
            const std::streampos END(input.tellg( ));

            input.seekg((END - START) / 2, std::ios::beg);
        }
    };

    Case( )( );
}

BOOST_AUTO_TEST_CASE(InputStreamAtItsEnd)
{
    class Case : public SuccessfullCase
    {
        virtual void OpenInputStream(std::ifstream &input)
        {
            TestCase::OpenInputStream(input);
            input.seekg(0, std::ios::end);
        }

        virtual std::streampos GetOutputSize( )
        {
            return 0;
        }
    };

    Case( )( );
}

BOOST_AUTO_TEST_SUITE_END( )

namespace
{

TestCase::~TestCase( )
{
}

void TestCase::operator( )( )
{
    myrrh::file::Temporary outputFile(OutputStreamName( ));

    std::ifstream input;
    OpenInputStream(input);

    std::ofstream output;
    OpenOutputStream(output);
    const std::streamoff OUTPUT_SIZE = GetOutputSize( );

    const std::streampos ORIGINAL_OUTPUT_POS(output.tellp( ));

    DoTest(input, output, OUTPUT_SIZE);

    const std::string RESULT(ReadRangeFromFile(OutputStreamName( ),
                             ORIGINAL_OUTPUT_POS, output.tellp( )));

    BOOST_CHECK_EQUAL(RESULT, GetExpectedResult( ));
}

void TestCase::OpenInputStream(std::ifstream &input)
{
    const std::string PATH(InputStreamName( ));
    input.open(PATH.c_str( ));
    CheckPath(input, PATH);
}

void TestCase::OpenOutputStream(std::ofstream &output)
{
    const std::string PATH(OutputStreamName( ));
    output.open(PATH.c_str( ));
    CheckPath(output, PATH);
}

std::string TestCase::InputStreamName( )
{
    return Path(Files::ONE_LINE);
}

std::string TestCase::OutputStreamName( )
{
    return "output.txt";
}

std::streampos TestCase::GetOutputSize( )
{
    std::ifstream stream;
    OpenInputStream(stream);
    stream.seekg(0, std::ios::end);
    return stream.tellg( );
}

void TestCase::DoTest(std::istream &input, std::ostream &output,
                      std::streamsize outputSize)
{
    GenerateOutput(input, output, outputSize);
}

std::streampos NoInputCase::GetOutputSize( )
{
    return 20;
}

std::string NoInputCase::GetExpectedResult( )
{
    return "";
}

// Too big method
std::string SuccessfullCase::GetExpectedResult( )
{
    std::ifstream stream;
    OpenInputStream(stream);

    std::ostringstream stringStream;
    stringStream << stream.rdbuf( );

    const std::size_t SIZE_REQUIRED = GetOutputSize( );
    const std::string &WHOLE_CONTENT = stringStream.str( );
    if (WHOLE_CONTENT.size( ) >= SIZE_REQUIRED)
    {
        return WHOLE_CONTENT.substr(0, SIZE_REQUIRED);
    }

    const size_t ENTIRE_COPY_TIMES = SIZE_REQUIRED / WHOLE_CONTENT.size( );
    const size_t REMAINDER = SIZE_REQUIRED % WHOLE_CONTENT.size( );

    std::string result;
    for (std::size_t i = 0; i < ENTIRE_COPY_TIMES; ++i)
    {
        result += WHOLE_CONTENT;
    }

    result += WHOLE_CONTENT.substr(0, REMAINDER);

    return result;
}

ResultFromFileCase::ResultFromFileCase(const std::string &fileName) :
    FILE_NAME_(fileName)
{
}

std::string ResultFromFileCase::InputStreamName( )
{
    return Path(FILE_NAME_);
}
std::string Path(const std::string &restOfPath)
{
    const std::string START("../../../../../..");
    return START + restOfPath;
}

template <typename Stream>
void CheckPath(const Stream &stream, const std::string &path)
{
    if (!stream.is_open( ))
    {
        BOOST_ERROR("Could not open " + path);
    }
}

std::string ReadRangeFromFile(const std::string &fileName,
                              const std::streampos start,
                              const std::streampos end)
{
    std::ifstream file(fileName.c_str( ));
    CheckPath(file, fileName);

    file.seekg(start);

    std::ostringstream stream;
    stream << file.rdbuf( );

    return stream.str( ).substr(0, end - start);
}

}
