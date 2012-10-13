// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "myrrh/log/policy/Appender.hpp"
#include "myrrh/log/policy/Creator.hpp"
#include "myrrh/log/policy/Resizer.hpp"
#include "myrrh/log/policy/PathPart.hpp"
#include "myrrh/log/policy/Path.hpp"
#include "myrrh/log/policy/File.hpp"
#include "myrrh/file/Eraser.hpp"

#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_SIGNED_UNSIGNED_MISMATCH
#include "myrrh/util/Preprocessor.hpp"

#define BOOST_AUTO_TEST_MAIN
#include "boost/test/auto_unit_test.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

/// @todo Disabling warning for the whole of the file?
#pragma warning(disable: 4511)
#pragma warning(disable: 4512)

#include <iostream>

using namespace myrrh::log::policy;

// Local declarations
namespace
{

Path GetPath(const std::string &name, const std::string &folder = "");
template <typename T>
void TestConstruction( );
template <typename T>
void TestConstruction(T &opener);
std::string GetFileContent(const boost::filesystem::path &path);
std::streamsize StringSize(const std::string &text);
template <typename T>
void DoWritingEmptyLine(T &opener);
template <typename T>
void DoWritingLine(T &opener);
template <typename T>
void DoWritingLines(T &opener);
template <typename T>
void DoOpeningFromSubFolder(T &opener, const boost::filesystem::path &folder);
void CreateFile(const boost::filesystem::path &path,
                const std::string &content);

const std::string ORIGINAL_CONTENT("Original content\n");
const std::string NEW_CONTENT("New content\n");

}

BOOST_AUTO_TEST_CASE(WritingWhenFileNotOpen)
{
    class FailingOpener : public Opener
    {
        virtual boost::filesystem::path DoOpen(std::filebuf &, Path&)
        {
            return "";
        }
    };

    Path path("File.txt");
    FailingOpener opener;
    FilePtr file(opener.Open(path));
    BOOST_CHECK_EQUAL(-1, file->Write("A line that cannot get written"));
    BOOST_CHECK_EQUAL(0, file->WrittenSize( ));
}

BOOST_AUTO_TEST_CASE(AppenderConstruction)
{
    TestConstruction<Appender>( );
}

BOOST_AUTO_TEST_CASE(CreatorConstruction)
{
    TestConstruction<Creator>( );
}

BOOST_AUTO_TEST_CASE(ResizerConstruction)
{
    myrrh::file::Eraser eraser("tmp.log");
    Resizer resizer(0);
    TestConstruction(resizer);
}

BOOST_AUTO_TEST_CASE(WritingEmptyLineThroughAppender)
{
    Appender appender;
    DoWritingEmptyLine(appender);
}

BOOST_AUTO_TEST_CASE(WritingEmptyLineThroughCreator)
{
    Creator creator;
    DoWritingEmptyLine(creator);
}

BOOST_AUTO_TEST_CASE(WritingEmptyLineThroughResizer)
{
    Resizer resizer(128);
    DoWritingEmptyLine(resizer);
}

BOOST_AUTO_TEST_CASE(WritingLineThroughAppender)
{
    Appender appender;
    DoWritingLine(appender);
}

BOOST_AUTO_TEST_CASE(WritingLineThroughCreator)
{
    Creator creator;
    DoWritingLine(creator);
}

BOOST_AUTO_TEST_CASE(WritingLineThroughResizer)
{
    Resizer resizer(128);
    DoWritingLines(resizer);
}

BOOST_AUTO_TEST_CASE(WritingLinesThroughAppender)
{
    Appender appender;
    DoWritingLines(appender);
}

BOOST_AUTO_TEST_CASE(WritingLinesThroughCreator)
{
    Creator creator;
    DoWritingLines(creator);
}

BOOST_AUTO_TEST_CASE(WritingLinesThroughResizer)
{
    Resizer resizer(128);
    DoWritingLines(resizer);
}

