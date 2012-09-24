// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for example functions declared in
 * myrrh/log/policy/Examples.hpp header.
 *
 * @todo refactor into several separate files
 *
 * $Id: TestExamples.cpp 369 2007-09-20 20:08:51Z byon $
 */

// This file is not really worth maintaining. If there are problems later,
// rewrite. Problems:
// - Too much complexity
// - Not clear which tests fail
// - Not clear what fails when a test fails

#include "myrrh/log/policy/Examples.hpp"
#include "File.hpp"
#include "myrrh/file/Eraser.hpp"

#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#define DISABLE_COPY_CONSTRUCTOR_COULD_NOT_BE_GENERATED
#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_DEPRECATED_FUNCTION_WARNING
#define DISABLE_SIGNED_UNSIGNED_MISMATCH
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/regex.hpp"
#include "boost/date_time/special_defs.hpp"
#include "boost/date_time/local_time/local_time_types.hpp"
#include "boost/filesystem/convenience.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

typedef boost::unit_test::test_suite TestSuite;
typedef std::vector<std::string> Lines;

using namespace myrrh::log;

// Forward declarations
struct TimestampedFileNamer;
struct PlainFileNamer;

// Helper function and class declarations
template <typename Host>
Files CreateStartFiles(Host &host);
template <typename Host>
boost::filesystem::path CreateFileName(const Host &host);
Files GetPhysicalFiles(const boost::filesystem::path &folder,
                       PlainFileNamer &namer);
Files GetPhysicalFiles(const boost::filesystem::path &folder,
                       TimestampedFileNamer &namer);
template <typename Host>
void SetupOneStartFileForChecking(Files &files, Host &host);
void WriteFiles(const Files &files);
void EraseMatchingFiles(const std::string &expression);
void EraseTestFiles( );
File FileFromHardDisk(const boost::filesystem::path &path);
void AddDatedFolderCases(TestSuite *test);
template <typename Creator, typename StartFiles, typename Subfolder>
void AddSizeRestrictedLogCases(TestSuite *test);
template <typename Creator, typename StartFiles, typename Subfolder>
void AddSizeRestrictedLogsCases(TestSuite *test);
boost::regex CreateTimeStampExpression(const std::string &folderPath,
                                       const std::string &name);
size_t AdjustSize(const std::string &text);
template <typename T>
size_t FileAndRowSize(const File &file, const T &row);

struct SizeRowStopper
{
    SizeRowStopper( );

    template <typename ContentCreator>
    bool DoesRowFit(ContentCreator &contentCreator, const std::string &row);

    void StoreRow(const std::string &row);

private:

    std::size_t written_;
};

struct NeverStopper
{
    template <typename ContentCreator>
    bool DoesRowFit(ContentCreator &contentCreator, const std::string &row);

    void StoreRow(const std::string &row);
};

template <typename ContentCreator, typename Stopper = SizeRowStopper>
struct Row
{
    explicit Row(ContentCreator &contentCreator);

    Row( );

    const std::string &operator*( ) const;

    std::size_t Size( ) const;

    Row &operator++( );

    friend bool operator==(const Row &left, const Row &right)
    {
        return left.contentCreator_ == right.contentCreator_ &&
               left.currentRow_ == right.currentRow_;
    }

    friend bool operator!=(const Row &left, const Row &right)
    {
        return !(left == right);
    }

private:
    ContentCreator *contentCreator_;
    std::string currentRow_;
    Stopper stopper_;
};

template <std::size_t Divider = 2>
struct DividerOperation
{
    std::size_t operator( )(size_t originalSize);
};

struct NoOperation
{
    std::size_t operator( )(size_t originalSize);
};

template <std::size_t Multiplier = 2>
struct MultiplyOperation
{
    std::size_t operator( )(size_t originalSize);
};

template <typename T>
boost::unit_test::test_case *NewCase(T caseFunction);

class TestSetup
{
public:

    TestSetup(const Files &files);
    ~TestSetup( );
};

// Test case policy class declarations

// PolicyCreator classes

struct DatedFolderLogCreator
{
    template <typename T>
    policy::PolicyPtr CreatePolicy(const T &host) const;
};

