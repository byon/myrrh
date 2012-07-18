// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for myrrh::log::Log and its nested
 * classes.
 *
 * The following tests are made:
 * -Writing empty message
 * -Writing one very long string
 * -Writing float
 * -Writing int
 * -Writing user defined class
 * -Writing several items of different type
 * -Writing does nothing if global verbosity level is too low
 * -Changing global verbosity level changes the entries that are written
 * -The verbosity level can be queried
 * -Adding an output target with private verbosity
 * -When writing at TRACE level the output comes only at debug build
 * -The writability can be correctly queried
 * -The TimestampHeader is used by default
 * -The default header can be replaced by another header
 * -The default header can be taken back to use.
 * -Writing empty message with empty header
 * -If multiple output targets are selected, the output is similar in all
 * -When output guard is released the guarded target is removed
 * -When output guard object goes out of scope, the output target is removed
 * -Removing all output targets
 * -Adding a non-open stream as output target
 * -Writing does nothing, if there are no output targets
 * -Writing from several threads at the same time.
 * -Writing fails
 * -Use of floating point number presentation manipulators
 *
 * The following situations are not tested:
 * -Setting verbosity level to illegal value (compiler should take care of this
 *  error, it is an user error to bypass the compiler's type checking).
 *
 * $Id: TestLog.cpp 364 2007-09-20 18:00:58Z byon $
 */

#include "myrrh/log/Log.hpp"
#include "myrrh/util/BufferedStream.hpp"
#include "myrrh/file/Temporary.hpp"
#include "myrrh/data/test/Files.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/thread.hpp"
#include "boost/tokenizer.hpp"
#include "boost/regex.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>

using namespace myrrh::log;
using namespace myrrh::file;

typedef boost::shared_ptr<std::ostringstream> OstreamPtr;
typedef std::vector<OstreamPtr> Ostreams;
typedef std::vector<std::string> Strings;
typedef std::vector<Log::OutputGuard> Guards;
typedef boost::tokenizer<boost::char_separator<char> >  Tokenizer;

// Declarations of test case functions
void WritingEmptyMessage( );
void WritingLongString( );
void WritingFloat( );
void WritingInt( );
void WritingUserDefinedClass( );
void WritingSeveralItems( );
void VerbosityLevelTooLow( );
void VerbosityLevelChanges( );
void QueryingVerbosityLevel( );
void WritingWithPrivateVerbosity( );
void WritingTrace( );
void QueryingWritability( );
void ReplacingDefaultHeader( );
void WritingTrulyEmptyMessage( );
void UsingMultipleOutputTargets( );
void OutputGuardScopeTest( );
void OutputGuardManualRelease( );
void RemovingAllOfOutputGuards( );
void UsingNonOpenOutputStream( );
void WritingWithNoOutputTargets( );
void WritingFromSeveralThreads( );
void WritingFails( );
void UseManipulators( );
void WritingFromContendingThreads( );
void SimultaneousWriting( );

// Declarations of helper functions
Guards SetOutputStreams(const Ostreams &streams);
int CurrentVerbosityToInt( );
std::string RemoveTimestampHeader(const std::string &line);
std::string RemoveTimestamp(const std::string &line);
void StreamContainsOnlyOneLine(std::ostringstream &stream,
                               const std::string &expected);
template <typename T>
void WriteThreadTestLine(int currentCount, char id);

class TestCase
{
public:

    TestCase( );

    virtual ~TestCase( );

    virtual void operator( )( );

protected:

    /// @note There is no function for setting verbosity level, because that
    ///       can be done more easily outside of the class.
    virtual void Write( );
    virtual Ostreams GetOutputStreams( );
    virtual void SetHeader( );
    virtual std::string RemoveHeader(const std::string &line);
    virtual void CheckResult(Ostreams &streams);
    virtual std::string GetExpectedLine( );
};

struct StreamInserter
{
    virtual void operator( )(const OstreamPtr &stream);

    Guards GetGuards( ) const;

private:

    Guards guards_;
};

template <typename T>
struct ManyItemsWriter
{
    void operator( )( );
};

struct DummyClass
{
    friend const std::ostream &operator <<(std::ostream &stream,
                                            const DummyClass &)
    {
        stream << "Output of user defined class";
        return stream;
    }
};


class WriterThread
{
public:

    WriterThread(int testCount, char id);

    void operator( )( );

    void CheckResult(const std::string &result);

    WriterThread &operator=(const WriterThread &orig);

private:

    template <typename T>
    void Write(int currentCount);

    int testCount_;
    char id_;
};

class Buffer
{
public:

    explicit Buffer(size_t count);

    void Send(const std::string& toSend);

    std::string Receive( );

private:

    boost::mutex mutex_;
    std::deque<std::string> buffer_;
};

class BufferReader
{
public:

    BufferReader(Buffer& buffer);

    void operator( )( );

private:

    Buffer& buffer_;
};

class BufferWriter
{
public:

    BufferWriter(Buffer& buffer);

    void operator( )( );

private:

    Buffer& buffer_;
};

class NotWorkingBuffer : public myrrh::util::BufferedStream
{
public:
    NotWorkingBuffer( );

protected:
    virtual int SyncImpl( );

private:
    NotWorkingBuffer(const NotWorkingBuffer &);
};

class NotWorkingStream : public std::ostream
{
public:

    NotWorkingStream( );

protected:

    NotWorkingBuffer buffer_;

private:

    NotWorkingStream(const NotWorkingStream &);
    NotWorkingStream &operator=(const NotWorkingStream &);
};

typedef boost::unit_test::test_suite TestSuite;

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for myrrh::Log");

    test->add(BOOST_TEST_CASE(WritingEmptyMessage));
    test->add(BOOST_TEST_CASE(WritingLongString));
    test->add(BOOST_TEST_CASE(WritingFloat));
    test->add(BOOST_TEST_CASE(WritingInt));
    test->add(BOOST_TEST_CASE(WritingUserDefinedClass));
    test->add(BOOST_TEST_CASE(WritingSeveralItems));
    test->add(BOOST_TEST_CASE(VerbosityLevelTooLow));
    test->add(BOOST_TEST_CASE(VerbosityLevelChanges));
    test->add(BOOST_TEST_CASE(QueryingVerbosityLevel));
    test->add(BOOST_TEST_CASE(WritingWithPrivateVerbosity));
    test->add(BOOST_TEST_CASE(WritingTrace));
    test->add(BOOST_TEST_CASE(QueryingWritability));
    test->add(BOOST_TEST_CASE(ReplacingDefaultHeader));
    test->add(BOOST_TEST_CASE(WritingTrulyEmptyMessage));
    test->add(BOOST_TEST_CASE(UsingMultipleOutputTargets));
    test->add(BOOST_TEST_CASE(OutputGuardScopeTest));
    test->add(BOOST_TEST_CASE(OutputGuardManualRelease));
    test->add(BOOST_TEST_CASE(RemovingAllOfOutputGuards));
    test->add(BOOST_TEST_CASE(UsingNonOpenOutputStream));
    test->add(BOOST_TEST_CASE(WritingWithNoOutputTargets));
    test->add(BOOST_TEST_CASE(WritingFromSeveralThreads));
    test->add(BOOST_TEST_CASE(WritingFails));
    test->add(BOOST_TEST_CASE(UseManipulators));
    test->add(BOOST_TEST_CASE(WritingFromContendingThreads));
    test->add(BOOST_TEST_CASE(SimultaneousWriting));

    return test;
}