BOOST_AUTO_TEST_CASE(FileEquality)
{
    Path origName(GetPath("tmp.log"));
    myrrh::file::Eraser origEraser(origName.Generate( ));
    Creator origCreator;
    FilePtr origFile(origCreator.Open(origName));

    Path notSameName(GetPath("tmp2.log"));
    myrrh::file::Eraser notSameEraser(notSameName.Generate( ));
    Creator notSameCreator;
    FilePtr notSameFile(notSameCreator.Open(notSameName));

    Creator sameCreator;
    FilePtr sameFile(sameCreator.Open(origName));

    BOOST_CHECK(*origFile == *sameFile);
    BOOST_CHECK(*sameFile == *origFile);
    BOOST_CHECK(*origFile != *notSameFile);
    BOOST_CHECK(*notSameFile != *origFile);
    BOOST_CHECK(*notSameFile != *sameFile);
    BOOST_CHECK(*sameFile != *notSameFile);
}

BOOST_AUTO_TEST_CASE(FilePathName)
{
    Path path(GetPath("tmp.log"));
    myrrh::file::Eraser eraser(path.Generate( ));
    Creator creator;
    FilePtr file(creator.Open(path));

    BOOST_CHECK_EQUAL(path.Generate( ).string( ), file->Path( ).string( ));
}

BOOST_AUTO_TEST_CASE(OpeningFromSubFolderThroughAppender)
{
    myrrh::file::Eraser eraser("subfolder1");
    using namespace boost::filesystem;
    Appender appender;
    DoOpeningFromSubFolder(appender, "subfolder1/subfolder2/subfolder3/");
}

BOOST_AUTO_TEST_CASE(OpeningFromSubFolderThroughCreator)
{
    myrrh::file::Eraser eraser("subfolder1");
    using namespace boost::filesystem;
    Creator creator;
    DoOpeningFromSubFolder(creator, "subfolder1/subfolder2/subfolder3/");
}

BOOST_AUTO_TEST_CASE(OpeningFromSubFolderThroughResizer)
{
    myrrh::file::Eraser eraser("subfolder1");
    using namespace boost::filesystem;
    Resizer resizer(128);
    DoOpeningFromSubFolder(resizer, "subfolder1/subfolder2/subfolder3/");
}

BOOST_AUTO_TEST_CASE(CreatingFilePathWithSubfolderThatIsActuallyAFile)
{
    myrrh::file::Eraser eraser("folder");
    CreateFile("folder/subfolder", "Some content\n");
    Creator creator;
    creator.Open(GetPath("tmp.log", "folder/subfolder/"));
    BOOST_CHECK_EQUAL("Some content\n", GetFileContent("folder/subfolder"));
}

BOOST_AUTO_TEST_CASE(AppendingFilePathWithSubfolderThatIsActuallyAFile)
{
    myrrh::file::Eraser eraser("folder");
    CreateFile("folder/subfolder", "Some content\n");
    Appender appender;
    appender.Open(GetPath("tmp.log", "folder/subfolder/"));
    BOOST_CHECK_EQUAL("Some content\n", GetFileContent("folder/subfolder"));
}

BOOST_AUTO_TEST_CASE(ResizingFilePathWithSubfolderThatIsActuallyAFile)
{
    myrrh::file::Eraser eraser("folder");
    CreateFile("folder/subfolder", "Some content\n");
    Resizer resizer(128);
    FilePtr file(resizer.Open(GetPath("tmp.log", "folder/subfolder/")));
    BOOST_CHECK_EQUAL("Some content\n", GetFileContent("folder/subfolder"));
    BOOST_CHECK_EQUAL(-1, file->Write("Some content"));
}

BOOST_AUTO_TEST_CASE(AppendingWhenFileExists)
{
    myrrh::file::Eraser eraser("tmp.log");
    CreateFile("tmp.log", ORIGINAL_CONTENT);

    Appender opener;
    FilePtr file(opener.Open(GetPath("tmp.log")));
    BOOST_CHECK_EQUAL(StringSize(ORIGINAL_CONTENT), file->WrittenSize( ));

    BOOST_CHECK_EQUAL(StringSize(NEW_CONTENT), file->Write(NEW_CONTENT));

    BOOST_CHECK_EQUAL(ORIGINAL_CONTENT + NEW_CONTENT,
                      GetFileContent("tmp.log"));
}

