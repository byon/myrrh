// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Note: these are not really good tests. They are more like a collection of
// different use scenarios. The results are not tested. Testing happens by
// viewing the output.
/**
 * This file contains tests for myrrh::log::Log and related classes.
 * The tested functionalities include:
 *  - Using Verbosity classes to write c-strings, std::strings, integers,
 *    floats, characters and classes that have std::ostream output stream
 *    operator defined.
 *  - Access to Verbosity class verbosity limit and character id constants
 *  - Writing of (rather) long string. This should be tested with truly long
 *    strings (of several kilobytes)
 *  - Performance tests in comparison to similar implementation with fprintf
 *    and directly to std::ostream
 *  - Concurrency tests with 10 different threads writing at the same time
 *  - A simple deadlock test
 *  - Using customized headers in output lines
 *  - Resetting the default line header
 *  - Changing output threshold level
 *  - Using output target specific verbosity threshold
 *  - Using myrrh::log::DividedLogStream, which divides the output into
 *    separate files based on defined max length
 *  - The different functionality Trace verbosity level based on build
 *    environment; the trace output is written only in debug builds
 *
 * $Id: TestLogger.cpp 286 2007-03-18 15:04:53Z Byon $
 *
 */

#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#define DISABLE_COPY_CONSTRUCTOR_COULD_NOT_BE_GENERATED
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_UNINITIALIZED_LOCAL_VARIABLE
#include "myrrh/util/Preprocessor.hpp"

#include "myrrh/log/DefaultLogPolicy.hpp"
#include "myrrh/log/DividedLogStream.hpp"
#include "myrrh/log/Log.hpp"
#include "myrrh/util/Error.hpp"
#include "myrrh/util/ProgressTimer.hpp"
#include "myrrh/util/Time.hpp"
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>

/**
 * Tests the output of one myrrh::Log::Verbosity log level, The level to be
 * used is defined by template parameter Verbosity.
 * @param value A variable that will be output to the verbosity level.
 */
template <typename Verbosity, typename ValueType>
void TestLogLevel(const ValueType &value)
{
    Verbosity( ) << "Testing " << typeid(Verbosity).name( ) << " with "
                 << typeid(value).name( ) << " " << value
                 << ". Level's threshold is " << Verbosity::VERBOSITY_LIMIT
                 << " and character id " << Verbosity::CHAR_ID;
}

/**
 * Tests long output (several << operator calls after another) of
 * myrrh::Log::Verbosity. The log level of myrrh::Log::Verbosity is determined
 * by template parameter Verbosity.
 * Writes a longish line to the used myrrh::log::Verbosity.
 */
template <typename Verbosity>
void TestLongOutput( )
{
    Verbosity( ) << "This is a very long string indeed with no point at all. "
                    "In matter of fact there is quite distinct difficulty in "
                    "trying to invent any meaningful content to it. It might "
                    "indeed be said there is some paradox in inventing meaning"
                    " to something that by nature is utterly without meaning "
                    "or a point what so ever. "
                 << " This thing then again is much shorter. "
                 << 'T' << 'h' << 'i' << 's' << ' ' << 'i' << 's' << ' ' << 's'
                 << 't' << 'r' << 'i' << 'n' << 'g' << ' ' << 'o' << 'f'
                 << ' ' << 'c' << 'h' << 'a' << 'r' << 'a' << 'c' << 't'
                 << 'e' << 'r' << 's' << '.' << ' '
                 << " Now follows some integers: "
                 << 398475 << " " << 293839 << 94928
                 << " Then some floating numbers "
                 << 2983.3298 << 32983.744 << " " << 2388923.93883
                 << std::string(" And in the end a std::string");
}

/**
 * Writes the given value using all of the pre-determined myrrh::log logger
 * levels.
 * @param value The value to be written using all of the logger levels.
 */
template <typename T>
void TestLoggerWithType(const T &value)
{
    using namespace myrrh::log;

    TestLogLevel<Critical>(value);
    TestLogLevel<Error>(value);
    TestLogLevel<Warn>(value);
    TestLogLevel<Notify>(value);
    TestLogLevel<Info>(value);
    TestLogLevel<Debug>(value);
    TestLogLevel<Trace>(value);
}

/**
 * A dummy class to be used for testing that myrrh::log can be used with any
 * class that implements operator << against std::ostream interface.
 */
class DummyClass
{
public:

    friend const std::ostream &operator <<(std::ostream &stream,
                                           const DummyClass &)
    {
        stream << "This is dummy class output";
        return stream;
    }
};

/**
 * Tests myrrh::log::Logger with various types of output and all of its logger
 * levels.
 */