TestCase::TestCase( )
{
    Log::Instance( ).RemoveAllOutputTargets( );
    Log::Instance( ).SetHeader( );
}

TestCase::~TestCase( )
{
    Log::Instance( ).RemoveAllOutputTargets( );
    Log::Instance( ).SetHeader( );
    Log::Instance( ).SetVerbosity(INFO);
}

void TestCase::operator( )( )
{
    Ostreams streams(GetOutputStreams( ));
    Guards guards(SetOutputStreams(streams));

    SetHeader( );

    Write( );

    CheckResult(streams);
}

void TestCase::Write( )
{
    const std::string text(GetExpectedLine( ));
    Critical( ) << text;
    Error( ) << text;
    Warn( ) << text;
    Notify( ) << text;
    Info( ) << text;
    Debug( ) << text;
    Trace( ) << text;
}

Ostreams TestCase::GetOutputStreams( )
{
    Ostreams streams;
    streams.push_back(OstreamPtr(new std::ostringstream));
    return streams;
}

void TestCase::SetHeader( )
{
    // Just use the default header
}

std::string TestCase::RemoveHeader(const std::string &line)
{
    return RemoveTimestampHeader(line);
}

void TestCase::CheckResult(Ostreams &streams)
{
    const std::string EXPECTED_RESULT(GetExpectedLine( ));

    for (Ostreams::iterator i = streams.begin( ); streams.end( ) != i; ++i)
    {
        boost::char_separator<char> separator("\n");
        const std::string CONTENT((*i)->str( ));
        Tokenizer lines(CONTENT, separator);
        for (Tokenizer::iterator j = lines.begin( ); lines.end( ) != j; ++j)
        {
            BOOST_CHECK_EQUAL(RemoveHeader(*j), EXPECTED_RESULT);
        }
    }
}

