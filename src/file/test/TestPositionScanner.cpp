// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for PositionScanner
 *
 * $Id: TestPositionScanner.cpp 360 2007-09-19 18:44:33Z byon $
 */

#include "myrrh/file/PositionScanner.hpp"
#include "myrrh/file/Eraser.hpp"
#include "myrrh/util/GenerateOutput.hpp"
#include "myrrh/data/test/Files.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#include "myrrh/util/Preprocessor.hpp"

#include "boost/shared_ptr.hpp"
#include "boost/algorithm/string/erase.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/test/unit_test.hpp"

#ifdef WIN32
#pragma warning (pop)
// This disables the assignment operator could not be generated for function
// local classes.
#pragma warning(disable: 4512)
#endif

#include <fstream>
#include <sstream>
#include <string>

typedef boost::unit_test::test_suite TestSuite;

using namespace myrrh::util;
using namespace myrrh::file;
using namespace myrrh::data::test;

// Helper function declarations
template <typename Stream>
void OpenStream(Stream &stream, const std::string &fileName,
                std::ios::openmode mode);
template <typename Stream>
void CheckStream(Stream &stream, const std::string &fileName);
std::string Path(const std::string &fileName);
void CheckScan(std::ifstream &stream, PositionScanner &scanner,
               std::streampos expectedResult);
const std::string GetContent(const boost::filesystem::path &path);
std::streampos FileEndPosition(const boost::filesystem::path &path);
template <typename ScannerTypes>
void AddBasicCases(TestSuite *suite);
void AddToStartCases(TestSuite *suite);
void AddScanFromStartPointCases(TestSuite *suite);
void AddScanFromEndPointCases(TestSuite *suite);

template <typename T>
boost::unit_test::test_case *NewCase(T caseFunction);

struct NoParams
{
    static const int PARAMETER_COUNT = 0;
};

template <typename PointCalculator>
struct PointParams : public PointCalculator
{
    template <typename FileNamer>
    std::streamsize GetParam1(const FileNamer &namer) const;
    static const int PARAMETER_COUNT = 1;
};

struct BeginningOfFile
{
    template <typename FileNamer>
    std::streamsize CalculatePoint(const FileNamer &namer) const;
};

struct EndOfFile
{
    template <typename FileNamer>
    std::streamsize CalculatePoint(const FileNamer &namer) const;
};

struct MiddleOfFile
{
    template <typename FileNamer>
    std::streamsize CalculatePoint(const FileNamer &namer) const;
};

struct TwiceFileSize
{
    template <typename FileNamer>
    std::streamsize CalculatePoint(const FileNamer &namer) const;
};

template <typename T, int ParamCount>
struct Creator { };

template <typename T>
struct Creator<T, 0>
{
    template <typename Params>
    PositionScannerPtr CreateScanner(const Params &params);
};

template <typename T>
struct Creator<T, 1>
{
    template <typename Params>
    PositionScannerPtr CreateScanner(const Params &params);
};

template <typename T>
struct Creator<T, 2>
{
    template <typename Params>
    PositionScannerPtr CreateScanner(const Params &params);
};

struct FalseOpener
{
    void Open(std::ifstream &stream, const boost::filesystem::path &path);
};

struct Opener
{
    void Open(std::ifstream &stream, const boost::filesystem::path &path);
};

struct DeleteAfterOpener
{
    void Open(std::ifstream &stream, const boost::filesystem::path &path);
};

template <int Index>
struct ExistingFileNamer
{
    boost::filesystem::path CreatePath( ) const;
};

struct CreatedFileNamer
{
    boost::filesystem::path CreatePath( ) const;
};

struct StartOutcome
{
    template <typename Params>
    std::streampos ExpectedOutcome(const boost::filesystem::path &path,
                                   const Params &params);
};

struct EndOutcome
{
    template <typename Params>
    std::streampos ExpectedOutcome(const boost::filesystem::path &path,
                                   const Params &params);
};

struct ScanFromStartOutcome
{
    template <typename Params>
    std::streampos ExpectedOutcome(const boost::filesystem::path &path,
                                   const Params &params);
};

struct ScanFromEndOutcome
{
    template <typename Params>
    std::streampos ExpectedOutcome(const boost::filesystem::path &path,
                                   const Params &params);
};

