// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Path
 *
 * $Id: TestPath.cpp 356 2007-09-18 19:55:21Z byon $
 */

#include "myrrh/log/policy/Path.hpp"
#include "myrrh/log/policy/PathPart.hpp"
#include "myrrh/log/policy/RestrictionStore.hpp"
#include "myrrh/log/policy/Restriction.hpp"
#include "myrrh/log/policy/Opener.hpp"
#include "myrrh/file/MatchFiles.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_UNINITIALIZED_LOCAL_VARIABLE
#define DISABLE_SIGNED_UNSIGNED_MISMATCH
#include "myrrh/util/Preprocessor.hpp"

#include <boost/lexical_cast.hpp>
#define BOOST_AUTO_TEST_MAIN
#include "boost/test/auto_unit_test.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

#include <sstream>

using namespace myrrh::log::policy;

namespace boost
{
namespace filesystem
{

std::ostream &operator<<(std::ostream &stream, const path &input)
{
    stream << input.string( );
    return stream;
}

bool operator==(const path &input, const std::string &text)
{
    return input.string( ) == text;
}

bool operator==(const std::string &text, const path &input)
{
    return input.string( ) == text;
}

}
}

namespace
{

template <typename T>
void TestAddingPart( );
template <typename T>
void TestAddingPart(T &part);
void ParseTimeString(const std::string &source, int &timeStamp,
                     int &nanoSeconds);
template <size_t SIZE>
int ParseInt(const std::string &source);
template <typename T1, typename T2>
void AddingPartsTogether(const T1 &part1, const T2 &partTwo);
void TestParentPath(const boost::filesystem::path &expected);
void CheckEntityCount(int expected, const Path &path);
void TestAddingString(const std::string &part);

template <bool IS_RESTRICTED>
class RestrictionHardCoded : public myrrh::log::policy::Date
{
public:
    RestrictionHardCoded( );
    virtual void AppendRestrictions(RestrictionStore &store);

    operator PartSum( ) const;

private:

};

}

BOOST_AUTO_TEST_CASE(DefaultConstruction)
{
    BOOST_CHECK_EQUAL(boost::filesystem::path( ), Path( ).Generate( ));
    BOOST_CHECK_EQUAL(boost::filesystem::path( ), Path( ).ParentPath( ));
}

BOOST_AUTO_TEST_CASE(ParentPathPassing)
{
    using namespace boost::filesystem;
    TestParentPath(path( ));
    TestParentPath(path("."));
    TestParentPath(path("folder/"));
    TestParentPath(path("folder/subFolder/"));

    path absolutePath("c:\\folder\\subFolder\\", native);
    TestParentPath(absolutePath);
}

BOOST_AUTO_TEST_CASE(AddingText)
{
    Text nonWhitespace("text");
    TestAddingPart(nonWhitespace);
    Text whitespace("text with some whitespace");
    TestAddingPart(whitespace);
}

BOOST_AUTO_TEST_CASE(AddingPureText)
{
    const std::string NONWHITESPACE("text");
    TestAddingString(NONWHITESPACE);
    const std::string WHITESPACE("text with some whitespace");
    TestAddingString(WHITESPACE);
}

BOOST_AUTO_TEST_CASE(AddingPureTextWithSeveralFolders)
{
    const std::string TEXT("first/second/third/fourth");
    TestAddingString(TEXT);
}

BOOST_AUTO_TEST_CASE(AddingDate)
{
    TestAddingPart<Date>( );
}

BOOST_AUTO_TEST_CASE(AddingTime)
{
    Path path;
    Time timePart;
    path += timePart;
    const std::string PART_RESULT(timePart.Generate( ));
    const std::string PATH_RESULT(path.Generate( ).string( ));

    int partTime = 0;
    int partNanoSeconds = 0;
    ParseTimeString(PART_RESULT, partTime, partNanoSeconds);

    int pathTime = 0;
    int pathNanoSeconds = 0;
    ParseTimeString(PATH_RESULT, pathTime, pathNanoSeconds);

    BOOST_CHECK_EQUAL(pathTime, partTime);
    BOOST_CHECK((pathNanoSeconds - partNanoSeconds) < 20000);
}