std::string TestCase::GetExpectedLine( )
{
    return "Just some text";
}

void StreamInserter::operator( )(const OstreamPtr &stream)
{
    guards_.push_back(Log::Instance( ).AddOutputTarget(*stream));
}

Guards StreamInserter::GetGuards( ) const
{
    return guards_;
}

template <typename T>
void ManyItemsWriter<T>::operator( )( )
{
    T( ) << "First a string followed by int " << 1234
         << " followed by two doubles " << 1234.123 << 12345.12
         << " and finally an user defined object: " << DummyClass( );
}

WriterThread::WriterThread(int testCount, char id) :
    testCount_(testCount),
    id_(id)
{
}

WriterThread &WriterThread::operator=(const WriterThread &orig)
{
    testCount_ = orig.testCount_;
    id_ = orig.id_;
    return *this;
}

void WriterThread::operator( )( )
{
    using namespace myrrh::log;
    for (int i = 0; i < testCount_; ++i)
    {
        Write<Critical>(i);
        Write<Error>(i);
        Write<Warn>(i);
        Write<Notify>(i);
        Write<Info>(i);
        Write<Debug>(i);
    }
}

void WriterThread::CheckResult(const std::string &result)
{
    boost::char_separator<char> separator("\n");
    Tokenizer lines(result, separator);

    int currentIdCount = 0;
    int currentLevelCount = 0;
    for (Tokenizer::iterator i = lines.begin( ); lines.end( ) != i; ++i)
    {
        std::ostringstream stream;
        stream << "Testing .+ with thread " << id_
                << ". Current test count is " << currentIdCount;
        boost::regex expression(stream.str( ));

        if (boost::regex_search(*i, expression))
        {
            ++currentLevelCount;
            // The level count is not 6, because DEBUG level entries do not
            // get written as the verbosity level should be INFO.
            const int LEVEL_COUNT = 5;
            if (currentLevelCount == LEVEL_COUNT)
            {
                ++currentIdCount;
                currentLevelCount = 0;
            }
        }
    }

    BOOST_CHECK_EQUAL(currentIdCount, testCount_);
}

template <typename T>
void WriterThread::Write(int currentCount)
{
    T( ) << "Testing " << typeid(T).name( ) << " with thread " << id_
        << ". Current test count is " << currentCount;
}

NotWorkingBuffer::NotWorkingBuffer( )
{
}

int NotWorkingBuffer::SyncImpl( )
{
    return -1;
}

NotWorkingStream::NotWorkingStream( ) :
    std::ostream(0)
{
    rdbuf(&buffer_);
}