struct NoSetup
{
    template <typename T>
    NoSetup(T &host);
};

struct CreateLargeFile
{
    template <typename T>
    CreateLargeFile(T &host);

private:

    const boost::filesystem::path NAME_;
    Eraser eraser_;
};

struct NormalScan
{
    template <typename T>
    void Test(T &host);
};

struct ScanTwice
{
    template <typename T>
    void Test(T &host);
};

template <typename Error>
struct ErrorScan
{
    template <typename T>
    void Test(T &host);
};

template <typename Type,
          typename Params,
          typename Opener,
          typename Namer,
          typename Output,
          typename Case,
          typename Setup = NoSetup>
struct Host : public Params,
              public Creator<Type, Params::PARAMETER_COUNT>,
              public Opener,
              public Namer,
              public Output,
              public Case
{
    void operator( )( );
};

struct StartScannerTypes
{
    typedef StartScanner Scanner;
    typedef NoParams Params;
    typedef StartOutcome ExpectedOutcome;
};

struct EndScannerTypes
{
    typedef EndScanner Scanner;
    typedef NoParams Params;
    typedef EndOutcome ExpectedOutcome;
};

template <typename PointCalculator>
struct ScanFromStartTypes
{
    typedef ScanFromStart Scanner;
    typedef PointParams<PointCalculator> Params;
    typedef ScanFromStartOutcome ExpectedOutcome;
};

template <typename PointCalculator>
struct ScanFromEndTypes
{
    typedef ScanFromEnd Scanner;
    typedef PointParams<PointCalculator> Params;
    typedef ScanFromEndOutcome ExpectedOutcome;
};

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for PositionScanner");
    AddBasicCases<StartScannerTypes>(test);
    AddBasicCases<EndScannerTypes>(test);
    AddScanFromStartPointCases(test);
    AddScanFromEndPointCases(test);

    return test;
}

template <typename Types>
void AddBasicCases(TestSuite *suite)
{
    // File is not open case
    suite->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            FalseOpener,
                            ExistingFileNamer<Files::Index::SEVERAL_LINES>,
                            Types::ExpectedOutcome,
                            ErrorScan<PositionScanner::NotOpen> >( )));
    // File is deleted after opening
    /** @todo Not working test->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            DeleteAfterOpener,
                            ExistingFileNamer<Files::Index::SEVERAL_LINES>,
                            Types::ExpectedOutcome,
                            NormalScan>( )));*/
    // Reading empty file
    suite->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            Opener,
                            ExistingFileNamer<Files::Index::EMPTY>,
                            Types::ExpectedOutcome,
                            NormalScan>( )));
    // Reading file of one character
    suite->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            Opener,
                            ExistingFileNamer<Files::Index::ONE_CHAR>,
                            Types::ExpectedOutcome,
                            NormalScan>( )));
    // Reading file of one line
    suite->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            Opener,
                            ExistingFileNamer<Files::Index::ONE_LINE>,
                            Types::ExpectedOutcome,
                            NormalScan>( )));
    // Reading file of one long line
    suite->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            Opener,
                            ExistingFileNamer<Files::Index::ONE_LONG_LINE>,
                            Types::ExpectedOutcome,
                            NormalScan>( )));
    // Reading file of several lines
    suite->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            Opener,
                            ExistingFileNamer<Files::Index::SEVERAL_LINES>,
                            Types::ExpectedOutcome,
                            NormalScan>( )));
    // Reading file of several lines that are not of equal length
    suite->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            Opener,
                            ExistingFileNamer<
                                Files::Index::SEVERAL_LINES_NOT_EQUAL_LENGTH>,
                            Types::ExpectedOutcome,
                            NormalScan>( )));
    // Reading large file
    suite->add(NewCase(Host<Types::Scanner,
                            Types::Params,
                            Opener,
                            CreatedFileNamer,
                            Types::ExpectedOutcome,
                            NormalScan,
                            CreateLargeFile>( )));
}

void AddScanFromStartPointCases(TestSuite *suite)
{
    AddBasicCases<ScanFromStartTypes<BeginningOfFile> >(suite);
    AddBasicCases<ScanFromStartTypes<EndOfFile> >(suite);
    AddBasicCases<ScanFromStartTypes<MiddleOfFile> >(suite);
    AddBasicCases<ScanFromStartTypes<TwiceFileSize> >(suite);
}