struct SizeRestrictedLogCreator
{
    template <typename T>
    policy::PolicyPtr CreatePolicy(const T &host) const;
};

struct SizeRestrictedDatedFolderLogCreator
{
    template <typename T>
    policy::PolicyPtr CreatePolicy(const T &host) const;
};

struct SizeRestrictedLogsCreator
{
    template <typename T>
    policy::PolicyPtr CreatePolicy(const T &host) const;
};

struct SizeRestrictedDatedFolderLogsCreator
{
    template <typename T>
    policy::PolicyPtr CreatePolicy(const T &host) const;
};

// Parameter classes

struct NoParams
{
};

template <std::size_t MaxSize>
struct MaxSizeParams
{
    template <typename T>
    std::size_t GetParam1(const T &host) const;
};

// Row content creator classes

struct FixedRows
{
    std::string FirstRow( );
    std::string NextRow( );
};

struct CountedRows
{
    CountedRows( );
    std::string FirstRow( );
    std::string NextRow( );

private:

    std::size_t counter_;
};

// File size classes

template <std::size_t Size>
struct FixedSize
{
    template <typename Host>
    size_t GetWriteSize(const Host &);
};

template <typename Operation>
struct CountedSize
{
    template <typename Host>
    size_t GetWriteSize(const Host &host);
};

// Start file creator classes

struct NoStartFiles
{
    template <typename Host, typename RowType>
    void FillStartFiles(Files &files, const Host &host,
                        RowType &currentRow) const;
};

template <std::size_t Size>
struct FixedSizeFile
{
    template <typename Host, typename RowType>
    void FillStartFiles(Files &files, const Host &host,
                        RowType &currentRow) const;
};

template <typename Operation, typename Next = NoStartFiles>
struct CountedStartFiles
{
    template <typename Host, typename RowType>
    void FillStartFiles(Files &files, const Host &host,
                        RowType &currentRow) const;
};

template <typename Next = NoStartFiles>
struct DividedFiles : public CountedStartFiles<DividerOperation<>, Next>
{
};

template <typename Next = NoStartFiles>
struct MultipliedFiles : public CountedStartFiles<MultiplyOperation<>, Next>
{
};

template <typename Next = NoStartFiles>
struct MaxSizeFiles : public CountedStartFiles<NoOperation, Next>
{
};

typedef DividedFiles<> OneHalfFile;
typedef MaxSizeFiles<> OneFile;
typedef MultipliedFiles<> OneDoubleFile;

template <template <typename> class Next, typename Last, int Count>
struct FixedCountStartFiles
{
    typedef typename FixedCountStartFiles<Next, Last, Count -1>::Type NextType;
    typedef Next<NextType> Type;
};

template <template <typename> class Next, typename Last>
struct FixedCountStartFiles<Next, Last, 0>
{
    typedef Last Type;
};

typedef FixedCountStartFiles<DividedFiles, NoStartFiles, 2>::Type TwoHalfFiles;
typedef FixedCountStartFiles<MultipliedFiles,
                             NoStartFiles, 2>::Type TwoDoubleFiles;
typedef FixedCountStartFiles<MaxSizeFiles, NoStartFiles, 2>::Type TwoFiles;

typedef FixedCountStartFiles<DividedFiles,
                             OneHalfFile, 2>::Type TwoFilesAndOneHalf;
typedef FixedCountStartFiles<MultipliedFiles,
                             OneDoubleFile, 2>::Type TwoFilesAndOneDouble;

typedef DividedFiles<TwoFiles> OneHalfAndTwoFiles;
typedef DividedFiles<TwoFiles> OneDoubleAndTwoFiles;

typedef FixedCountStartFiles<MaxSizeFiles, NoStartFiles, 3>::Type ThreeFiles;


// Expected outcome classes

struct OneLogOutcome
{
    template <typename T>
    Files ExpectedOutcome(T &host);
};

struct SizeRestrictedLogOutcome
{
    template <typename T>
    Files ExpectedOutcome(T &host);
};

struct SizeRestrictedLogsOutcome
{
    template <typename T>
    Files ExpectedOutcome(T &host);
};

// Path classes

struct NoSubFolder
{
    const boost::filesystem::path GetPath( ) const;
};

