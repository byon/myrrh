// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for PathPart
 *
 * $Id: TestPathPart.cpp 366 2007-09-20 18:23:05Z byon $
 */

#include "myrrh/log/policy/PathPart.hpp"
#include "myrrh/log/policy/RestrictionStore.hpp"

#include <ctime>
#include <iomanip>

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_COPY_CONSTRUCTOR_COULD_NOT_BE_GENERATED
#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#include "myrrh/util/Preprocessor.hpp"

#include "boost/lexical_cast.hpp"
#include "boost/test/unit_test.hpp"
#include "boost/filesystem/path.hpp"

#ifdef WIN32

#include <process.h>
#pragma warning(pop)
#pragma warning(disable: 4996)

#endif

typedef boost::unit_test::test_suite TestSuite;
using namespace myrrh::log::policy;

// Declarations of helper functions

namespace
{

std::string DateString( );
std::string GetProcessId( );
std::string GenerateFromSum(const PartSum &sum);
boost::regex ExpressionFromSum(const PartSum &sum);
void TestTextSum(const std::string &text1, const std::string &text2);
template <typename T>
void IsFirstLater(const T &part, const std::string &later,
             const std::string &earlier);

}

// Declarations of tester functions

void TestTextPathPart( );
void TestStringToPartSum( );
void TestFolderStringToPartSum( );
void TestSeveralFolderStringsToPartSum( );
void TestStringBeforeFolderToPartSum( );
void TestStringAfterFolderToPartSum( );
void TestStringSurroundedByFoldersToPartSum( );
void TestStringBothSidesOfFolderToPartSum( );
void TestSeveralLegalFoldersToPartSum( );
void TestFolderPathPart( );
void TestDatePathPart( );
void TestTimePathPart( );
void TestTimeGenerationIsAlwaysUnique( );
void TestIndexPathPart( );
void TestPidPathPart( );
void TestAddingPartsTogether( );
void TestDotInText( );

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for PathPart");
    test->add(BOOST_TEST_CASE(TestTextPathPart));
    test->add(BOOST_TEST_CASE(TestStringToPartSum));
    test->add(BOOST_TEST_CASE(TestFolderPathPart));
    test->add(BOOST_TEST_CASE(TestSeveralFolderStringsToPartSum));
    test->add(BOOST_TEST_CASE(TestStringBeforeFolderToPartSum));
    test->add(BOOST_TEST_CASE(TestStringAfterFolderToPartSum));
    test->add(BOOST_TEST_CASE(TestStringBothSidesOfFolderToPartSum));
    test->add(BOOST_TEST_CASE(TestSeveralLegalFoldersToPartSum));
    test->add(BOOST_TEST_CASE(TestStringSurroundedByFoldersToPartSum));
    test->add(BOOST_TEST_CASE(TestDatePathPart));
    test->add(BOOST_TEST_CASE(TestTimePathPart));
    test->add(BOOST_TEST_CASE(TestTimeGenerationIsAlwaysUnique));
    test->add(BOOST_TEST_CASE(TestIndexPathPart));
    test->add(BOOST_TEST_CASE(TestPidPathPart));
    test->add(BOOST_TEST_CASE(TestAddingPartsTogether));
    test->add(BOOST_TEST_CASE(TestDotInText));
    test->add(BOOST_TEST_CASE(TestFolderStringToPartSum));
    return test;
}

// Implementations of tester functions

void TestTextPathPart( )
{
    const std::string TEXT("SomeText");
    Text text(TEXT);
    BOOST_CHECK_EQUAL(TEXT, text.Generate( ));
    BOOST_CHECK_EQUAL(TEXT, text.GetExpression( ).str( ));

    BOOST_CHECK(boost::regex_match(TEXT, text.GetExpression( )));

    Text text2(text);
    RestrictionStore store;
    text.AppendRestrictions(store);
    text2.AppendRestrictions(store);
    BOOST_CHECK(!store.Count( ));

    BOOST_CHECK_THROW(Text("/"), Text::IllegalText);
    BOOST_CHECK_THROW(Text("/text"), Text::IllegalText);
    BOOST_CHECK_THROW(Text("text/"), Text::IllegalText);
    BOOST_CHECK_THROW(Text("text/text"), Text::IllegalText);
    BOOST_CHECK_THROW(Text("\\"), Text::IllegalText);
    BOOST_CHECK_THROW(Text("\\text"), Text::IllegalText);
    BOOST_CHECK_THROW(Text("text\\"), Text::IllegalText);
    BOOST_CHECK_THROW(Text("text\\text"), Text::IllegalText);

    BOOST_CHECK(!text.IsFirstEarlier(TEXT, TEXT));
}