void TestLogger(int testCount)
{
    TestLoggerWithType(testCount);
    TestLoggerWithType("this is a c string");
    TestLoggerWithType(std::string("This is a std::string"));
    TestLoggerWithType(983.3948983);
    TestLoggerWithType(DummyClass( ));

    TestLongOutput<myrrh::log::Critical>( );
    TestLongOutput<myrrh::log::Error>( );
    TestLongOutput<myrrh::log::Warn>( );
    TestLongOutput<myrrh::log::Notify>( );
    TestLongOutput<myrrh::log::Info>( );
    TestLongOutput<myrrh::log::Debug>( );
    TestLongOutput<myrrh::log::Trace>( );
}

// These global variables are used for testing of printf log output. They are
// global because c-like implementation has no objects and it would be hard(er)
// to implement the functionality by passing of values.

/** Imitates the global verbosity level for printf tests */
myrrh::log::VerbosityLevel currentLevel;
/** The first printf test file */
FILE *file = 0;
/** The second printf test file */
FILE *file2 = 0;
/** The third printf test file */
FILE *file3 = 0;

/**
 * Tests the performance of printf type logging. The function writes n lines
 * (with n being the value of param testCount) to three different files.
 * Each line will hold various types of paramters.
 * @param testCount The count of lines to be written to the files.
 */
void TestPrintfLogging(int testCount)
{
    myrrh::util::ProgressTimer printfTimer("printf logging timer");

    if (!file || !file2 || !file3)
    {
        throw std::runtime_error("Printf file not ready");
    }

    if (currentLevel >= myrrh::log::INFO)
    {
        for (int i = 0; i < testCount; ++i)
        {
            const char * const header = myrrh::util::TimeStampAsCString( );
            fprintf(file, "%s %c %s %d. %s %.3f. %s %s %i.\n", header, '?',
                    "This is test string followed by an integer", i,
                    "Now follows a float", 123.1234,
                    "This is new test string.",
                    "And another with a final integer", i * 2);
            fprintf(file2, "%s %c %s %d. %s %.3f. %s %s %i.\n", header, '?',
                    "This is test string followed by an integer", i,
                    "Now follows a float", 123.1234,
                    "This is new test string.",
                    "And another with a final integer", i * 2);
            fprintf(file3, "%s %c %s %d. %s %.3f. %s %s %i.\n", header, '?',
                    "This is test string followed by an integer", i,
                    "Now follows a float", 123.1234,
                    "This is new test string.",
                    "And another with a final integer", i * 2);

            if (EOF == fflush(file))
            {
                throw std::runtime_error("fprintf flush failed");
            }
        }
    }
}

std::ofstream stream;
std::ofstream stream2;
std::ofstream stream3;

/**
 * Tests the performance of direct std::ofstream logging. The function writes
 * n lines (with n being the value of param testCount) to three different
 * files. Each line will hold various types of paramters.
 * @param testCount The count of lines to be written to the files.
 */
void TestStreamLogging(int testCount)
{
    myrrh::util::ProgressTimer streamTimer("stream logging timer");

    if (currentLevel >= myrrh::log::INFO)
    {
        time_t now;
        tm *nowStamp;
        for (int i = 0; i < testCount; ++i)
        {
            now = time(0);
            nowStamp = localtime(&now);

            std::ostringstream localStream;
            localStream << nowStamp->tm_year + 1900
                        << '.'
                        << nowStamp->tm_mon +1
                        << '.'
                        << nowStamp->tm_mday
                        << ' '
                        << nowStamp->tm_hour
                        << ':'
                        << nowStamp->tm_min
                        << ':'
                        << nowStamp->tm_sec
                        << " ? "
                        << "This is test string followed by an integer "
                        << i
                        << ". Now follows a float "
                        << 123.1234
                        << ". This is new test string. "
                        << "And another with a final integer "
                        << i * 2
                        << std::endl;

            std::string lineString(localStream.str( ));
            stream2 << lineString;
            stream3 << lineString;
        }
    }
}

/**
 * Tests the performance of myrrh logging. The function writesn lines (with
 * n being the value of param testCount) to three different files.
 * Each line will hold various types of paramters.
 * @param testCount The count of lines to be written to the files.
 */
void TestMyrrhLogging(int testCount)
{
    myrrh::util::ProgressTimer myrrhTimer("myrrh logging timer");

    for (int i = 0; i < testCount; ++i)
    {
        myrrh::log::Info( ) << "This is test string followed by an integer "
                            << i
                            << ". Now follows a float "
                            << 123.1234
                            << ". This is new test string. "
                            << "And another with a final integer "
                            << i * 2;
    }
}

/**
 * Opens a file using c fopen
 * @param name Path to the file to open
 * @param how The file opening attributes, see documentation of fopen
 * @throws std::runtime_error if opening fails
 * @return Handle to open file
 */
FILE *openFile(const std::string &name, const std::string &how)
{
    FILE *newFile = fopen(name.c_str( ), how.c_str( ));
    if (!newFile)
    {
        throw std::runtime_error("Failed to open '" + name + "'");
    }
    return newFile;
}

