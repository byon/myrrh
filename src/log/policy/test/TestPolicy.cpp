// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Policy
 *
 * The following situations are tested:
 * -The correct number of bytes is returned after writing
 * -Copy construction
 * -Assignment
 * -The initial opener opens a file that is usable
 * -The initial opener opens a file that is not usable
 * -Several attempts are needed to open a usable file
 * -The initial opener is usable for several writes
 * -The subsequent opener is usable for several writes
 * -Writing fails
 *
 * $Id: TestPolicy.cpp 369 2007-09-20 20:08:51Z byon $
 */

#include "myrrh/log/policy/Policy.hpp"
#include "myrrh/log/policy/Path.hpp"
#include "myrrh/log/policy/PathPart.hpp"
#include "myrrh/log/policy/Creator.hpp"
#include "myrrh/log/policy/Restriction.hpp"
#include "myrrh/file/Eraser.hpp"
#include "myrrh/file/ReadOnly.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_COPY_CONSTRUCTOR_COULD_NOT_BE_GENERATED
#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/lexical_cast.hpp"

#ifdef WIN32
#pragma warning(pop)
#include <windows.h>
#endif

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include <vector>

#undef TEXT

using namespace myrrh::log::policy;

typedef boost::unit_test::test_suite TestSuite;

// Test case function declarations
void WriteSize( );
void InitialFileIsNotRestricted( );
void InitialFileIsRestricted( );
void SeveralAttemptsNeededForUnrestrictedFile( );
void InitialFileIsNotRestrictedForSeveralWritings( );
void SubsequentFileIsUnrestrictedForSeveralWritings( );
void WritingFails( );
void PathIsRestricted( );
void TryingToOpenProtectedFile( );
void TryingToOpenReadOnlyFile( );
void FileBecomesReadOnly( );

// Helper function declarations
std::string GetFileContent(const boost::filesystem::path &path);
std::streamsize StringSize(const std::string &toMeasure);
boost::filesystem::path IntegerToPath(const boost::filesystem::path &path,
                                      int integer);

// Constants
const std::string TEST_FILE_DIRECTORY("testFiles/");
const std::string TEST_FILE_BASE(TEST_FILE_DIRECTORY + "tmp.log");
const std::string TEST_FILE2(TEST_FILE_DIRECTORY + "tmp2.log");

class ConfigurableRestriction : public Restriction
{
public:

    ConfigurableRestriction();
    virtual bool IsRestricted(const File &file, std::size_t toWrite) const;
    void AddUsability(bool isRestricted);

private:

    std::vector<bool> restrictionStore_;
    mutable std::vector<bool>::iterator current_;
};

typedef boost::shared_ptr<ConfigurableRestriction> ConfigurableUsablePtr;

class CasePolicy
{
public:

    CasePolicy( );

    virtual ~CasePolicy( );

    void AddUsability(bool isRestricted);
    virtual RestrictionPtr GetRestriction( );
    virtual Path GetPath(const boost::filesystem::path &path);
    virtual InitialOpenerPtr GetInitialOpener( );
    virtual OpenerPtr GetSubsequentOpener( );
    virtual PolicyPtr GetPolicy(const boost::filesystem::path &path);
    virtual void TestWrite(const std::string &toWrite, Policy &policy,
                           const boost::filesystem::path &path,
                           const std::string &expectedResult);

private:

    ConfigurableUsablePtr restriction_;
};

typedef boost::shared_ptr<CasePolicy> CasePolicyPtr;

class TestCase
{
public:

    TestCase(CasePolicyPtr casePolicy = CasePolicyPtr( ));

    virtual ~TestCase( );

    void operator( )( );

protected:

    virtual void DoTest( );
    CasePolicy &GetCasePolicy( );

private:

    CasePolicyPtr casePolicy_;
};

class IndexedPolicy : public CasePolicy
{
public:

    virtual Path GetPath(const boost::filesystem::path &path);
};

class NoOpener : public InitialOpener
{
    virtual boost::filesystem::path DoOpen(std::filebuf &, Path& path);
};

class FailingPolicy : public CasePolicy
{
public:
    virtual InitialOpenerPtr GetInitialOpener( );
};

class Restricted : public myrrh::log::policy::Restriction
{
public:

    Restricted( );
    virtual bool IsRestricted(const File &, std::size_t) const;

private:

    mutable bool isRestricted_;
};

class RestrictedPart : public myrrh::log::policy::Index
{
public:

    void AppendRestrictions(RestrictionStore &store);

