// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for MatchFiles
 *
 * $Id: TestMatchFiles.cpp 332 2007-06-17 11:27:49Z byon $
 */

#include "myrrh/file/MatchFiles.hpp"
#include "myrrh/file/Eraser.hpp"
#include "myrrh/file/Temporary.hpp"
#include "boost/filesystem/exception.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_SIGNED_UNSIGNED_MISMATCH
#include "myrrh/util/Preprocessor.hpp"

#define BOOST_AUTO_TEST_MAIN
#include "boost/lexical_cast.hpp"
#include "boost/test/auto_unit_test.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

#include <fstream>

using namespace myrrh::file;

// Local helper declarations

namespace
{

const std::string TEST_FILE_HEADER("TEST_FILE_HEADER_");

void EraseMatchingFiles(const boost::filesystem::path &folder,
                        const std::string &expression);

void CreateFiles(const PathStore &files);

void CheckExpressionStart(const std::string &toCheck);

class TestCase
{
public:

    TestCase(const boost::filesystem::path &folder);

    ~TestCase( );

    void operator( )( );

protected:

    const boost::filesystem::path &Folder( ) const;

private:

    virtual void DoTest( );
    virtual PathStore GetFiles( );
    virtual PathStore ExpectedMatches( );
    virtual boost::regex Expression( );

    boost::filesystem::path folder_;
};

class FilesExistCase : public TestCase
{
public:

    FilesExistCase(const boost::filesystem::path &folder, int count);
    virtual PathStore GetFiles( );

private:

    int count_;
};

}

// Test implementations

BOOST_AUTO_TEST_CASE(ExpressionMatcherWithEmptyPath)
{
    const ExpressionMatcher matcher(boost::regex("file\\.txt"));
    BOOST_CHECK(!matcher(""));
}

BOOST_AUTO_TEST_CASE(ExpressionMatcherMatches)
{
    const ExpressionMatcher matcher(boost::regex("file\\.\\w+"));
    BOOST_CHECK(matcher("file.txt"));
}

BOOST_AUTO_TEST_CASE(ExpressionMatcherDoesNotMatch)
{
    const ExpressionMatcher matcher(boost::regex("file\\.\\w+"));
    BOOST_CHECK(!matcher("file2.txt"));
}

BOOST_AUTO_TEST_CASE(ExpressionMatchInSubfolder)
{
    const ExpressionMatcher matcher(boost::regex("file\\.\\w+"));
    BOOST_CHECK(matcher("folder/file.txt"));
}

BOOST_AUTO_TEST_CASE(MatchFromCurrentDirectory)
{
    TestCase(".")( );
}

BOOST_AUTO_TEST_CASE(MatchFromSubDirectory)
{
    Eraser eraser(TEST_FILE_HEADER);
    boost::filesystem::create_directories(TEST_FILE_HEADER);
    TestCase test(TEST_FILE_HEADER);
    test( );
}

BOOST_AUTO_TEST_CASE(DirectoryDoesNotExist)
{
    try
    {
        MatchFiles("A/path/That/Definitely/Does/Not/Exist",
                   ExpressionMatcher(boost::regex(".*")));
        BOOST_ERROR("Passing path to unexisting folder should throw");
    }
    catch (const boost::filesystem::filesystem_error &)
    {
    }
}

BOOST_AUTO_TEST_CASE(NotADirectory)
{
    Temporary temporary(TEST_FILE_HEADER + ".txt");
    try
    {
        MatchFiles(temporary.Path( ), ExpressionMatcher(boost::regex(".*")));
        BOOST_ERROR("Passing path to unexisting folder should throw");
    }
    catch (const boost::filesystem::filesystem_error &)
    {
    }
}

BOOST_AUTO_TEST_CASE(DirectoryIsEmpty)
{
    const std::string PATH(TEST_FILE_HEADER);
    Eraser eraser(PATH);
    boost::filesystem::create_directories(PATH);
    ExpressionMatcher matcher(boost::regex(".*"));
    BOOST_CHECK(MatchFiles(PATH, matcher).empty( ));
}

BOOST_AUTO_TEST_CASE(DirectoryHasOnlyMatches)
{
    const std::string PATH(TEST_FILE_HEADER);
    Eraser eraser(PATH);
    boost::filesystem::create_directories(PATH);
    FilesExistCase(PATH, 5)( );
}

BOOST_AUTO_TEST_CASE(FilesButNoMatches)
{
    class Case : public FilesExistCase
    {
    public:

        Case( ) : FilesExistCase(".", 10)
        {
        }
        virtual boost::regex Expression( )
        {
            return boost::regex("^" + TEST_FILE_HEADER + "no matches");
        }
    };
    Case( )( );
}