struct DatedSubFolder
{
    const boost::filesystem::path GetPath( ) const;
};

// File namer classes

struct PlainFileNamer
{
    const boost::filesystem::path GetName( ) const;
};

struct TimestampedFileNamer
{
    const boost::filesystem::path GetName( ) const;
};

struct NormalCase
{
    template <typename T>
    void Test(T &host);
};

template <typename Creator,
          typename Params,
          typename RowCreator,
          typename FileSizeDeterminer,
          typename StartFileCreator,
          typename ExpectedOutcome,
          typename Subfolder,
          typename Namer,
          typename Case>
struct Host :
    public Creator,
    public Params,
    public RowCreator,
    public FileSizeDeterminer,
    public StartFileCreator,
    public ExpectedOutcome,
    public Subfolder,
    public Namer,
    public Case
{
    void operator( )( );
};

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for Examples");

    AddDatedFolderCases(test);

    // SizeRestrictedLogCreator
    AddSizeRestrictedLogCases<SizeRestrictedLogCreator, NoStartFiles,
                              NoSubFolder>(test);
    AddSizeRestrictedLogCases<SizeRestrictedLogCreator, OneHalfFile,
                              NoSubFolder>(test);
    AddSizeRestrictedLogCases<SizeRestrictedLogCreator, OneFile,
                              NoSubFolder>(test);
    AddSizeRestrictedLogCases<SizeRestrictedLogCreator, OneDoubleFile,
                              NoSubFolder>(test);
    // SizeRestrictedDatedFolderLogCreator
    AddSizeRestrictedLogCases<SizeRestrictedDatedFolderLogCreator,
                              NoStartFiles, DatedSubFolder>(test);
    AddSizeRestrictedLogCases<SizeRestrictedDatedFolderLogCreator,
                              OneHalfFile, DatedSubFolder>(test);
    AddSizeRestrictedLogCases<SizeRestrictedDatedFolderLogCreator,
                              OneFile, DatedSubFolder>(test);
    AddSizeRestrictedLogCases<SizeRestrictedDatedFolderLogCreator,
                              OneDoubleFile, DatedSubFolder>(test);
    // SizeRestrictedLogsCreator
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               NoStartFiles, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               OneHalfFile, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               OneFile, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               OneDoubleFile, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               TwoHalfFiles, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               TwoDoubleFiles, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               TwoFiles, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               TwoFilesAndOneHalf, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               TwoFilesAndOneDouble, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               OneHalfAndTwoFiles, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               OneDoubleAndTwoFiles, NoSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedLogsCreator,
                               ThreeFiles, NoSubFolder>(test);
    // SizeRestrictedDatedFolderLogsCreator
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               NoStartFiles, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               OneHalfFile, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               OneFile, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               OneDoubleFile, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               TwoHalfFiles, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               TwoDoubleFiles, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               TwoFiles, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               TwoFilesAndOneHalf, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               TwoFilesAndOneDouble, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               OneHalfAndTwoFiles, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               OneDoubleAndTwoFiles, DatedSubFolder>(test);
    AddSizeRestrictedLogsCases<SizeRestrictedDatedFolderLogsCreator,
                               ThreeFiles, DatedSubFolder>(test);
    return test;
}

void AddDatedFolderCases(TestSuite *test)
{
    test->add(NewCase(Host<DatedFolderLogCreator,
                           NoParams,
                           CountedRows,
                           FixedSize<64>,
                           NoStartFiles,
                           OneLogOutcome,
                           DatedSubFolder,
                           PlainFileNamer,
                           NormalCase>( )));
    test->add(NewCase(Host<DatedFolderLogCreator,
                           NoParams,
                           CountedRows,
                           FixedSize<64>,
                           FixedSizeFile<64>,
                           OneLogOutcome,
                           DatedSubFolder,
                           PlainFileNamer,
                           NormalCase>( )));
}