void TestStringToPartSum( )
{
    PartSum sum;
    sum += "text";

    PartStore parts(sum.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(1), parts.size( ));
    BOOST_CHECK_EQUAL("text", parts.front( )->Generate( ));
}

void TestFolderStringToPartSum( )
{
    PartSum sum;
    sum += "/";
    PartStore parts(sum.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(1), parts.size( ));
    BOOST_CHECK_EQUAL("/", parts.front( )->Generate( ));
    BOOST_CHECK_EQUAL("/", parts.back( )->Generate( ));
    PartSum sum2;
    sum2 += "\\";
    PartStore parts2(sum2.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(1), parts2.size( ));
    BOOST_CHECK_EQUAL("/", parts2.front( )->Generate( ));
    BOOST_CHECK_EQUAL("/", parts2.back( )->Generate( ));
}

void TestSeveralFolderStringsToPartSum( )
{
    PartSum sum;
    sum += "/////";
    PartStore parts(sum.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(5), parts.size( ));
    BOOST_CHECK_EQUAL("/", parts.front( )->Generate( ));
}

void TestStringBeforeFolderToPartSum( )
{
    PartSum sum;
    sum += "text/";

    PartStore parts(sum.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(2), parts.size( ));
    BOOST_CHECK_EQUAL("text", parts.front( )->Generate( ));
    BOOST_CHECK_EQUAL("/", parts.back( )->Generate( ));
}

void TestStringAfterFolderToPartSum( )
{
    PartSum sum;
    sum += "/text";

    PartStore parts(sum.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(2), parts.size( ));
    BOOST_CHECK_EQUAL("/", parts.front( )->Generate( ));
    BOOST_CHECK_EQUAL("text", parts.back( )->Generate( ));
}

void TestStringSurroundedByFoldersToPartSum( )
{
    PartSum sum;
    sum += "/text/";

    PartStore parts(sum.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(3), parts.size( ));
    BOOST_CHECK_EQUAL("/", parts[0]->Generate( ));
    BOOST_CHECK_EQUAL("text", parts[1]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[2]->Generate( ));
}

void TestStringBothSidesOfFolderToPartSum( )
{
    PartSum sum;
    sum += "first/second";

    PartStore parts(sum.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(3), parts.size( ));
    BOOST_CHECK_EQUAL("first", parts[0]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[1]->Generate( ));
    BOOST_CHECK_EQUAL("second", parts[2]->Generate( ));
}

void TestSeveralLegalFoldersToPartSum( )
{
    PartSum sum;
    sum += "first/second/third/fourth/fifth/sixth/seventh/eight/ninth/tenth/";

    PartStore parts(sum.Parts( ));
    BOOST_CHECK_EQUAL(std::size_t(20), parts.size( ));
    BOOST_CHECK_EQUAL("first", parts[0]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[1]->Generate( ));
    BOOST_CHECK_EQUAL("second", parts[2]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[3]->Generate( ));
    BOOST_CHECK_EQUAL("third", parts[4]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[5]->Generate( ));
    BOOST_CHECK_EQUAL("fourth", parts[6]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[7]->Generate( ));
    BOOST_CHECK_EQUAL("fifth", parts[8]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[9]->Generate( ));
    BOOST_CHECK_EQUAL("sixth", parts[10]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[11]->Generate( ));
    BOOST_CHECK_EQUAL("seventh", parts[12]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[13]->Generate( ));
    BOOST_CHECK_EQUAL("eight", parts[14]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[15]->Generate( ));
    BOOST_CHECK_EQUAL("ninth", parts[16]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[17]->Generate( ));
    BOOST_CHECK_EQUAL("tenth", parts[18]->Generate( ));
    BOOST_CHECK_EQUAL("/", parts[19]->Generate( ));
}