/**
 * Opens a given output file stream to given file path.
 * @param streamToOpen The output file stream to open
 * @param name Path to file, which will be opened to stream
 */
void openStream(std::ofstream &streamToOpen, const std::string &name)
{
    streamToOpen.open(name.c_str( ),
                      std::ios_base::out | std::ios_base::trunc);

    if (!streamToOpen)
    {
        throw std::runtime_error("Failed to open '" + name + "'");
    }
}

/**
 * Tests all verbosity levels of myrrh and does a comparing performance
 * measurement with C fprinf and c++ output streams.
 * @param testCount The count of lines to be written to the files.
 */
void TestAllVerbosityLevels(int testCount)
{
    using namespace myrrh::log;
    Log &log = Log::Instance( );

    file = openFile("printf.log", "w");
    file2 = openFile("printf2.log", "w");
    file3 = openFile("printf3.log", "w");

    openStream(stream, "stream.log");
    openStream(stream2, "stream2.log");
    openStream(stream3, "stream3.log");

    for (int level = TRACE; level >= CRIT; --level)
    {
        currentLevel = static_cast<VerbosityLevel>(level);
        log.SetVerbosity(currentLevel);

        std::cout << "NOW TESTING WITH " << level << std::endl;
        Critical( ) << "NOW TESTING WITH " << level;
        fprintf(file, "NOW TESTING WITH %d\n", level);
        stream << "NOW TESTING WITH " << level << std::endl;

        TestPrintfLogging(testCount);
        TestStreamLogging(testCount);
        TestMyrrhLogging(testCount);
    }
}

/**
 * A functor class that is used for testing concurrent myrrh writing.
 */
class WriterThread
{
public:

    /**
     * Constructor
     * @param testCount The count of lines to be written to the files.
     * @param id Identifies this thread
     */
    explicit WriterThread(int testCount, char id) :
        testCount_(testCount),
        id_(id)
    {
    }

    /**
     * This is the main function for the thread. It writes as many times as
     * specified in constructor a string to all myrrh log levels.
     */
    void operator( )( )
    {
        using namespace myrrh::log;
        for (int i = 0; i < testCount_; ++i)
        {
            Write<Critical>(i);
            Write<Error>(i);
            Write<Warn>(i);
            Write<Info>(i);
            Write<Debug>(i);
            Write<Trace>(i);
        }
    }

private:

    /**
     * Writes a line to a type of template parameter T, which is assumed to be
     * a type of myrrh::log::Verbosity.
     * @param name Name of the verbosity class to be used.
     */
    template <typename T>
    inline void Write(int currentCount)
    {
        T( ) << "Testing " << typeid(T).name( ) << " with thread " << id_
             << ". Current test count is " << currentCount;
    }

    /** The count of times to test log writing in thread */
    const int testCount_;
    /** Identifies the thread */
    const char id_;
};

/**
 * A helper function used to test for deadlock possibility during concurrency
 * tests.
 */
std::string GiveString( )
{
    myrrh::log::Error( ) << "Giving string";

    return "A string";
}

/**
 * Tests if we can arrive at deadlock if the log output takes a function
 * that does more output to the same verbosity level.
 */
void TestDeadLock( )
{
    // This does not cause deadlock because GiveString is performed first.
    myrrh::log::Error( ) << "Testing for deadlock " << GiveString( );

    myrrh::log::Error error;
    error << GiveString( );

    myrrh::log::Info( ) << GiveString( );
}

/**
 * Tests that myrrh::log can be used for concurrent logging
 * @param testCount The count of lines to be written to the files by each
 *                  tested thread
 */
void TestConcurrentLogging(int testCount)
{
    std::cout << "Starting concurrency tests" << std::endl;

    const int THREAD_COUNT = 10;
    boost::thread_group threads;

    myrrh::util::ProgressTimer mainTimer("Concurrency timer");

    for (int i = 0; i < THREAD_COUNT; ++i)
    {
        threads.create_thread(WriterThread(testCount / THREAD_COUNT, 'A' + i));
    }

    TestDeadLock( );

    threads.join_all( );
}

/**
 * A class that implements a customized line header to lines written by
 * myrrh::log. The header implemented by our class contains only the verbosity
 * level id and a whitespace. The class is used only for testing that it
 * truly is possible to customize the line header.
 */
class CustomisedHeader : public myrrh::log::Header
{
    /**
     * Writes customized line header
     * @param stream The stream into which the header should be written
     * @param id Identifies the used verbosity level.
     */
    virtual void Write(std::ostream &stream, char id)
    {
        stream << id << ' ';
    }
};

/**
 * Tests that line header customization for myrrh::log works.
 */
