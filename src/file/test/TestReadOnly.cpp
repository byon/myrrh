/**
 * This file contains the unit test(s) for ReadOnlyFile
 *
 * $Id: TestReadOnly.cpp 361 2007-09-19 19:59:44Z byon $
 */

#include "myrrh/file/ReadOnly.hpp"
#include "myrrh/file/Temporary.hpp"
#include "boost/filesystem/operations.hpp"

#define BOOST_AUTO_TEST_MAIN
#include "boost/test/auto_unit_test.hpp"
#include <fstream>

// Local declarations

namespace
{

std::string FileContent(const boost::filesystem::path& path);

}

BOOST_AUTO_TEST_CASE(FileIsWritten)
{
    BOOST_CHECK(!boost::filesystem::exists("tmp.txt"));
    myrrh::file::ReadOnly file("tmp.txt", "content");
    BOOST_CHECK(boost::filesystem::exists("tmp.txt"));
}

BOOST_AUTO_TEST_CASE(FileIsReadOnly)
{
    myrrh::file::ReadOnly file("tmp.txt", "content");
    std::ofstream writeTry("tmp.txt");
    BOOST_CHECK(!writeTry.is_open( ));
}

BOOST_AUTO_TEST_CASE(AutomaticDeletion)
{
    {
        myrrh::file::ReadOnly file("tmp.txt", "content");
        BOOST_CHECK(boost::filesystem::exists("tmp.txt"));
    }
    BOOST_CHECK(!boost::filesystem::exists("tmp.txt"));
}

BOOST_AUTO_TEST_CASE(ContentIsCorrect)
{
    myrrh::file::ReadOnly file("tmp.txt", "content");
    BOOST_CHECK_EQUAL(FileContent("tmp.txt"), "content");
}

BOOST_AUTO_TEST_CASE(FileAlreadyExists)
{
    myrrh::file::Temporary temporary("tmp.txt");
    temporary.Stream( ) << "Old content" << std::endl;

    myrrh::file::ReadOnly file("tmp.txt", "content");
    BOOST_CHECK_EQUAL(FileContent("tmp.txt"), "Old content\n");
}

// Local declarations

namespace
{

std::string FileContent(const boost::filesystem::path& path)
{
    std::ifstream readFile(path.string( ).c_str( ));
    BOOST_CHECK(readFile.is_open( ));
    std::ostringstream stream;
    stream << readFile.rdbuf( );
    return stream.str( );
}

}
