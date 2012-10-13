// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for SafeModify
 *
 * $Id: TestSafeModify.cpp 354 2007-09-17 17:39:58Z byon $
 */

#include "myrrh/file/SafeModify.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/filesystem/operations.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <fstream>
#include <sstream>

typedef boost::unit_test::test_suite TestSuite;

// constants for testing
static const std::string FILE_NAME("File.txt");
static const std::string ORIGINAL_CONTENT("This is the original content");
static const std::string REPLACED_CONTENT("This is the replaced content");

// forward declarations of helper functions
void WriteContentToFile(const boost::filesystem::path fileName,
                        const std::string &content);
void CheckContent(const std::string &requiredContent);
void ReplaceFileContent(bool doDelete, bool doException);

/**
 * A helper class used to ensure that tests leave no files hanging around
 */
class FileRemover
{
public:

    FileRemover( )
    {
        Remove( );
    }

    ~FileRemover( )
    {
        try
        {
            Remove( );
        }
        catch (...)
        {
        }
    }

private:

    void Remove( )
    {
        using namespace boost::filesystem;
        remove(FILE_NAME);
        remove(myrrh::file::SafeModify::Name(FILE_NAME));
    }
};

/**
 * A local class for simulated exception
 */
class TesterError : public std::runtime_error
{
public:

    TesterError(const std::string &what) :
        std::runtime_error(what)
    {
    }
};


void OriginalFileHasEmptyPath( )
{
    FileRemover cleanup;
    boost::filesystem::path path;
    try
    {
        myrrh::file::SafeModify modifier(path);
        BOOST_ERROR("SafeModify constructor should have thrown");
    }
    catch(const boost::filesystem::filesystem_error &)
    {
    }
}

void OriginalFileDoesNotExist( )
{
    FileRemover cleanup;
    boost::filesystem::path path("DoesNotExist.txt");
    try
    {
        myrrh::file::SafeModify modifier(path);
        BOOST_ERROR("SafeModify constructor should have thrown");
    }
    catch(const boost::filesystem::filesystem_error &)
    {
    }
}

void TemporaryFileName( )
{
    FileRemover cleanup;
    const std::string BASE_NAME("BaseName.txt");
    const boost::filesystem::path PATH(BASE_NAME);

    const std::string RESULT(myrrh::file::SafeModify::Name(PATH));

    BOOST_CHECK(!RESULT.empty( ));
    BOOST_CHECK_EQUAL(RESULT, BASE_NAME + ".tmp");
}

void SuccessfullModification( )
{
    FileRemover cleanup;
    WriteContentToFile(FILE_NAME, ORIGINAL_CONTENT);

    ReplaceFileContent(true, false);

    using namespace boost::filesystem;
    BOOST_CHECK(!exists(myrrh::file::SafeModify::Name(FILE_NAME)));
    BOOST_CHECK(exists(FILE_NAME));

    CheckContent(REPLACED_CONTENT + '\n');
}

void ForgottenDelete( )
{
    FileRemover cleanup;
    WriteContentToFile(FILE_NAME, ORIGINAL_CONTENT);
    ReplaceFileContent(false, false);

    using namespace boost::filesystem;
    BOOST_CHECK(!exists(myrrh::file::SafeModify::Name(FILE_NAME)));
    BOOST_CHECK(exists(FILE_NAME));

    CheckContent(ORIGINAL_CONTENT + '\n');
}

void UnsuccessfullModification( )
{
    FileRemover cleanup;
    WriteContentToFile(FILE_NAME, ORIGINAL_CONTENT);

    try
    {
        ReplaceFileContent(true, true);
        BOOST_ERROR("The previous call should have thrown");
    }
    catch (const TesterError &)
    {
    }

    using namespace boost::filesystem;
    BOOST_CHECK(!exists(myrrh::file::SafeModify::Name(FILE_NAME)));
    BOOST_CHECK(boost::filesystem::exists(FILE_NAME));

    CheckContent(ORIGINAL_CONTENT + '\n');
}

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for SafeModify");

    test->add(BOOST_TEST_CASE(OriginalFileHasEmptyPath));
    test->add(BOOST_TEST_CASE(OriginalFileDoesNotExist));
    test->add(BOOST_TEST_CASE(TemporaryFileName));
    test->add(BOOST_TEST_CASE(SuccessfullModification));
    test->add(BOOST_TEST_CASE(ForgottenDelete));
    test->add(BOOST_TEST_CASE(UnsuccessfullModification));

    return test;
}

// Helper functions

void WriteContentToFile(const boost::filesystem::path fileName,
                        const std::string &content)
{
    std::ofstream fileStream(fileName.string( ).c_str( ));
    BOOST_REQUIRE(fileStream.is_open( ));
    fileStream << content << std::endl;
}

void CheckContent(const std::string &requiredContent)
{
    std::ifstream file(FILE_NAME.c_str( ));
    BOOST_REQUIRE(file.is_open( ));

    std::ostringstream content;
    content << file.rdbuf( );
    content.flush( );

    BOOST_CHECK_EQUAL(requiredContent, content.str( ));
}

void ReplaceFileContent(bool doDelete, bool doException)
{
    using namespace myrrh::file;
    SafeModify modifier(FILE_NAME);

    BOOST_CHECK(boost::filesystem::exists(SafeModify::Name(FILE_NAME)));
    BOOST_CHECK(!boost::filesystem::exists(FILE_NAME));

    WriteContentToFile(FILE_NAME, REPLACED_CONTENT);

    if (doException)
    {
        throw TesterError("An exception for testing purpose");
    }

    if (doDelete)
    {
        modifier.Commit( );
    }
}