void AddScanFromEndPointCases(TestSuite *suite)
{
    AddBasicCases<ScanFromEndTypes<BeginningOfFile> >(suite);
    AddBasicCases<ScanFromEndTypes<EndOfFile> >(suite);
    AddBasicCases<ScanFromEndTypes<MiddleOfFile> >(suite);
    AddBasicCases<ScanFromEndTypes<TwiceFileSize> >(suite);
}

template <typename T>
boost::unit_test::test_case *NewCase(T caseFunction)
{
    struct BeautifySymbol
    {
        std::string operator( )(const std::string &orig)
        {
            std::string copy(orig);
            boost::replace_all(copy, typeid(std::string).name( ),
                               "std::string");
            boost::erase_all(copy, "struct ");
            boost::erase_all(copy, "class ");

            return copy;
        }
    };

    using namespace boost::unit_test;
    BeautifySymbol beautifier;
    return make_test_case(callback0<>(caseFunction),
                          beautifier(typeid(T).name( )));
};

template <typename PointCalculator>
    template <typename FileNamer>
std::streamsize PointParams<PointCalculator>::
GetParam1(const FileNamer &namer) const
{
    return CalculatePoint(namer);
}

template <typename FileNamer>
std::streamsize BeginningOfFile::CalculatePoint(const FileNamer &) const
{
    return std::streampos(0);
}

template <typename FileNamer>
std::streamsize EndOfFile::CalculatePoint(const FileNamer &namer) const
{
    return FileEndPosition(namer.CreatePath( ));
}

template <typename FileNamer>
std::streamsize MiddleOfFile::CalculatePoint(const FileNamer &namer) const
{
    return FileEndPosition(namer.CreatePath( )) / 2;
}

template <typename FileNamer>
std::streamsize TwiceFileSize::CalculatePoint(const FileNamer &namer) const
{
    return FileEndPosition(namer.CreatePath( )) * 2;
}

template <typename T>
    template <typename Params>
PositionScannerPtr Creator<T, 0>::CreateScanner(const Params &)
{
    return PositionScannerPtr(new T( ));
}

template <typename T>
    template <typename Params>
PositionScannerPtr Creator<T, 1>::CreateScanner(const Params &params)
{
    return PositionScannerPtr(new T(params.GetParam1(params)));
}

template <typename T>
    template <typename Params>
PositionScannerPtr Creator<T, 2>::CreateScanner(const Params &params)
{
    return PositionScannerPtr(new T(params.GetParam1(params),
                                    params.GetParam2(params)));
}

void FalseOpener::Open(std::ifstream &stream, const boost::filesystem::path &)
{
    BOOST_CHECK(!stream.is_open( ));
}

void Opener::Open(std::ifstream &stream, const boost::filesystem::path &path)
{
    OpenStream(stream, path.string( ), std::ios::in);
}

void DeleteAfterOpener::Open(std::ifstream &stream,
                             const boost::filesystem::path &path)
{
    Eraser tmp(path.string( ));
    OpenStream(stream, path.string( ), std::ios::in);
}

template <int Index>
boost::filesystem::path ExistingFileNamer<Index>::CreatePath( ) const
{
    return Path(Files::GetAll( )[Index]);
}

boost::filesystem::path CreatedFileNamer::CreatePath( ) const
{
    return "output.txt";
}

template <typename Params>
std::streampos
StartOutcome::ExpectedOutcome(const boost::filesystem::path &, const Params &)
{
    return std::streampos(0);
}

template <typename Params>
std::streampos
EndOutcome::ExpectedOutcome(const boost::filesystem::path &path,
                            const Params &)
{
    return FileEndPosition(path);
}

template <typename Params>
std::streampos ScanFromStartOutcome::
ExpectedOutcome(const boost::filesystem::path &path, const Params &params)
{
    const std::streamsize POINT = params.GetParam1(params);
    const std::streamsize SIZE = FileEndPosition(path);

    if (POINT >= SIZE)
    {
        return SIZE;
    }

    const std::string CONTENT = GetContent(path);
    const std::size_t NEXT_END_OF_LINE = CONTENT.find('\n', POINT);
    if (std::string::npos == NEXT_END_OF_LINE)
    {
        return FileEndPosition(path);
    }

    return static_cast<std::streamsize>(NEXT_END_OF_LINE +1);
}