template <typename Creator, typename StartFiles, typename Subfolder>
void AddSizeRestrictedLogCases(TestSuite *test)
{
    test->add(NewCase(Host<Creator,
                           MaxSizeParams<64>,
                           CountedRows,
                           CountedSize<DividerOperation<> >,
                           StartFiles,
                           SizeRestrictedLogOutcome,
                           Subfolder,
                           PlainFileNamer,
                           NormalCase>( )));
    test->add(NewCase(Host<Creator,
                           MaxSizeParams<64>,
                           CountedRows,
                           CountedSize<NoOperation >,
                           StartFiles,
                           SizeRestrictedLogOutcome,
                           Subfolder,
                           PlainFileNamer,
                           NormalCase>( )));
    test->add(NewCase(Host<Creator,
                           MaxSizeParams<64>,
                           CountedRows,
                           CountedSize<MultiplyOperation<> >,
                           StartFiles,
                           SizeRestrictedLogOutcome,
                           Subfolder,
                           PlainFileNamer,
                           NormalCase>( )));
}

template <typename Creator, typename StartFiles, typename Subfolder>
void AddSizeRestrictedLogsCases(TestSuite *test)
{
    test->add(NewCase(Host<Creator,
                           MaxSizeParams<64>,
                           CountedRows,
                           CountedSize<DividerOperation<> >,
                           StartFiles,
                           SizeRestrictedLogsOutcome,
                           Subfolder,
                           TimestampedFileNamer,
                           NormalCase>( )));
    test->add(NewCase(Host<Creator,
                           MaxSizeParams<64>,
                           CountedRows,
                           CountedSize<NoOperation >,
                           StartFiles,
                           SizeRestrictedLogsOutcome,
                           Subfolder,
                           TimestampedFileNamer,
                           NormalCase>( )));
    test->add(NewCase(Host<Creator,
                           MaxSizeParams<64>,
                           CountedRows,
                           CountedSize<MultiplyOperation<> >,
                           StartFiles,
                           SizeRestrictedLogsOutcome,
                           Subfolder,
                           TimestampedFileNamer,
                           NormalCase>( )));
}