BOOST_AUTO_TEST_CASE(AddingPid)
{
    TestAddingPart<ProcessId>( );
}

BOOST_AUTO_TEST_CASE(AddingIndex)
{
    TestAddingPart<Index>( );
}

BOOST_AUTO_TEST_CASE(AddingTextTogether)
{
    AddingPartsTogether(Text("Text1"), Text("Text2"));
}

BOOST_AUTO_TEST_CASE(AddingDateTogether)
{
    AddingPartsTogether(Date( ), Date( ));
}

BOOST_AUTO_TEST_CASE(AddingTimeTogether)
{
    AddingPartsTogether(Time( ), Time( ));
}

BOOST_AUTO_TEST_CASE(AddingIndexTogether)
{
    AddingPartsTogether(Index( ), Index( ));
}

BOOST_AUTO_TEST_CASE(AddingPidTogether)
{
    AddingPartsTogether(ProcessId( ), ProcessId( ));
}

BOOST_AUTO_TEST_CASE(AddingDifferentTypes)
{
    Text text("Just some random text");
    AddingPartsTogether(text, Date( ));
    AddingPartsTogether(text, Time( ));
    AddingPartsTogether(text, Index( ));
    AddingPartsTogether(text, ProcessId( ));

    AddingPartsTogether(Date( ), text);
    AddingPartsTogether(Date( ), Time( ));
    AddingPartsTogether(Date( ), Index( ));
    AddingPartsTogether(Date( ), ProcessId( ));

    AddingPartsTogether(Time( ), text);
    AddingPartsTogether(Time( ), Date( ));
    AddingPartsTogether(Time( ), Index( ));
    AddingPartsTogether(Time( ), ProcessId( ));

    AddingPartsTogether(Index( ), text);
    AddingPartsTogether(Index( ), Date( ));
    AddingPartsTogether(Index( ), Time( ));
    AddingPartsTogether(Index( ), ProcessId( ));

    AddingPartsTogether(ProcessId( ), text);
    AddingPartsTogether(ProcessId( ), Date( ));
    AddingPartsTogether(ProcessId( ), Time( ));
    AddingPartsTogether(ProcessId( ), Index( ));
}

BOOST_AUTO_TEST_CASE(AddingPureTextBeforeOtherType)
{
    Path path;
    path += "someText" + Index( );
    BOOST_CHECK_EQUAL("someText1", path.Generate( ).string( ));
}

BOOST_AUTO_TEST_CASE(AddingPureTextAfterOtherType)
{
    Path path;
    path += Index( ) + "someText";
    BOOST_CHECK_EQUAL("1someText", path.Generate( ).string( ));
}

BOOST_AUTO_TEST_CASE(AddingPureTextBetweenOtherTypes)
{
    Path path;
    path += Index( ) + "someText" + Index( );
    BOOST_CHECK_EQUAL("1someText1", path.Generate( ).string( ));
}

BOOST_AUTO_TEST_CASE(AddingMultipleParts)
{
    ProcessId pid;
    Date date;
    const std::string PID(pid.Generate( ));
    const std::string DATE(date.Generate( ));
    Path path;
    path += Text("Text1") + Text("Text2") + Date( ) + Index( ) + Text("-") +
            pid + Text("Text3");
    BOOST_CHECK_EQUAL("Text1Text2" + DATE + "1-" + PID + "Text3",
                      path.Generate( ).string( ));
}

BOOST_AUTO_TEST_CASE(NoEntities)
{
    Path path;
    BOOST_CHECK(path.BeginEntity( ) == path.EndEntity( ));
}