    operator PartSum( ) const;
};

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test =
        BOOST_TEST_SUITE("Test suite for myrrh::log::policy::Policy");
    test->add(BOOST_TEST_CASE(WriteSize));
    test->add(BOOST_TEST_CASE(InitialFileIsNotRestricted));
    test->add(BOOST_TEST_CASE(InitialFileIsRestricted));
    test->add(BOOST_TEST_CASE(SeveralAttemptsNeededForUnrestrictedFile));
    test->add(BOOST_TEST_CASE(InitialFileIsNotRestrictedForSeveralWritings));
    test->add(BOOST_TEST_CASE(SubsequentFileIsUnrestrictedForSeveralWritings));
    test->add(BOOST_TEST_CASE(WritingFails));
    test->add(BOOST_TEST_CASE(PathIsRestricted));
#if WIN32
    test->add(BOOST_TEST_CASE(TryingToOpenProtectedFile));
#endif
    test->add(BOOST_TEST_CASE(TryingToOpenReadOnlyFile));
    test->add(BOOST_TEST_CASE(FileBecomesReadOnly));
    return test;
}

CasePolicy::CasePolicy( ) :
    restriction_(ConfigurableUsablePtr(new ConfigurableRestriction))
{
}

CasePolicy::~CasePolicy( )
{
}

RestrictionPtr CasePolicy::GetRestriction( )
{
    return restriction_;
}

void CasePolicy::AddUsability(bool isRestricted)
{
    restriction_->AddUsability(isRestricted);
}

Path CasePolicy::GetPath(const boost::filesystem::path &path)
{
    Path newPath;
    newPath += path.string( );
    return newPath;
}


InitialOpenerPtr CasePolicy::GetInitialOpener( )
{
    return InitialOpenerPtr(new Creator);
}

OpenerPtr CasePolicy::GetSubsequentOpener( )
{
    return OpenerPtr(new Creator);
}

PolicyPtr CasePolicy::GetPolicy(const boost::filesystem::path &path)
{
    InitialOpenerPtr initialOpener(GetInitialOpener( ));
    OpenerPtr subsequentOpener(GetSubsequentOpener( ));

    Path newPath(GetPath(path));
    PolicyPtr result(new Policy(newPath, initialOpener, subsequentOpener));

    RestrictionPtr restriction(GetRestriction( ));
    result->AddRestriction(restriction);

    return result;
}

void CasePolicy::TestWrite(const std::string &toWrite, Policy &policy,
                           const boost::filesystem::path &path,
                           const std::string &expectedResult)
{
    BOOST_CHECK_EQUAL(StringSize(toWrite), policy.Write(toWrite));
    BOOST_CHECK_EQUAL(GetFileContent(path), expectedResult);
}

TestCase::TestCase(CasePolicyPtr casePolicy) :
    casePolicy_(casePolicy ? casePolicy : CasePolicyPtr(new CasePolicy))
{
}

TestCase::~TestCase( )
{
    myrrh::file::Eraser eraser(TEST_FILE_DIRECTORY);
}

void TestCase::operator( )( )
{
    DoTest( );
}

void TestCase::DoTest( )
{
    const std::string TEXT("Just a string");

    PolicyPtr policy(casePolicy_->GetPolicy(TEST_FILE_BASE));

    casePolicy_->TestWrite(TEXT, *policy, TEST_FILE_BASE, TEXT);
}

CasePolicy &TestCase::GetCasePolicy( )
{
    return *casePolicy_;
}

ConfigurableRestriction::ConfigurableRestriction( ) :
    current_(restrictionStore_.begin( ))
{
}

bool ConfigurableRestriction::IsRestricted(const File &, std::size_t) const
{
    assert(current_ != restrictionStore_.end( ));
    return *current_++;
}

void ConfigurableRestriction::AddUsability(bool isRestricted)
{
    restrictionStore_.push_back(isRestricted);
    current_ = restrictionStore_.begin( );
}

Path IndexedPolicy::GetPath(const boost::filesystem::path &path)
{
    Path newPath;
    newPath += path.string( ) + Index( );
    return newPath;
}

boost::filesystem::path NoOpener::DoOpen(std::filebuf &, Path&)
{
    return TEST_FILE_BASE;
}

InitialOpenerPtr FailingPolicy::GetInitialOpener( )
{
    return InitialOpenerPtr(new NoOpener);
}

Restricted::Restricted( ) :
    isRestricted_(true)
{
}

bool Restricted::IsRestricted(const File &, std::size_t) const
{
    bool result = isRestricted_;
    isRestricted_ = !isRestricted_;
    return result;
}

void RestrictedPart::AppendRestrictions(RestrictionStore &store)
{
    store.Add(myrrh::log::policy::RestrictionPtr(new Restricted));
}