void TestFolderPathPart( )
{
    const std::string SEPARATOR("/");
    Folder folder;
    BOOST_CHECK_EQUAL(SEPARATOR, folder.Generate( ));
    BOOST_CHECK_EQUAL(SEPARATOR, folder.GetExpression( ).str( ));

    Folder folder2;
    RestrictionStore store;
    folder.AppendRestrictions(store);
    folder2.AppendRestrictions(store);
    BOOST_CHECK(!store.Count( ));

    BOOST_CHECK(!folder.IsFirstEarlier(SEPARATOR, SEPARATOR));
}

void TestDatePathPart( )
{
    const std::string DATE(DateString( ));
    const std::string EXPRESSION(
        "\\d{4}(0[1-9]|1[0-2])(0[1-9]|[12]\\d|3[01])");
    Date date;
    BOOST_CHECK_EQUAL(DATE, date.Generate( ));
    BOOST_CHECK_EQUAL(EXPRESSION, date.GetExpression( ).str( ));

    BOOST_CHECK(boost::regex_match(DATE, date.GetExpression( )));

    // Note that the test for the appended restrictions is not really proper.
    // It is only testing that restrictions are added, not their type. This is
    // not done, because the test would be too burdensome.
    Date date2(date);
    RestrictionStore store;
    date.AppendRestrictions(store);
    date2.AppendRestrictions(store);

    const unsigned int EXPECTED_SIZE = 2;
    BOOST_CHECK_EQUAL(EXPECTED_SIZE, store.Count( ));

    IsFirstLater(date, "20060114", "20060112");
    IsFirstLater(date, "20060112", "20060106");
    IsFirstLater(date, "20060228", "20060128");
    IsFirstLater(date, "20060128", "20050109");
    IsFirstLater(date, "10010101", "10000101");
    IsFirstLater(date, "10010101", "10000130");
    IsFirstLater(date, "10010101", "10001230");

    BOOST_CHECK(!date.IsFirstEarlier("20060109", "20060109"));
}

void TestTimePathPart( )
{
    // The generated string is not checked exactly, because it contains a
    // timestamp that could be too precise to check. It is trusted that if the
    // regular expression matches to the timestamp, it is ok.
    const std::string EXPRESSION(
        "([01]\\d|2[0-3])([0-5]\\d)([0-5]\\d)-\\d{6}-\\d+");
    Time timePart;

    const std::string TIME(timePart.Generate( ));
    BOOST_CHECK_EQUAL(EXPRESSION, timePart.GetExpression( ).str( ));

    BOOST_CHECK(boost::regex_match(TIME, timePart.GetExpression( )));

    Time timePart2(timePart);
    RestrictionStore store;
    timePart.AppendRestrictions(store);
    timePart2.AppendRestrictions(store);
    BOOST_CHECK(!store.Count( ));

    IsFirstLater(timePart, "101010-000100-0", "101010-000099-0");
    IsFirstLater(timePart, "101010-000000-0", "101001-000000-0");
    IsFirstLater(timePart, "101010-000000-0", "100210-000000-0");
    IsFirstLater(timePart, "101010-000000-0", "051010-000000-0");
    IsFirstLater(timePart, "101010-000000-0", "051010-999999-0");
    IsFirstLater(timePart, "101010-000000-0", "051059-999999-0");
    IsFirstLater(timePart, "101010-000000-0", "055959-999999-0");
    IsFirstLater(timePart, "101010-000100-1", "101010-000100-0");
    IsFirstLater(timePart, "101010-000100-10", "101010-000100-0");
    IsFirstLater(timePart, "101010-000100-99", "101010-000100-0");

    BOOST_CHECK(!timePart.IsFirstEarlier("101010-000100-0",
                                         "101010-000100-0"));
}

void TestTimeGenerationIsAlwaysUnique( )
{
    Time timePart;
    std::string previous;
    for (int i = 0; i < 10000; ++i)
    {
        const std::string TIME(timePart.Generate( ));
        BOOST_CHECK(TIME != previous);
        previous = TIME;
    }
}