void WritingEmptyMessage( )
{
    class Case : public TestCase
    {
        virtual std::string GetExpectedLine( )
        {
            return "";
        }
    };

    Case( )( );
}

void WritingLongString( )
{
    class Case : public TestCase
    {
        virtual std::string GetExpectedLine( )
        {
            using namespace myrrh::data::test;
            const std::string NAME("../../../../../.." +
                                   Files::SEVERAL_LINES_NOT_EQUAL_LENGTH);
            std::ifstream file(NAME.c_str( ));
            BOOST_REQUIRE(file.is_open( ));

            std::ostringstream stream;
            stream << file.rdbuf( );

            return boost::replace_all_copy(stream.str( ), "\n", " ");
        }
    };

    Case( )( );
}

void WritingFloat( )
{
    class Case : public TestCase
    {
        virtual void Write( )
        {
            const double FLOAT = 0.0123456789;
            Critical( ) << FLOAT;
            Error( ) << FLOAT;
            Warn( ) << FLOAT;
            Notify( ) << FLOAT;
            Info( ) << FLOAT;
            Debug( ) << FLOAT;
            Trace( ) << FLOAT;
        }

        virtual std::string GetExpectedLine( )
        {
            return "0.0123457";
        }
    };

    Case( )( );
}

void WritingInt( )
{
    class Case : public TestCase
    {
        virtual void Write( )
        {
            const int INT= 1234567890;
            Critical( ) << INT;
            Error( ) << INT;
            Warn( ) << INT;
            Notify( ) << INT;
            Info( ) << INT;
            Debug( ) << INT;
            Trace( ) << INT;
        }

        virtual std::string GetExpectedLine( )
        {
            return "1234567890";
        }
    };

    Case( )( );
}

void WritingUserDefinedClass( )
{
    class Case : public TestCase
    {
        virtual void Write( )
        {
            DummyClass dummy;
            Critical( ) << dummy;
            Error( ) << dummy;
            Warn( ) << dummy;
            Notify( ) << dummy;
            Info( ) << dummy;
            Debug( ) << dummy;
            Trace( ) << dummy;
        }

        virtual std::string GetExpectedLine( )
        {
            return "Output of user defined class";
        }
    };

    Case( )( );
}

void WritingSeveralItems( )
{
    class Case : public TestCase
    {
        virtual void Write( )
        {
            ManyItemsWriter<Critical>( )( );
            ManyItemsWriter<Error>( )( );
            ManyItemsWriter<Warn>( )( );
            ManyItemsWriter<Notify>( )( );
            ManyItemsWriter<Info>( )( );
            ManyItemsWriter<Debug>( )( );
            ManyItemsWriter<Trace>( )( );
        }

        virtual std::string GetExpectedLine( )
        {
            return "First a string followed by int 1234 followed by two "
                   "doubles 1234.12312345.12 and finally an user defined "
                   "object: Output of user defined class";
        }
    };
}

void VerbosityLevelTooLow( )
{
    Log::Instance( ).SetVerbosity(ERROR);

    std::ostringstream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));

    Info( ) << "This should not cause output";

    BOOST_CHECK_EQUAL("", stream.str( ));

    Log::Instance( ).SetVerbosity(INFO);
}

void VerbosityLevelChanges( )
{
    Log::Instance( ).SetVerbosity(ERROR);

    std::ostringstream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));

    Info( ) << "This should not cause output";

    BOOST_CHECK_EQUAL("", stream.str( ));

    Log::Instance( ).SetVerbosity(INFO);

    Info( ) << "This should cause output";

    BOOST_CHECK(stream.str( ).size( ) > 0);

    Log::Instance( ).SetVerbosity(INFO);
}

void QueryingVerbosityLevel( )
{
    for (int i = CRIT; i <= TRACE; ++i)
    {
        const VerbosityLevel LEVEL = static_cast<VerbosityLevel>(i);
        Log::Instance( ).SetVerbosity(LEVEL);
        BOOST_CHECK_EQUAL(LEVEL, Log::Instance( ).GetVerbosity( ));
    }
    Log::Instance( ).SetVerbosity(INFO);
}