RestrictedPart::operator PartSum( ) const
{
    PartSum sum;
    sum.Add(*this);
    return sum;
}

void WriteSize( )
{
    class Case : public TestCase
    {
        virtual void DoTest( )
        {
            const std::string TEXT("This is a string without line end");

            GetCasePolicy( ).AddUsability(false);
            GetCasePolicy( ).AddUsability(false);

            PolicyPtr policy(GetCasePolicy( ).GetPolicy(TEST_FILE_BASE));
            GetCasePolicy( ).TestWrite(TEXT, *policy, TEST_FILE_BASE, TEXT);

            const std::string TEXT2("This is a string with line end\n");
            GetCasePolicy( ).TestWrite(TEXT2, *policy, TEST_FILE_BASE,
                                       TEXT + TEXT2);
        }
    };

    Case( )( );
}

void InitialFileIsNotRestricted( )
{
    class Case : public TestCase
    {
        virtual void DoTest( )
        {
            const std::string TEXT("Just a string");

            GetCasePolicy( ).AddUsability(false);

            PolicyPtr policy(GetCasePolicy( ).GetPolicy(TEST_FILE_BASE));
            GetCasePolicy( ).TestWrite(TEXT, *policy, TEST_FILE_BASE, TEXT);
        }
    };

    Case( )( );
}

void InitialFileIsRestricted( )
{
    class Case : public TestCase
    {
    public:

        virtual void DoTest( )
        {
            const std::string TEXT("Just a string");

            GetCasePolicy( ).AddUsability(true);
            GetCasePolicy( ).AddUsability(false);

            PolicyPtr policy(GetCasePolicy( ).GetPolicy(TEST_FILE_BASE));
            GetCasePolicy( ).TestWrite(TEXT, *policy, TEST_FILE_BASE, TEXT);
        }
    };

    Case( )( );
}

void SeveralAttemptsNeededForUnrestrictedFile( )
{
    class Case : public TestCase
    {
    public:

        Case( ) :
            TestCase(CasePolicyPtr(new IndexedPolicy))
        {
        }

        virtual void DoTest( )
        {
            const std::string TEXT("Just a string");
            using namespace boost::filesystem;

            const int NOT_USABLE_COUNT = 100;

            for (int i = 0; i < NOT_USABLE_COUNT; ++i)
            {
                GetCasePolicy( ).AddUsability(true);
            }

            GetCasePolicy( ).AddUsability(false);
            PolicyPtr policy(GetCasePolicy( ).GetPolicy(TEST_FILE_BASE));

            BOOST_CHECK_EQUAL(StringSize(TEXT), policy->Write(TEXT));

            for (int i = 1; i < NOT_USABLE_COUNT; ++i)
            {
                using namespace boost::filesystem;
                const path PATH(IntegerToPath(TEST_FILE_BASE, i));
                BOOST_CHECK_EQUAL(true, exists(PATH));
                BOOST_CHECK_EQUAL(0, file_size(PATH));
            }

            BOOST_CHECK_EQUAL(GetFileContent(
                IntegerToPath(TEST_FILE_BASE, NOT_USABLE_COUNT +1)), TEXT);
        }
    };

    Case( )( );
}

void InitialFileIsNotRestrictedForSeveralWritings( )
{
    class Case : public TestCase
    {
    public:

        virtual void DoTest( )
        {
            const std::string TEXT("Just a string");

            const int USABLE_COUNT = 100;

            for (int i = 0; i < USABLE_COUNT; ++i)
            {
                GetCasePolicy( ).AddUsability(false);
            }

            PolicyPtr policy(GetCasePolicy( ).GetPolicy(TEST_FILE_BASE));
            std::string written;

            for (int i = 0; i < USABLE_COUNT; ++i)
            {
                GetCasePolicy( ).TestWrite(TEXT, *policy, TEST_FILE_BASE,
                                           written + TEXT);
                written += TEXT;
            }
        }
    };

    Case( )( );
}

void SubsequentFileIsUnrestrictedForSeveralWritings( )
{
    class Case : public TestCase
    {
    public:

        virtual void DoTest( )
        {
            const std::string TEXT("Just a string");

            const int USABLE_COUNT = 100;

            GetCasePolicy( ).AddUsability(true);

            for (int i = 0; i < USABLE_COUNT; ++i)
            {
                GetCasePolicy( ).AddUsability(false);
            }

            PolicyPtr policy(GetCasePolicy( ).GetPolicy(TEST_FILE_BASE));

            std::string written;

            for (int i = 0; i < USABLE_COUNT; ++i)
            {
                GetCasePolicy( ).TestWrite(TEXT, *policy, TEST_FILE_BASE,
                                           written + TEXT);
                written += TEXT;
            }
        }
    };

    Case( )( );
}