BOOST_AUTO_TEST_CASE(OneMatchFromSeveral)
{
    class Case : public FilesExistCase
    {
    public:

        Case( ) : FilesExistCase(".", 10)
        {
        }
        virtual boost::regex Expression( )
        {
            return boost::regex("^" + TEST_FILE_HEADER + "5\\.txt");
        }
    };
    Case( )( );
}

BOOST_AUTO_TEST_CASE(SeveralMatchesFromSeveral)
{
    class Case : public FilesExistCase
    {
    public:

        Case( ) : FilesExistCase(".", 20)
        {
        }
        virtual boost::regex Expression( )
        {
            return boost::regex("^" + TEST_FILE_HEADER + "\\d\\.txt");
        }
    };
    Case( )( );
}

BOOST_AUTO_TEST_CASE(MatchingADirectory)
{
    class Case : public TestCase
    {
    public:

        Case( ) : TestCase(".")
        {
        }
        virtual boost::regex Expression( )
        {
            return boost::regex("^" + TEST_FILE_HEADER + ".*");
        }
        void DoTest( )
        {
            boost::filesystem::create_directories(TEST_FILE_HEADER);

            const PathStore EXPECTED(1, "./" + TEST_FILE_HEADER);
            const PathStore MATCHED(
                MatchFiles(Folder( ), ExpressionMatcher(Expression( ))));

            BOOST_CHECK(EXPECTED == MATCHED);
        }
    };
    Case( )( );
}

// Local helper implementations

namespace
{

void EraseMatchingFiles(const boost::filesystem::path &folder,
                        const std::string &expression)
{
    using namespace boost::filesystem;

    const boost::regex EXPRESSION(expression);
    for (directory_iterator i(folder); directory_iterator( ) != i; ++i)
    {
        if (boost::regex_search(i->path( ).string( ), EXPRESSION))
        {
            remove_all(*i);
        }
    }
}

void CreateFiles(const PathStore &store)
{
    for (PathStore::const_iterator i = store.begin( ); store.end( ) != i; ++i)
    {
        std::ofstream file(i->string( ).c_str( ));
        BOOST_ASSERT(file.is_open( ));
        file << "Jumprahuitti" << std::endl;
    }
}

#ifndef NDEBUG

void CheckExpressionStart(const std::string &toCheck)
{
    const size_t SIZE = TEST_FILE_HEADER.size( );
    assert(toCheck.size( ) > SIZE);
    assert(toCheck[0] == '^');
    assert(toCheck.substr(1, SIZE) == TEST_FILE_HEADER);
}

#else

void CheckExpressionStart(const std::string &)
{
}

#endif

TestCase::TestCase(const boost::filesystem::path &folder) :
    folder_(folder)
{
    EraseMatchingFiles(folder_, TEST_FILE_HEADER);
}

TestCase::~TestCase( )
{
    EraseMatchingFiles(folder_, TEST_FILE_HEADER);
}

void TestCase::operator( )( )
{
    DoTest( );
}

void TestCase::DoTest( )
{
    PathStore files(GetFiles( ));
    CreateFiles(files);

    const boost::regex EXPRESSION(Expression( ));
    CheckExpressionStart(EXPRESSION.str( ));

    const PathStore MATCHED(
        MatchFiles(Folder( ), ExpressionMatcher(EXPRESSION)));

    BOOST_CHECK(ExpectedMatches( ) == MATCHED);
}

PathStore TestCase::GetFiles( )
{
    return PathStore( );
}

PathStore TestCase::ExpectedMatches( )
{
    PathStore store(GetFiles( ));

    if (!store.empty( ))
    {
        ExpressionMatcher matcher(Expression( ));
        PathStore::iterator end(store.end( ));
        store.erase(std::remove_if(store.begin( ), end, std::not1(matcher)),
                    end);
    }

    return store;
}

boost::regex TestCase::Expression( )
{
    return boost::regex("^" + TEST_FILE_HEADER + ".*\\.txt$");
}

const boost::filesystem::path &TestCase::Folder( ) const
{
    return folder_;
}

FilesExistCase::FilesExistCase(const boost::filesystem::path &folder,
                               int count) :
    TestCase(folder),
    count_(count)
{
}

PathStore FilesExistCase::GetFiles( )
{
    PathStore result;
    for (int i = 0; i < count_; ++i)
    {
        const std::string ID(boost::lexical_cast<std::string>(i));
        result.push_back(Folder( ) / (TEST_FILE_HEADER + ID + ".txt"));
    }

    return result;
}

}