void WritingWithPrivateVerbosity( )
{
    Log::Instance( ).SetVerbosity(INFO);

    std::ostringstream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));
    std::ostringstream stream2;
    Log::OutputGuard guard2(Log::Instance( ).AddOutputTarget(stream2, ERROR));

    Info( ) << "This should cause output only to first stream";

    BOOST_CHECK(stream.str( ).size( ) > 0);
    BOOST_CHECK_EQUAL("", stream2.str( ));

    stream.str("");
    stream2.str("");

    Error( ) << "This should cause output to both streams";

    BOOST_CHECK(stream.str( ).size( ) > 0);
    BOOST_CHECK(stream2.str( ).size( ) > 0);
}

void WritingTrace( )
{
    Log::Instance( ).SetVerbosity(TRACE);
    std::ostringstream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));

    Trace( ) << "This should only be printed on debug build";

#ifdef NDEBUG
    BOOST_CHECK_EQUAL("", stream.str( ));
#else
    BOOST_CHECK(stream.str( ).size( ) > 0);
#endif

    Log::Instance( ).SetVerbosity(INFO);
}

void QueryingWritability( )
{
    for (int i = CRIT; i <= TRACE; ++i)
    {
        const VerbosityLevel LEVEL = static_cast<VerbosityLevel>(i);
        Log::Instance( ).SetVerbosity(LEVEL);

        for (int k = CRIT; k <= i; ++k)
        {
            const VerbosityLevel TEST = static_cast<VerbosityLevel>(k);
            BOOST_CHECK_EQUAL(true, Log::Instance( ).IsWritable(TEST));
        }

        for (int j = i + 1; j <= TRACE; ++j)
        {
            const VerbosityLevel TEST = static_cast<VerbosityLevel>(j);
            BOOST_CHECK_EQUAL(false, Log::Instance( ).IsWritable(TEST));
        }
    }

    Log::Instance( ).SetVerbosity(INFO);
}

void ReplacingDefaultHeader( )
{
    struct AddStringHeader : public Header
    {
        virtual void Write(std::ostream &stream, char)
        {
            stream << "Hubbadeijaa";
        }
    };

    class Case : public TestCase
    {
        virtual void SetHeader( )
        {
            HeaderPtr newHeader(new AddStringHeader);
            Log::Instance( ).SetHeader(newHeader);
        }

        virtual std::string RemoveHeader(const std::string &line)
        {
            const std::string EXPECTED("Hubbadeijaa");
            BOOST_CHECK_EQUAL(EXPECTED, line.substr(0, EXPECTED.size( )));

            return line.substr(EXPECTED.size( ));
        }
    };

    // First test with the AddStringHeader
    Case( )( );
    // Then test the normal test case so we know the default header is replaced
    TestCase( )( );
}

void WritingTrulyEmptyMessage( )
{
    struct DoNothingHeader : public Header
    {
        virtual void Write(std::ostream &, char)
        {
        }
    };

    class Case : public TestCase
    {
        virtual void SetHeader( )
        {
            HeaderPtr newHeader(new DoNothingHeader);
            Log::Instance( ).SetHeader(newHeader);
        }

        virtual std::string RemoveHeader(const std::string &line)
        {
            return line;
        }

        virtual std::string GetExpectedLine( )
        {
            return "";
        }
    };

    Case( )( );
}

void UsingMultipleOutputTargets( )
{
    class Case : public TestCase
    {
        virtual Ostreams GetOutputStreams( )
        {
            Ostreams result(TestCase::GetOutputStreams( ));
            Ostreams tmp(TestCase::GetOutputStreams( ));
            std::copy(tmp.begin( ), tmp.end( ), std::back_inserter(result));
            TestCase::GetOutputStreams( ).swap(tmp);
            std::copy(tmp.begin( ), tmp.end( ), std::back_inserter(result));
            TestCase::GetOutputStreams( ).swap(tmp);
            std::copy(tmp.begin( ), tmp.end( ), std::back_inserter(result));

            typedef unsigned int uint;
            const uint SIZE = static_cast<uint>(result.size( ));
            const uint EXPECTED = 4;
            BOOST_CHECK_EQUAL(SIZE, EXPECTED);

            return result;
        }
    };

    return Case( )( );
}