void WritingFails( )
{
    class Case : public TestCase
    {
    public:

        Case( ) :
            TestCase(CasePolicyPtr(new FailingPolicy))
        {
        }

        virtual void DoTest( )
        {
            GetCasePolicy( ).AddUsability(false);
            PolicyPtr policy(GetCasePolicy( ).GetPolicy(TEST_FILE_BASE));
            BOOST_CHECK_EQUAL(-1, policy->Write("Something"));
        }
    };

    Case( )( );
}

void PathIsRestricted( )
{
    using namespace myrrh::log::policy;
    Path path;
    path += "TestFolder/Test" + RestrictedPart( ) + ".txt";

    myrrh::file::Eraser eraser("TestFolder");
    InitialOpenerPtr opener(new Creator( ));
    Policy policy(path, opener, opener);

    policy.Write("Something");
    BOOST_CHECK(GetFileContent("TestFolder/Test1.txt").empty( ));
    BOOST_CHECK_EQUAL("Something", GetFileContent("TestFolder/Test2.txt"));
}

#if WIN32

/// @todo Find a similar scenario for linux as well?

void TryingToOpenProtectedFile( )
{
    class ProtectedFile
    {
    public:
        explicit ProtectedFile(const boost::filesystem::path &path)
        {
            file_ = CreateFile(path.string( ).c_str( ), GENERIC_WRITE,
                               FILE_SHARE_READ, 0, CREATE_ALWAYS,
                               FILE_FLAG_DELETE_ON_CLOSE, 0);
            BOOST_REQUIRE(file_ != INVALID_HANDLE_VALUE);
        }

        ~ProtectedFile( )
        {
            CloseHandle(file_);
        }

        void Write(const std::string& text)
        {
            DWORD written = 0;
            BOOST_REQUIRE(WriteFile(file_, text.c_str( ),
                                    static_cast<DWORD>(text.size( )),
                                    &written, 0));
            BOOST_REQUIRE(written == text.size( ));
        }

    private:

        HANDLE file_;
    };

    myrrh::log::policy::Path path;
    path += Text("Test.txt");

    ProtectedFile file(path.Generate( ));
    file.Write("JustSomeRubbish");

    InitialOpenerPtr opener(new Creator( ));
    Policy policy(path, opener, opener);
    BOOST_CHECK_EQUAL(-1, policy.Write("Hubbadeijaa"));
}
#endif

void TryingToOpenReadOnlyFile( )
{
    myrrh::log::policy::Path path;
    path += Text("Test.txt");

    myrrh::file::ReadOnly file(path.Generate( ), "JustSomeRubbish");

    InitialOpenerPtr opener(new Creator( ));
    Policy policy(path, opener, opener);
    BOOST_CHECK_EQUAL(-1, policy.Write("Hubbadeijaa"));
}

void FileBecomesReadOnly( )
{
    myrrh::log::policy::Path path;
    path += Text("Test.txt");

    // By declaring the scoped_ptr before Policy, we'll get the correct order
    // of destructing objects (i.e. std::ofstream in Policy gets closed before
    // myrrh::file::ReadOnly destructor tries to delete the file).
    typedef boost::shared_ptr<myrrh::file::ReadOnly> ReadOnlyPtr;
    ReadOnlyPtr readOnlyFile;

    InitialOpenerPtr opener(new Creator( ));
    Policy policy(path, opener, opener);
    BOOST_CHECK(policy.Write("Hubbadeijaa") > 0);

    readOnlyFile = ReadOnlyPtr(new myrrh::file::ReadOnly(path.Generate( ), ""));
    BOOST_CHECK(policy.Write("Hubbadeijaa") > 0);
}

std::string GetFileContent(const boost::filesystem::path &path)
{
    std::ifstream file(path.string( ).c_str( ));
    if (!file.is_open( ))
    {
        BOOST_ERROR("Failed to open file " + path.string( ));
    }

    std::ostringstream stringStream;
    stringStream << file.rdbuf( );

    return stringStream.str( );
}

std::streamsize StringSize(const std::string &toMeasure)
{
    return static_cast<std::streamsize>(toMeasure.size( ));
}

boost::filesystem::path IntegerToPath(const boost::filesystem::path &path,
                                      int integer)
{
    return boost::filesystem::path(path.string( ) +
                                   boost::lexical_cast<std::string>(integer));
}