void TestIndexPathPart( )
{
    Index index;
    for (int i = 1; i < 10; ++i)
    {
        const std::string INDEX(boost::lexical_cast<std::string>(i));
        BOOST_CHECK_EQUAL(INDEX, index.Generate( ));
        BOOST_CHECK_EQUAL("\\d+", index.GetExpression( ).str( ));

        BOOST_CHECK(boost::regex_match(INDEX, index.GetExpression( )));
    }

    Index index2(index);
    RestrictionStore store;
    index.AppendRestrictions(store);
    index2.AppendRestrictions(store);
    BOOST_CHECK(!store.Count( ));

    IsFirstLater(index, "2", "1");
    IsFirstLater(index, "20", "1");
    IsFirstLater(index, "200020100", "200020099");
}

void TestPidPathPart( )
{
    const std::string PID(GetProcessId( ));
    ProcessId pid;
    BOOST_CHECK_EQUAL(PID, pid.Generate( ));
    BOOST_CHECK_EQUAL(PID, pid.GetExpression( ).str( ));

    BOOST_CHECK(boost::regex_match(PID, pid.GetExpression( )));

    ProcessId pid2(pid);
    RestrictionStore store;
    pid.AppendRestrictions(store);
    pid2.AppendRestrictions(store);
    BOOST_CHECK(!store.Count( ));

    BOOST_CHECK(!pid.IsFirstEarlier(PID, PID));
}

void TestAddingPartsTogether( )
{
    const std::string TEXT1("Text1");
    const std::string TEXT2("Text2");
    TestTextSum(TEXT1, TEXT2);
    TestTextSum(TEXT2, TEXT1);
}

void TestDotInText( )
{
    const std::string TEXT("Some.thing");
    Text text(TEXT);
    BOOST_CHECK_EQUAL(TEXT, text.Generate( ));
    BOOST_CHECK(!boost::regex_match("Some+thing", text.GetExpression( )));
    BOOST_CHECK(!boost::regex_match("Sometthing", text.GetExpression( )));
    BOOST_CHECK(boost::regex_match(TEXT, text.GetExpression( )));
}

namespace
{

std::string DateString( )
{
    std::time_t now = std::time(0);
    std::tm *localNow = std::localtime(&now);

    std::ostringstream stream;

    stream << 1900 + localNow->tm_year
           << std::setfill('0')
           << std::setw(2)
           << localNow->tm_mon + 1
           << std::setfill('0')
           << std::setw(2)
           << localNow->tm_mday;

    return stream.str( );
}

std::string GetProcessId( )
{
#ifdef WIN32
    const int PID(_getpid( ));
#else
    const int PID(getpid( ));
#endif

    return boost::lexical_cast<std::string>(PID);
}

std::string GenerateFromSum(const PartSum &sum)
{
    std::string result;
    const PartStore &store = sum.Parts( );
    for (PartStore::const_iterator i(store.begin( )); store.end( ) != i; ++i)
    {
        PathPartPtr current(*i);
        result += current->Generate( );
    }

    return result;
}

boost::regex ExpressionFromSum(const PartSum &sum)
{
    std::string result;
    const PartStore &store = sum.Parts( );
    for (PartStore::const_iterator i(store.begin( )); store.end( ) != i; ++i)
    {
        PathPartPtr current(*i);
        result += current->GetExpression( ).str( );
    }

    return boost::regex(result);
}

void TestTextSum(const std::string &text1, const std::string &text2)
{
    PartSum sum(Text(text1) + Text(text2));

    const std::string ADDED_TEXTS(text1 + text2);
    BOOST_CHECK_EQUAL(ADDED_TEXTS, GenerateFromSum(sum));

    const boost::regex EXPRESSION(ExpressionFromSum(sum));
    BOOST_CHECK_EQUAL(ADDED_TEXTS, EXPRESSION.str( ));

    BOOST_CHECK(boost::regex_match(ADDED_TEXTS, EXPRESSION));
}

template <typename T>
inline void IsFirstLater(const T &part, const std::string &later,
                           const std::string &earlier)
{
    if (part.IsFirstEarlier(later, earlier))
    {
        BOOST_ERROR(later + " should be earlier than " + earlier);
    }

    if (!part.IsFirstEarlier(earlier, later))
    {
        BOOST_ERROR(earlier + " should be later than " + later);
    }
}

}