BOOST_AUTO_TEST_CASE(OneEntity)
{
    const std::string TEXT("Some text");
    Path path;
    path += Text(TEXT);

    BOOST_CHECK_EQUAL(TEXT, path.Generate( ).string( ));
    CheckEntityCount(1, path);
}

BOOST_AUTO_TEST_CASE(OneEntityFromSeveralParts)
{
    Path path;
    path += Text("First") + Text("Second") + Text("Third") + Text("Fourth");

    const std::string EXPECTED("FirstSecondThirdFourth");
    BOOST_CHECK_EQUAL(EXPECTED, path.Generate( ).string( ));

    CheckEntityCount(1, path);
}

BOOST_AUTO_TEST_CASE(AddFolderAsFirstPart)
{
    Path path;

    try
    {
        path += Folder( );
        BOOST_ERROR("Adding folder as first part should throw an exception");
    }
    catch (const Path::Error &e)
    {
        BOOST_CHECK_EQUAL("Folder not allowed as first path part", e.what( ));
    }
}

BOOST_AUTO_TEST_CASE(UnseparatedFolders)
{
    Path path;

    try
    {
        path += Text("Something") + Folder( ) + Folder( ) + Text("Something");
        BOOST_ERROR("Adding two folders without anything in between should "
                    "throw an exception");
    }
    catch (const Path::Error &e)
    {
        BOOST_CHECK_EQUAL("Two unseparated Folders not allowed in path",
                          e.what( ));
    }
}

BOOST_AUTO_TEST_CASE(UnseparatedFoldersThroughSeparateComputations)
{
    Path path;

    try
    {
        path += Text("Something") + Folder( );
        path += Folder( ) + Text("Something");
        BOOST_ERROR("Adding two folders without anything in between should "
                    "throw an exception");
    }
    catch (const Path::Error &e)
    {
        BOOST_CHECK_EQUAL("Two unseparated Folders not allowed in path",
                          e.what( ));
    }
}

BOOST_AUTO_TEST_CASE(UnseparatedFoldersInAString)
{
    Path path;

    try
    {
        path += "Something//Something";
        BOOST_ERROR("Adding two folders without anything in between should "
                    "throw an exception");
    }
    catch (const Path::Error &e)
    {
        BOOST_CHECK_EQUAL("Two unseparated Folders not allowed in path",
                          e.what( ));
    }
}

BOOST_AUTO_TEST_CASE(FolderLastAndFirst)
{
    Path path;

    try
    {
        path += Text("Something")+ Folder( );
        path += Folder( ) + Text("Something");
        BOOST_ERROR("Adding two folders without anything in between should "
                    "throw an exception");
    }
    catch (const Path::Error &e)
    {
        BOOST_CHECK_EQUAL("Two unseparated Folders not allowed in path",
                          e.what( ));
    }
}

BOOST_AUTO_TEST_CASE(TwoEntities)
{
    const std::string TEXT("Some text");
    Path path;
    path += Text(TEXT) + Folder( ) + Text(TEXT);

    BOOST_CHECK_EQUAL(TEXT + "/" + TEXT, path.Generate( ).string( ));

    CheckEntityCount(2, path);
}

BOOST_AUTO_TEST_CASE(SeveralEntities)
{
    const std::string TEXT("Some text");
    Path path;
    path += Text(TEXT) + Folder( ) +
            Text(TEXT) + Folder( ) +
            Text(TEXT) + Folder( ) + 
            Text(TEXT) + Folder( ) + 
            Text(TEXT);

    const std::string EXPECTED(
        TEXT + "/" + TEXT + "/" + TEXT + "/" + TEXT + "/" + TEXT);

    BOOST_CHECK_EQUAL(EXPECTED, path.Generate( ).string( ));

    CheckEntityCount(5, path);
}