void OutputGuardScopeTest( )
{
    std::ostringstream stream;
    {
        Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));
        Notify( ) << "This goes to output";
    }

    Notify( ) << "This does not go to output";

    StreamContainsOnlyOneLine(stream, "This goes to output\n");
}

void OutputGuardManualRelease( )
{
    std::ostringstream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));

    Notify( ) << "This goes to output";
    guard.Release( );
    Notify( ) << "This does not go to output";

    StreamContainsOnlyOneLine(stream, "This goes to output\n");
}

void RemovingAllOfOutputGuards( )
{
    std::ostringstream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));
    std::ostringstream stream2;
    Log::OutputGuard guard2(Log::Instance( ).AddOutputTarget(stream2));
    std::ostringstream stream3;
    Log::OutputGuard guard3(Log::Instance( ).AddOutputTarget(stream3));

    Notify( ) << "This goes to output";
    Log::Instance( ).RemoveAllOutputTargets( );
    Notify( ) << "This does not go to output";

    StreamContainsOnlyOneLine(stream, "This goes to output\n");
    StreamContainsOnlyOneLine(stream2, "This goes to output\n");
    StreamContainsOnlyOneLine(stream3, "This goes to output\n");
}

void UsingNonOpenOutputStream( )
{
    std::ofstream file;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(file));
    Notify( ) << "A piece of string";
}

void WritingWithNoOutputTargets( )
{
    Log::Instance( ).RemoveAllOutputTargets( );
    Notify( ) << 1234 << " is an integer in a log entry that goes nowhere";
}

void WritingFromSeveralThreads( )
{
    const std::string FILE_NAME("Output.txt");
    Temporary fileDeleter(FILE_NAME);
    Log::OutputGuard guard(
        Log::Instance( ).AddOutputTarget(fileDeleter.Stream( )));

    const int THREAD_COUNT = 10;
    boost::thread_group threadGroup;
    typedef std::vector<WriterThread> Writers;
    Writers writers;

    for (int i = 0; i < THREAD_COUNT; ++i)
    {
        writers.push_back(WriterThread(10, 'A' + static_cast<char>(i)));
        threadGroup.create_thread(writers[i]);
    }

    threadGroup.join_all( );

    std::ifstream inputFile(FILE_NAME.c_str( ));
    BOOST_REQUIRE(inputFile.is_open( ));
    std::ostringstream stream;
    stream << inputFile.rdbuf( );

    for (Writers::iterator i = writers.begin( ); writers.end( ) != i; ++i)
    {
        i->CheckResult(stream.str( ));
    }
}

void WritingFails( )
{
    // The test hear is that the program continues to function normally even
    // if writing fails.
    NotWorkingStream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));
    Notify( ) << "Put some strings and some integers" << 1234 << 4321;
}

void UseManipulators( )
{
    class Case : public TestCase
    {
    public:
        Case(double value, const std::string expected) :
            VALUE_(value),
            expected_(expected)
        {
        }
        virtual void Write( )
        {
            using namespace std;
            Critical( ) << fixed << VALUE_ << " " << scientific << VALUE_;
            Error( ) << fixed << VALUE_ << " " << scientific << VALUE_;
            Warn( ) << fixed << VALUE_ << " " << scientific << VALUE_;
            Notify( ) << fixed << VALUE_ << " " << scientific << VALUE_;
            Info( ) << fixed << VALUE_ << " " << scientific << VALUE_;
            Debug( ) << fixed << VALUE_ << " " << scientific << VALUE_;
            Trace( ) << fixed << VALUE_ << " " << scientific << VALUE_;
        }

        virtual std::string GetExpectedLine( )
        {
            return expected_;
        }

        const double VALUE_;
        std::string expected_;
    };

    std::ostringstream expected;
    const double VALUE = 1234.12345678;
    expected << std::fixed << VALUE << " " << std::scientific << VALUE;

    Case(VALUE, expected.str( ))( );
}