template <typename Params>
std::streampos ScanFromEndOutcome::
ExpectedOutcome(const boost::filesystem::path &path, const Params &params)
{
    const std::streamsize POINT = params.GetParam1(params);
    const std::streamsize SIZE = FileEndPosition(path);

    if (POINT >= SIZE)
    {
        return SIZE;
    }

    const std::string CONTENT = GetContent(path);
    const std::size_t NEXT_END_OF_LINE = CONTENT.find('\n', SIZE - POINT);
    if (std::string::npos == NEXT_END_OF_LINE)
    {
        return FileEndPosition(path);
    }

    return static_cast<std::streamsize>(NEXT_END_OF_LINE +1);
}

template <typename T>
inline NoSetup::NoSetup(T &)
{
}

template <typename T>
CreateLargeFile::CreateLargeFile(T &host) :
    NAME_(host.CreatePath( )),
    eraser_(NAME_)
{
    std::ifstream input;
    OpenStream(input, Path(Files::SEVERAL_LINES_NOT_EQUAL_LENGTH),
               std::ios::in);

    std::ofstream output;
    OpenStream(output, NAME_.string( ), std::ios::out);

    GenerateOutput(input, output, 1000000);
}

template <typename T>
void NormalScan::Test(T &host)
{
    PositionScannerPtr scanner(host.CreateScanner(host));

    const boost::filesystem::path PATH(host.CreatePath( ));
    std::ifstream stream;
    host.Open(stream, PATH);

    CheckScan(stream, *scanner, host.ExpectedOutcome(PATH, host));
}

template <typename T>
void ScanTwice::Test(T &host)
{
    PositionScannerPtr scanner(host.CreateScanner(host));

    const boost::filesystem::path PATH(host.CreatePath( ));
    std::ifstream stream;
    host.Open(stream, PATH);

    CheckScan(stream, *scanner, host.ExpectedOutcome(PATH, host));
    CheckScan(stream, *scanner, host.ExpectedOutcome(PATH, host));
}

template <typename Error>
    template <typename T>
void ErrorScan<Error>::Test(T &host)
{
    PositionScannerPtr scanner(host.CreateScanner(host));

    const boost::filesystem::path PATH(host.CreatePath( ));
    std::ifstream stream;
    host.Open(stream, PATH);

    try
    {
        scanner->Scan(stream);
        BOOST_ERROR("Scanning was expected to throw");
    }
    catch (const Error &)
    {
    }
}

template <typename Type,
          typename Params,
          typename Opener,
          typename Namer,
          typename Output,
          typename Case,
          typename Setup>
void Host<Type,
          Params,
          Opener,
          Namer,
          Output,
          Case,
          Setup>::operator( )( )
{
    Setup setup(*this);
    Test(*this);
}

// Helper function implementations

template <typename Stream>
void OpenStream(Stream &stream, const std::string &fileName,
                const std::ios::openmode mode)
{
    stream.open(fileName.c_str( ), mode);
    CheckStream(stream, fileName);
}

template <typename Stream>
void CheckStream(Stream &stream, const std::string &fileName)
{
    if (!stream.is_open( ))
    {
        BOOST_ERROR("Failed to open " + fileName);
    }
}

std::string Path(const std::string &fileName)
{
    return "../../../../.." + fileName;
}

void CheckScan(std::ifstream &stream, PositionScanner &scanner,
               std::streampos expectedResult)
{
    const std::streampos ORIG_POS = stream.tellg( );

    const std::streampos RESULT = scanner.Scan(stream);

    BOOST_CHECK_EQUAL(ORIG_POS, stream.tellg( ));
    BOOST_CHECK_EQUAL(RESULT, expectedResult);
}

const std::string GetContent(const boost::filesystem::path &path)
{
    std::ifstream stream;
    OpenStream(stream, path.string( ), std::ios::in | std::ios::binary);

    std::ostringstream stringStream;
    stringStream << stream.rdbuf( );

    return stringStream.str( );
}

std::streampos FileEndPosition(const boost::filesystem::path &path)
{
    std::ifstream stream;
    OpenStream(stream, path.string( ), std::ios::in);
    stream.seekg(0, std::ios::end);
    return stream.tellg( );
}