template <typename T>
boost::unit_test::test_case *NewCase(T caseFunction)
{
    /// @todo Only worked for VS2005
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

template <typename T>
policy::PolicyPtr DatedFolderLogCreator::CreatePolicy(const T &) const
{
    return policy::DatedFolderLog( );
}

template <typename T>
policy::PolicyPtr SizeRestrictedLogCreator::CreatePolicy(const T &host) const
{
    return policy::SizeRestrictedLog(host.GetParam1(host));
}

template <typename T>
policy::PolicyPtr SizeRestrictedDatedFolderLogCreator::
CreatePolicy(const T &host) const
{
    return policy::SizeRestrictedDatedFolderLog(host.GetParam1(host));
}

template <typename T>
policy::PolicyPtr SizeRestrictedLogsCreator::CreatePolicy(const T &host) const
{
    return policy::SizeRestrictedLogs(host.GetParam1(host));
}

template <typename T>
policy::PolicyPtr SizeRestrictedDatedFolderLogsCreator::
CreatePolicy(const T &host) const
{
    return policy::SizeRestrictedDatedFolderLogs(host.GetParam1(host));
}

inline std::string FixedRows::FirstRow( )
{
    return "abcd";
}

inline std::string FixedRows::NextRow( )
{
    return FirstRow( );
}

CountedRows::CountedRows( ) :
    counter_(0)
{
}

inline std::string CountedRows::FirstRow( )
{
    counter_ = 0;
    return NextRow( );
}

inline std::string CountedRows::NextRow( )
{
    std::string counterString(boost::lexical_cast<std::string>(counter_++));
    return counterString + " Fixed row content";
}

template <std::size_t Size>
    template <typename Host>
inline size_t FixedSize<Size>::GetWriteSize(const Host &)
{
    return Size;
}

template <typename Operation>
    template <typename Host>
inline size_t CountedSize<Operation>::GetWriteSize(const Host &host)
{
    return Operation( )(host.GetParam1(host));
}

template <typename Host, typename RowType>
inline void NoStartFiles::FillStartFiles(Files &, const Host &,
                                         RowType &) const
{
}

template <std::size_t Size>
    template <typename Host, typename RowType>
void FixedSizeFile<Size>::FillStartFiles(Files &files, const Host &host,
                                         RowType &currentRow) const
{
    File newFile(CreateFileName(host));

    while (RowType( ) != currentRow &&
           newFile.Size( ) + currentRow.Size( ) <= Size)
    {
        newFile.AddLine(*currentRow);
        ++currentRow;
    }

    files.push_back(newFile);
}

template <typename Operation, typename Next>
    template <typename Host, typename RowType>
void CountedStartFiles<Operation, Next>::
FillStartFiles(Files &files, const Host &host, RowType &currentRow) const
{
    Operation operation;
    const std::size_t MAX_SIZE = operation(host.GetParam1(host));

    File newFile(CreateFileName(host));

    while (RowType( ) != currentRow &&
           newFile.Size( ) + currentRow.Size( ) <= MAX_SIZE)
    {
        newFile.AddLine(*currentRow);
        ++currentRow;
    }

    assert(!newFile.Content( ).empty( ));

    files.push_back(newFile);

    Next next;
    next.FillStartFiles(files, host, currentRow);
}

template <typename T>
Files OneLogOutcome::ExpectedOutcome(T &host)
{
    Files files(CreateStartFiles(host));
    SetupOneStartFileForChecking(files, host);

    for (Row<T> row(host); row != Row<T>( ); ++row)
    {
        files[0].AddLine(*row);
    }

    return files;
}

template <typename T>
Files SizeRestrictedLogOutcome::ExpectedOutcome(T &host)
{
    Files files(CreateStartFiles(host));
    SetupOneStartFileForChecking(files, host);
    File &file = files[0];

    const std::size_t MAX_SIZE = host.GetParam1(host);

    for (Row<T> row(host); row != Row<T>( ); ++row)
    {
        if (FileAndRowSize(file, row) > MAX_SIZE)
        {
            file.CropToSize(MAX_SIZE / 2);
        }

        file.AddLine(*row);
    }

    return files;
}

template <typename T>
Files SizeRestrictedLogsOutcome::ExpectedOutcome(T &host)
{
    Files files(CreateStartFiles(host));

    Row<T> row(host);
    const Row<T> END;

    assert(row != END);

    const std::size_t MAX_SIZE = host.GetParam1(host);
    while (row != END)
    {
        if (files.empty( ) ||
            FileAndRowSize(files.back( ), row) > MAX_SIZE)
        {
            files.push_back(File(CreateFileName(host)));
        }

        File &file = files.back( );

        while (row != END &&
               FileAndRowSize(file, row) <= MAX_SIZE)
        {
            file.AddLine(*row);
            ++row;
        }
    }

    return files;
}

template <std::size_t MaxSize>
    template <typename T>
std::size_t MaxSizeParams<MaxSize>::GetParam1(const T &) const
{
    return MaxSize;
}

inline const boost::filesystem::path NoSubFolder::GetPath( ) const
{
    return ".";
}

const boost::filesystem::path DatedSubFolder::GetPath( ) const
{
    std::time_t timeStamp = std::time(0);
    std::tm *local = std::localtime(&timeStamp);

    std::ostringstream dateStream;
    dateStream << local->tm_year + 1900
               << std::setfill('0')
               << std::setw(2)
               << local->tm_mon + 1
               << std::setfill('0')
               << std::setw(2)
               << local->tm_mday;

    return dateStream.str( );
}

inline const boost::filesystem::path PlainFileNamer::GetName( ) const
{
    return "myrrh.log";
}

const boost::filesystem::path TimestampedFileNamer::GetName( ) const
{
    std::time_t timeStamp = std::time(0);
    std::tm *local = std::localtime(&timeStamp);

    static int uniqueIdentifier = 0;

    std::ostringstream dateStream;
    dateStream << "myrrh"
               << std::setfill('0')
               << std::setw(2)
               << local->tm_hour
               << std::setfill('0')
               << std::setw(2)
               << local->tm_min
               << std::setfill('0')
               << std::setw(2)
               << local->tm_sec
               << '-'
               << std::setfill('0')
               << std::setw(6)
               << 0
               << '-'
               << uniqueIdentifier++
               << ".log";

    return dateStream.str( );
}

template <typename T>
void NormalCase::Test(T &host)
{
    policy::PolicyPtr policy(host.CreatePolicy(host));

    for (Row<T> row(host); row != Row<T>( ); ++row)
    {
        BOOST_CHECK_EQUAL(static_cast<std::size_t>(policy->Write(*row)),
                          row.Size( ));
    }

    Files physicalFiles(GetPhysicalFiles(host.GetPath( ), host));
    Files expectedFiles(host.ExpectedOutcome(host));

    BOOST_REQUIRE_EQUAL(physicalFiles.size( ), expectedFiles.size( ));

    for (std::size_t i = 0; i < physicalFiles.size( ); ++i)
    {
        BOOST_CHECK_EQUAL(expectedFiles[i].Content( ),
                          physicalFiles[i].Content( ));
    }
}

template <typename Creator,
          typename Params,
          typename RowCreator,
          typename FileSizeDeterminer,
          typename StartFileCreator,
          typename ExpectedOutcome,
          typename Subfolder,
          typename Namer,
          typename Case>
void Host<Creator,
          Params,
          RowCreator,
          FileSizeDeterminer,
          StartFileCreator,
          ExpectedOutcome,
          Subfolder,
          Namer,
          Case>::operator( )( )
{
    TestSetup setup(CreateStartFiles(*this));
    Test(*this);
}

// Helper function implementations

template <typename Host>
Files CreateStartFiles(Host &host)
{
    Files files;

    Row<Host, NeverStopper> row(host);
    host.FillStartFiles(files, host, row);

    return files;
}

template <typename Host>
inline boost::filesystem::path CreateFileName(const Host &host)
{
    return host.GetPath( ) / host.GetName( );
}

Files GetPhysicalFiles(const boost::filesystem::path &folder,
                       PlainFileNamer &namer)
{
    BOOST_CHECK(boost::filesystem::exists(folder));
    BOOST_CHECK(boost::filesystem::is_directory(folder));

    return Files(1, File(FileFromHardDisk(folder / namer.GetName( ))));
}

Files GetPhysicalFiles(const boost::filesystem::path &folder,
                       TimestampedFileNamer &namer)
{
    BOOST_CHECK(boost::filesystem::exists(folder));
    BOOST_CHECK(boost::filesystem::is_directory(folder));

    const std::string NAME(namer.GetName( ).string( ));
    const boost::regex FILE_EXPRESSION(
        CreateTimeStampExpression(folder.string( ), NAME));

    Files files;


    using namespace boost::filesystem;
    for (directory_iterator i(folder); directory_iterator( ) != i; ++i)
    {
        if (boost::regex_match(i->path( ).string( ), FILE_EXPRESSION))
        {
            files.push_back(File(FileFromHardDisk(*i)));
        }
    }

    return files;
}

template <typename Host>
void SetupOneStartFileForChecking(Files &files, Host &host)
{
    BOOST_CHECK(files.size( ) < 2);

    if (files.empty( ))
    {
        files.push_back(File(CreateFileName(host)));
    }
    else
    {
        BOOST_CHECK_EQUAL(files[0].Path( ).string( ),
                          CreateFileName(host).string( ));
    }
}

TestSetup::TestSetup(const Files &files)
{
    EraseTestFiles( );
    WriteFiles(files);
}

TestSetup::~TestSetup( )
{
    EraseTestFiles( );
}

void WriteFiles(const Files &files)
{
    for (Files::const_iterator i = files.begin( ); files.end( ) != i; ++i)
    {
        const boost::filesystem::path PATH(i->Path( ));
        boost::filesystem::create_directories(PATH.branch_path( ));
        std::ofstream file(PATH.string( ).c_str( ));
        if (!file.is_open( ))
        {
            BOOST_ERROR("Failed to open " + PATH.string( ));
        }
        file << i->Content( );
        file.flush( );
    }
}

void EraseMatchingFiles(const std::string &expression)
{
    using namespace boost::filesystem;

    const boost::regex EXPRESSION(expression);
    const path DIRECTORY(".");

    for (directory_iterator i(DIRECTORY); directory_iterator( ) != i; ++i)
    {
        if (boost::regex_search(i->path( ).string( ), EXPRESSION))
        {
            remove_all(*i);
        }
    }
}

void EraseTestFiles( )
{
    EraseMatchingFiles("myrrh.*\\.log$");
    EraseMatchingFiles("^(\\..)\\d{4}\\d{2}\\d{2}$");
}

File FileFromHardDisk(const boost::filesystem::path &path)
{
    std::ifstream stream(path.string( ).c_str( ));
    if (!stream.is_open( ))
    {
        BOOST_ERROR("Failed to open " + path.string( ));
    }

    std::string line;
    File file(path);

    while (!std::getline(stream, line).fail( ))
    {
        file.AddLine(line + '\n');
    }

    BOOST_CHECK(stream.eof( ));

    return file;
}

boost::regex CreateTimeStampExpression(const std::string &folderPath,
                                       const std::string &name)
{
    const boost::regex NAME_EXPRESSION("([a-zA-Z]+)(\\d+-\\d+-\\d+)(\\..+)");
    boost::smatch match;
    BOOST_REQUIRE(boost::regex_match(name, match, NAME_EXPRESSION));

    std::string folderString;
    if (!folderPath.empty( ))
    {
        folderString = folderPath + ".";
    }

    const std::string PREFIX(folderString + match[1].str( ));
    const std::string POSTFIX(match[3].str( ));

    return boost::regex(PREFIX + "\\d+-\\d+-\\d+" + POSTFIX);
}

SizeRowStopper::SizeRowStopper( ) :
    written_(0)
{
}

template <typename ContentCreator>
inline bool SizeRowStopper::DoesRowFit(ContentCreator &contentCreator,
                                       const std::string &row)
{
    std::size_t MAX_SIZE = contentCreator.GetWriteSize(contentCreator);
    return (row.size( ) + written_ <= MAX_SIZE);
}

inline void SizeRowStopper::StoreRow(const std::string &row)
{
    written_ += row.size( );
}

template <typename ContentCreator>
inline bool NeverStopper::DoesRowFit(ContentCreator &, const std::string &)
{
    return true;
}

inline void NeverStopper::StoreRow(const std::string &)
{
}

template <typename ContentCreator, typename Stopper>
inline Row<ContentCreator, Stopper>::Row(ContentCreator &contentCreator) :
    contentCreator_(&contentCreator),
    currentRow_(contentCreator_->FirstRow( ) + "\n")
{
}

template <typename ContentCreator, typename Stopper>
inline Row<ContentCreator, Stopper>::Row( ) :
    contentCreator_(0)
{
}

template <typename ContentCreator, typename Stopper>
inline const std::string &Row<ContentCreator, Stopper>::operator*( ) const
{
    assert(contentCreator_);
    return currentRow_;
}

template <typename ContentCreator, typename Stopper>
inline std::size_t Row<ContentCreator, Stopper>::Size( ) const
{
    assert(contentCreator_);
    return currentRow_.size( );
}

template <typename ContentCreator, typename Stopper>
inline
Row<ContentCreator, Stopper> &Row<ContentCreator, Stopper>::operator++( )
{
    assert(contentCreator_);

    const std::string NEW_ROW(contentCreator_->NextRow( ) + "\n");
    assert(!NEW_ROW.empty( ));

    if (stopper_.DoesRowFit(*contentCreator_, NEW_ROW))
    {
        currentRow_ = NEW_ROW;
        stopper_.StoreRow(NEW_ROW);
    }
    else
    {
        *this = Row<ContentCreator, Stopper>( );
    }

    return *this;
}

template <std::size_t Divider>
inline std::size_t DividerOperation<Divider>::operator( )(size_t originalSize)
{
    return originalSize / Divider;
}

inline std::size_t NoOperation::operator( )(size_t originalSize)
{
    return originalSize;
}

template <std::size_t Multiplier>
inline std::size_t MultiplyOperation<Multiplier>::
operator( )(size_t originalSize)
{
    return originalSize * Multiplier;
}

inline size_t AdjustSize(const std::string &text)
{
#ifdef WIN32
    return text.size( ) + std::count(text.begin( ), text.end( ), '\n');
#else
    return text.size( );
#endif
}

template <typename T>
inline size_t FileAndRowSize(const File &file, const T &row)
{
    return file.Size( ) + AdjustSize(*row);
}