BOOST_AUTO_TEST_CASE(SeveralEntitiesFromSeveralParts)
{
    Path path;
    path += Text("First") + Text("Second") + Text("Third") + Folder( ) +
            Text("First") + Text("Second") + Text("Third") + Folder( ) +
            Text("First") + Text("Second") + Text("Third");

    const std::string COMBINED("FirstSecondThird");
    const std::string EXPECTED(COMBINED + "/" + COMBINED + "/" + COMBINED);
    BOOST_CHECK_EQUAL(EXPECTED, path.Generate( ).string( ));

    CheckEntityCount(3, path);
}

BOOST_AUTO_TEST_CASE(MatcherFromOnePart)
{
    Path path;
    path += Text("SomeText");
    BOOST_CHECK(path.BeginEntity( )->Matcher( )("SomeText"));
}

BOOST_AUTO_TEST_CASE(MatcherFromSeveralParts)
{
    Path path;
    path += Text("SomeText") + Text("OtherText") + Index( );
    BOOST_CHECK(path.BeginEntity( )->Matcher( )("SomeTextOtherText1234"));
}

BOOST_AUTO_TEST_CASE(ComparerFromOnePart)
{
    Path path;
    path += Index( );
    Path::Entity::Comparer comparer(path.BeginEntity( )->GetComparer( ));
    BOOST_CHECK(comparer("1234", "123456"));
    BOOST_CHECK(!comparer("1234", "12"));
}

BOOST_AUTO_TEST_CASE(ComparerFromSeveralParts)
{
    Path path;
    path += Text("myrrh-") + Date( ) + Text("-") + Time( ) + Text(".log");
    Path::Entity::Comparer comparer(path.BeginEntity( )->GetComparer( ));
    BOOST_CHECK(!comparer("myrrh-20060112-101010-100100-0.log",
                          "myrrh-20060112-101010-000100-0.log"));
    BOOST_CHECK(comparer("myrrh-20060112-101010-000100-0.log",
                         "myrrh-20060112-101010-100100-0.log"));
    BOOST_CHECK(!comparer("myrrh-20060114-101010-000001-0.log",
                          "myrrh-20060112-101010-000100-0.log"));
    BOOST_CHECK(comparer("myrrh-20060112-101010-000100-0.log",
                         "myrrh-20060114-101010-000001-0.log"));
}

BOOST_AUTO_TEST_CASE(UseComparerForSorting)
{
    Path path;
    path += Text("myrrh-") + Date( ) + Text("-") + Time( ) + Text(".log");
    Path::Entity::Comparer comparer(path.BeginEntity( )->GetComparer( ));

    typedef std::vector<boost::filesystem::path> Paths;
    Paths toSort;
    toSort.push_back("myrrh-20060101-101010-000100-0.log");
    toSort.push_back("myrrh-20060101-101010-000100-0.log");
    toSort.push_back("myrrh-20060101-111011-999998-0.log");
    toSort.push_back("myrrh-20051211-235959-999999-9999999.log");
    toSort.push_back("myrrh-20060101-101011-000101-0.log");
    toSort.push_back("myrrh-20060101-101011-000100-0.log");
    toSort.push_back("myrrh-20060101-101111-000100-0.log");
    toSort.push_back("myrrh-20060101-101011-000101-10.log");
    toSort.push_back("myrrh-20060201-111011-999998-0.log");
    toSort.push_back("myrrh-20060101-101010-000100-0.log");
    toSort.push_back("myrrh-20051211-235959-999999-0.log");
    toSort.push_back("myrrh-20051212-101010-000100-0.log");

    std::sort(toSort.begin( ), toSort.end( ), std::not2(comparer));

    Paths expected;
    expected.push_back("myrrh-20060201-111011-999998-0.log");
    expected.push_back("myrrh-20060101-111011-999998-0.log");
    expected.push_back("myrrh-20060101-101111-000100-0.log");
    expected.push_back("myrrh-20060101-101011-000101-10.log");
    expected.push_back("myrrh-20060101-101011-000101-0.log");
    expected.push_back("myrrh-20060101-101011-000100-0.log");
    expected.push_back("myrrh-20060101-101010-000100-0.log");
    expected.push_back("myrrh-20060101-101010-000100-0.log");
    expected.push_back("myrrh-20060101-101010-000100-0.log");
    expected.push_back("myrrh-20051212-101010-000100-0.log");
    expected.push_back("myrrh-20051211-235959-999999-9999999.log");
    expected.push_back("myrrh-20051211-235959-999999-0.log");

    BOOST_CHECK(expected == toSort);
}