void WritingFromContendingThreads( )
{
    std::ostringstream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));

    Buffer buffer(5);
    boost::thread_group threads;
    threads.create_thread(BufferReader(buffer));
    threads.create_thread(BufferWriter(buffer));

    threads.join_all( );
}

/**
 * @note There is fear that this test will fail in other environments. For the
 *       the test to succeed the compiler must first call the function and only
 *       then instantiate the Verbosity object. Test this in other
 *       environments.
 */
void SimultaneousWriting( )
{
    std::ostringstream stream;
    Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(stream));

    struct WriteLogAndReturnString
    {
        std::string operator( )( )
        {
            myrrh::log::Error( ) << "String from function";
            return "Returned string";
        }
    };

    WriteLogAndReturnString functor;
    myrrh::log::Critical( ) << "Before " << functor( ) << " After";

    boost::char_separator<char> separator("\n");
    const std::string CONTENT(stream.str( ));
    Tokenizer lines(CONTENT, separator);
    Tokenizer::iterator i = lines.begin( );
    BOOST_CHECK_EQUAL(RemoveTimestamp(*i), " E String from function");
    ++i;
    BOOST_CHECK_EQUAL(RemoveTimestamp(*i),
                      " C Before Returned string After");
}

Guards SetOutputStreams(const Ostreams &streams)
{
    return std::for_each(streams.begin( ), streams.end( ),
                         StreamInserter( )).GetGuards( );
}

int CurrentVerbosityToInt( )
{
    return static_cast<int>(Log::Instance( ).GetVerbosity( ));
}

std::string RemoveTimestampHeader(const std::string &line)
{
    std::string result(RemoveTimestamp(line));
    const std::string ID(" I ");
    BOOST_CHECK(result.size( ) >= ID.size( ));

    return result.substr(ID.size( ));
}

std::string RemoveTimestamp(const std::string &line)
{
    const std::string TIMESTAMP("1234.12.12 12:12:12:123");
    BOOST_CHECK(line.size( ) >= TIMESTAMP.size( ));

    return line.substr(TIMESTAMP.size( ));
}

void StreamContainsOnlyOneLine(std::ostringstream &stream,
                               const std::string &expected)
{
    const std::string &RESULT = stream.str( );
    const int LINE_COUNT =
        static_cast<int>(std::count(RESULT.begin( ), RESULT.end( ), '\n'));
    BOOST_CHECK_EQUAL(LINE_COUNT, 1);
    BOOST_CHECK_EQUAL(RemoveTimestampHeader(RESULT), expected);
}

Buffer::Buffer(size_t count) :
    buffer_(count)
{
}

void Buffer::Send(const std::string& toSend)
{
    myrrh::log::Critical( ) << "Just something";
    boost::mutex::scoped_lock lock(mutex_);
    myrrh::log::Critical( ) << "Just something";
    buffer_.push_back(toSend);
}

std::string Buffer::Receive( )
{
    boost::mutex::scoped_lock lock(mutex_);

    if (buffer_.empty( ))
    {
        return "";
    }

    std::string result(buffer_.front( ));
    buffer_.pop_front( );

    myrrh::log::Critical( ) << "Received " << result;

    return result;
}

BufferReader::BufferReader(Buffer& buffer) :
    buffer_(buffer)
{
}

void BufferReader::operator( )( )
{
    for (size_t i = 0; i < 10000; ++i)
    {
        myrrh::log::Critical( ) << buffer_.Receive( );
    }
}

BufferWriter::BufferWriter(Buffer& buffer) :
    buffer_(buffer)
{
}

void BufferWriter::operator( )( )
{
    for (size_t i = 0; i < 10000; ++i)
    {
        buffer_.Send("String written to buffer");
    }
}