void TestCustomisedHeader( )
{
    using namespace myrrh::log;
    Log::Instance( ).SetHeader(HeaderPtr(new CustomisedHeader( )));

    TestLogger(1);

    Log::Instance( ).SetHeader( );
}

/**
 * Tests that myrrh::log can be used with multiple output targets.
 * @param testCount Count of tests to be carried out
 */
void TestMultipleTargets(int testCount)
{
    using namespace myrrh::log;

    Log &log = Log::Instance( );

    std::ofstream outputFile;
    std::ofstream outputFile2;
    std::ofstream outputFile3;

    openStream(outputFile, "myrrh.log");
    openStream(outputFile2, "myrrh2.log");
    openStream(outputFile3, "myrrh3.log");

    std::cout << "Starting performance tests" << std::endl;

    Log::OutputGuard fileGuard(log.AddOutputTarget(outputFile));
    Log::OutputGuard fileGuard2(log.AddOutputTarget(outputFile2));
    Log::OutputGuard fileGuard3(log.AddOutputTarget(outputFile3));

    TestAllVerbosityLevels(testCount);

    log.SetVerbosity(INFO);
    TestConcurrentLogging(testCount);
}

/**
 * Tests that myrrh::log::DividedLogStream can be used as myrrh::log output
 * target.
 * @param Count of tests
 */
void TestDividedLogging(int testCount)
{
    using namespace myrrh::log;

    const size_t MAX_FILE_SIZE = 1024 * 1024;

    std::cout << "Starting divided logging tests" << std::endl;

    DividedLogStream dividedStream(LogFilePolicyPtr(
        new DefaultLogPolicy(".", "myrrh", ".log", MAX_FILE_SIZE)));

    Log &log = Log::Instance( );

    Log::OutputGuard streamGuard(log.AddOutputTarget(dividedStream));

    for (int level = DEBUG; level >= CRIT; --level)
    {
        currentLevel = static_cast<VerbosityLevel>(level);
        log.SetVerbosity(currentLevel);
        TestMyrrhLogging(testCount);
    }
}

/**
 * Tests the TRACE verbosity level of myrrh::log. The tests should write output
 * only on debug builds. Nothing should be written on release builds.
 * @param testCount Count of tests
 */
void TestTrace(int testCount)
{
    std::cout << "Starting trace tests" << std::endl;

    std::ofstream outputFile;

    openStream(outputFile, "trace.log");

    using namespace myrrh::log;

    Log::OutputGuard fileGuard(Log::Instance( ).AddOutputTarget(outputFile));
    Log::Instance( ).SetVerbosity(TRACE);

    std::string toWrite;

    if (myrrh::util::IsDebugBuild( ))
    {
        toWrite = "Because this is debug build, you should see this line";
    }
    else
    {
        toWrite = "Because this is not debug build, you should not see "
                  "this line";
    }

    myrrh::util::ProgressTimer mainTimer("Trace timer");

    for (int i = 0; i < testCount; ++i)
    {
        Trace( ) << toWrite;
    }
}

/**
 * The true main function for the tests.
 * @param testCount The number of times that tests should be carried out. Note
 *                  that the count is only a suggestion. The tests may limit
 *                  the tests to some maximum count (as is done in case of
 *                  functional tests, which put the output to stdout).
 */
void DoMain(int testCount)
{
    using namespace myrrh::log;

    Log &log = Log::Instance( );
    Log::OutputGuard coutGuard(log.AddOutputTarget(std::cout));

    std::cout << "Starting basic tests" << std::endl;
    const int MAX_FUNC_TEST_COUNT = 50;
    int functionalTestCount = testCount;

    // We want to limit the count of functional tests because the output target
    // is stdout. It will take hell of a time to watch the output required to
    // make true performance tests.
    if (testCount > MAX_FUNC_TEST_COUNT)
    {
        functionalTestCount = MAX_FUNC_TEST_COUNT;
    }

    // Functional tests test that all verbosity levels can be used with a
    // variety of output data.
    for (int i = 0; i < functionalTestCount ; ++i)
    {
        TestLogger(i);
    }

    TestCustomisedHeader( );

    // The stdout is removed as output target, because the rest of the cases
    // are planned for performance test usage. We do not want such output to
    // console.
    coutGuard.Release( );

    // The rest of the test functions add their own output targets.
    TestMultipleTargets(testCount);
    TestDividedLogging(testCount);
    TestTrace(testCount);
}

/**
 * Kicks the tests alive.
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 */
int main(int argc, char *argv[ ])
{
    myrrh::util::ProgressTimer mainTimer("Main program timer");

    const int DEFAULT_TEST_COUNT = 10;
    int testCount = DEFAULT_TEST_COUNT;

    if (argc > 1)
    {
        testCount = boost::lexical_cast<int>(argv[1]);
    }

    return myrrh::util::CatchExceptions(DoMain, testCount, std::cerr);
}