BOOST_AUTO_TEST_CASE(CreatingWhenFileExists)
{
    myrrh::file::Eraser eraser("tmp.log");
    CreateFile("tmp.log", ORIGINAL_CONTENT);

    Creator opener;
    FilePtr file(opener.Open(GetPath("tmp.log")));
    BOOST_CHECK_EQUAL(0, file->WrittenSize( ));

    BOOST_CHECK_EQUAL(StringSize(NEW_CONTENT), file->Write(NEW_CONTENT));
    BOOST_CHECK_EQUAL(NEW_CONTENT, GetFileContent("tmp.log"));
}

BOOST_AUTO_TEST_CASE(ResizingWhenFileExistsAndMaxNotExceeded)
{
    myrrh::file::Eraser eraser("tmp.log");
    CreateFile("tmp.log", ORIGINAL_CONTENT);

    Resizer opener(1024);
    FilePtr file(opener.Open(GetPath("tmp.log")));
    BOOST_CHECK_EQUAL(0, file->WrittenSize( ));

    BOOST_CHECK_EQUAL(StringSize(NEW_CONTENT), file->Write(NEW_CONTENT));
    BOOST_CHECK_EQUAL(NEW_CONTENT, GetFileContent("tmp.log"));
}

BOOST_AUTO_TEST_CASE(ResizingWhenFileExistsAndMaxExceeded)
{
    myrrh::file::Eraser eraser("tmp.log");
    const std::string CONTENT_BEFORE(
        ORIGINAL_CONTENT + ORIGINAL_CONTENT + ORIGINAL_CONTENT +
        ORIGINAL_CONTENT + ORIGINAL_CONTENT + ORIGINAL_CONTENT);
    CreateFile("tmp.log", CONTENT_BEFORE);

    Resizer opener(64);
    FilePtr file(opener.Open(GetPath("tmp.log")));
    const std::streamsize EXPECTED_SIZE =
        (64 / StringSize(ORIGINAL_CONTENT)) * StringSize(ORIGINAL_CONTENT);

    const std::string EXPECTED_AFTER_RESIZE(
        ORIGINAL_CONTENT + ORIGINAL_CONTENT + ORIGINAL_CONTENT);
    assert(EXPECTED_SIZE == StringSize(EXPECTED_AFTER_RESIZE));

    BOOST_CHECK_EQUAL(EXPECTED_SIZE, file->WrittenSize( ));

    BOOST_CHECK_EQUAL(StringSize(NEW_CONTENT), file->Write(NEW_CONTENT));
    BOOST_CHECK_EQUAL(EXPECTED_AFTER_RESIZE + NEW_CONTENT,
                      GetFileContent("tmp.log"));
}

BOOST_AUTO_TEST_CASE(AppendingWhenManyFilesMatchPathRule)
{
    myrrh::file::Eraser eraser("folder");
    CreateFile("folder/myrrh2.log", "Some content\n");
    CreateFile("folder/myrrh1300.log", "Some content\n");
    CreateFile("folder/myrrh10.log", "Some content\n");
    CreateFile("folder/myrrh01.log", "Some content\n");
    CreateFile("folder/myrrh11.log", "Some content\n");
    CreateFile("folder/myrrh1234.log", "Some content\n");

    Path path;
    path += "folder/myrrh" + Index( ) + ".log";
    Appender appender;

    FilePtr file(appender.Open(path));

    boost::filesystem::path expected("./folder/myrrh1300.log");
    BOOST_CHECK_EQUAL(expected, file->Path( ));
}