BOOST_AUTO_TEST_CASE(AddingRestriction)
{
    class NoOpener : public myrrh::log::policy::Opener
    {
        boost::filesystem::path DoOpen(std::filebuf &, Path&)
        {
            return boost::filesystem::path( );
        }
    };

    Path path;
    path += RestrictionHardCoded<false>( );

    NoOpener opener;
    FilePtr file(opener.Open(path));

    RestrictionStore store;
    path.AppendRestrictions(store);
    BOOST_CHECK_EQUAL(1, store.Count( ));
    BOOST_CHECK(!store.IsRestricted(*file, 100));

    path += RestrictionHardCoded<true>( );

    RestrictionStore store2;
    path.AppendRestrictions(store2);
    BOOST_CHECK_EQUAL(2, store2.Count( ));
    BOOST_CHECK(store2.IsRestricted(*file, 100));
}

namespace
{

template <typename T>
void TestAddingPart( )
{
    T part;
    TestAddingPart(part);
}

template <typename T>
void TestAddingPart(T &part)
{
    Path path;
    path += part;
    BOOST_CHECK_EQUAL(part.Generate( ), path.Generate( ));
}

void TestAddingString(const std::string &part)
{
    Path path;
    path += part;
    BOOST_CHECK_EQUAL(part, path.Generate( ));
}

template <typename T1, typename T2>
void AddingPartsTogether(const T1 &part1, const T2 &part2)
{
    Path path;
    path += part1 + part2;
    const boost::regex EXPRESSION(part1.GetExpression( ).str( ) +
                                  part2.GetExpression( ).str( ));
    BOOST_CHECK(boost::regex_match(path.Generate( ).string( ), EXPRESSION));
}

void ParseTimeString(const std::string &source, int &timeStamp,
                     int &nanoSeconds)
{
    assert(!source.empty( ));
    std::istringstream stream(source);
    char separator;

    stream >> timeStamp >> separator >> nanoSeconds;

    assert(!stream.fail( ));
    assert('-' == separator);
}

template <size_t SIZE>
int ParseInt(const std::string &source)
{
    assert(source.size( ) == SIZE);
    return boost::lexical_cast<int>(source);
}

void TestParentPath(const boost::filesystem::path &expected)
{
    BOOST_CHECK_EQUAL(expected, Path(expected).Generate( ));
    BOOST_CHECK_EQUAL(expected, Path(expected).ParentPath( ));
}

void CheckEntityCount(int expected, const Path &path)
{
    const Path::EntityIterator::difference_type DISTANCE =
        std::distance(path.BeginEntity( ), path.EndEntity( ));
    BOOST_CHECK_EQUAL(expected, static_cast<int>(DISTANCE));
}

template <bool IS_RESTRICTED>
RestrictionHardCoded<IS_RESTRICTED>::RestrictionHardCoded( )
{
}

template <bool IS_RESTRICTED>
void RestrictionHardCoded<IS_RESTRICTED>::
AppendRestrictions(RestrictionStore &store)
{
    class RestrictionImplementation : public myrrh::log::policy::Restriction
    {
    public:
        virtual bool IsRestricted(const File &, std::size_t) const
        {
            return IS_RESTRICTED;
        }
    };

    RestrictionPtr restriction(new RestrictionImplementation);
    store.Add(restriction);
}

template <bool IS_RESTRICTED>
RestrictionHardCoded<IS_RESTRICTED>::operator PartSum( ) const
{
    PartSum sum;
    sum.Add(*this);
    return sum;
}

}