// Local declarations
namespace
{

Path GetPath(const std::string &name, const std::string &folder)
{
    Path path(folder);
    path += name;
    return path;
}

template <typename T>
void TestConstruction( )
{
    myrrh::file::Eraser eraser("tmp.log");
    T opener;
    TestConstruction(opener);
}

template <typename T>
void TestConstruction(T &opener)
{
    FilePtr file(opener.Open(GetPath("tmp.log")));
    BOOST_CHECK_EQUAL(0, file->WrittenSize( ));
    BOOST_CHECK_EQUAL("tmp.log", file->Path( ).string( ));
    BOOST_CHECK(boost::filesystem::exists("tmp.log"));
}

std::string GetFileContent(const boost::filesystem::path &path)
{
    std::ifstream file(path.string( ).c_str( ));
    if (!file.is_open( ))
    {
        return "";
    }
    std::ostringstream stream;
    stream << file.rdbuf( );
    return stream.str( );
}

unsigned GetEndOfLineSize( )
{
#ifdef WIN32
    return 2;
#else
    return 1;
#endif
}

std::streamsize StringSize(const std::string &text)
{
    static const unsigned EXTRA_LINE_SIZE = GetEndOfLineSize( ) - 1;
    const size_t LINES = std::count(text.begin( ), text.end( ), '\n');
    return static_cast<std::streamsize>(text.size( ) + LINES * EXTRA_LINE_SIZE);
}

template <typename T>
void DoWritingEmptyLine(T &opener)
{
    myrrh::file::Eraser eraser("tmp.log");
    Path path("tmp.log");
    FilePtr file(opener.Open(path));
    BOOST_CHECK_EQUAL(0, file->Write(""));
    BOOST_CHECK_EQUAL(0, file->WrittenSize( ));
    BOOST_CHECK(GetFileContent("tmp.log").empty( ));
}

template <typename T>
void DoWritingLine(T &opener)
{
    myrrh::file::Eraser eraser("tmp.log");
    Path path("tmp.log");
    FilePtr file(opener.Open(path));
    const std::string CONTENT("Line to be used for testing");
    BOOST_CHECK_EQUAL(std::streamsize(CONTENT.size( )), file->Write(CONTENT));
    BOOST_CHECK_EQUAL(std::streamsize(CONTENT.size( )), file->WrittenSize( ));
    BOOST_CHECK(CONTENT == GetFileContent("tmp.log"));
}

template <typename T>
void DoWritingLines(T &opener)
{
    myrrh::file::Eraser eraser("tmp.log");
    Path path("tmp.log");
    FilePtr file(opener.Open(path));
    const std::string CONTENT("Line to be used for testing\n");
    std::string written;

    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK_EQUAL(StringSize(CONTENT), file->Write(CONTENT));
        written += CONTENT;
        BOOST_CHECK_EQUAL(StringSize(written), file->WrittenSize( ));
    }

    BOOST_CHECK(written == GetFileContent("tmp.log"));
}

template <typename T>
void DoOpeningFromSubFolder(T &opener, const boost::filesystem::path &folder)
{
    Path path(GetPath("tmp.log", folder.string( )));
    FilePtr file(opener.Open(path));
    BOOST_CHECK_EQUAL((folder / "tmp.log").string( ), file->Path( ).string( ));
    BOOST_CHECK(boost::filesystem::exists(file->Path( )));
    const std::string CONTENT("Line to be used for testing");
    BOOST_CHECK_EQUAL(std::streamsize(CONTENT.size( )), file->Write(CONTENT));
    BOOST_CHECK_EQUAL(std::streamsize(CONTENT.size( )), file->WrittenSize( ));
    BOOST_CHECK(CONTENT == GetFileContent(file->Path( ).string( )));
}

void CreateFile(const boost::filesystem::path &path,
                const std::string &content)
{
    if (path.has_branch_path( ))
    {
        boost::filesystem::create_directories(path.branch_path( ));
    }
    std::ofstream file(path.string( ).c_str( ));
    BOOST_ASSERT(file.is_open( ));

    file << content;
}

}
