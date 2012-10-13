// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Restriction interface
 * implementations
 *
 * The following situations are tested for all subclasses of Restriction:
 * -The underlying file is not open
 * -Zero maximum size
 * -Zero size to write
 * -Size to write + content is smaller than maximum size
 * -Size to write + content one smaller than maximum size
 * -Size to write + content is equal than maximum size
 * -Size to write + content is one larger than maximum size
 * -Size to write + content is larger than maximum size
 * -Size to write is equal than the maximum size
 * -Size to write is larger than the maximum size
 *
 * In addition the following test sare made for DateRestriction:
 * -The date has changed since last writing.
 *
 * $Id: TestRestriction.cpp 356 2007-09-18 19:55:21Z byon $
 */

// Rewrite

#include "myrrh/log/policy/Restriction.hpp"
#include "myrrh/log/policy/Creator.hpp"
#include "myrrh/log/policy/File.hpp"
#include "myrrh/log/policy/Path.hpp"

#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"
#include "boost/algorithm/string/erase.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/filesystem/operations.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

using namespace myrrh::log::policy;

typedef boost::unit_test::test_suite TestSuite;

// Helper function and class declarations

template <typename T>
boost::unit_test::test_case *NewCase(T caseFunction);

template <typename Types, typename Case>
void AddFileStateCases(TestSuite *test);

template <typename Types, typename FileInitializer, typename Line,
         typename Case>
void AddCase(TestSuite *test);

int isNewDate = 0;
class DateCreatorForTests
{
public:
    typedef int Date;
    static int NewDate( )
    {
        return isNewDate;
    }
};

class TestSetup
{
public:

    TestSetup( );
    ~TestSetup( );
};


struct Double
{
    size_t operator( )(size_t size) const;
};

struct Half
{
    size_t operator( )(size_t size) const;
};

struct NoOperator
{
    size_t operator( )(size_t size) const;
};

struct MinusOne
{
    size_t operator( )(size_t size) const;
};

struct PlusOne
{
    size_t operator( )(size_t size) const;
};

const std::string FILE_NAME("RestrictionTest.txt");

//
// Test policy class declarations
//

// Creator class declarations

struct SizeRestrictionCreator
{
    template <typename T>
    RestrictionPtr NewRestriction(const T &host) const;
};

struct DateRestrictionCreator
{
    template <typename T>
    RestrictionPtr NewRestriction(const T &host) const;
};

// Params class declarations

struct NoParams
{
};

template <size_t Size>
struct SizeParams
{
    size_t GetParam1( ) const;
};

// FileInitializer class declarations

struct FileClosed
{
    template <typename T>
    FilePtr GetFile(const T &host) const;
};

struct EmptyFile
{
    static const size_t FILE_SIZE = 0;

    template <typename T>
    FilePtr GetFile(const T &host) const;
};

template <size_t Size>
struct FileWithContent
{
    static const size_t FILE_SIZE = Size;

    template <typename T>
    FilePtr GetFile(const T &host) const;
};

// LineInitializer class declarations

template <size_t Size>
struct FixedSizeLine
{
    template <typename T>
    size_t Line(const T &host) const;
};

template <typename Operation>
struct LineSizeFromMaxSize
{
    template <typename T>
    size_t Line(const T &host) const;
};

// TestCase class declarations

struct NormalCase
{
    template <typename T>
    void Test(T &host);
};

struct ChangeDateCase
{
    template <typename T>
    void Test(T &host);
};

// ExpectedOutcome class declarations

template <bool Result>
struct FixedOutcome
{
    template <typename T>
    bool Expected(const T &host) const;
};

struct Outcome
{
    template <typename T>
    bool Expected(const T &host) const;
};

template <typename Creator,
          typename Params,
          typename FileInitializer,
          typename LineInitializer,
          typename ExpectedOutcome,
          typename TestCase>
struct Host :
    public Creator,
    public Params,
    public FileInitializer,
    public LineInitializer,
    public ExpectedOutcome,
    public TestCase
{
    void operator( )( );
};

struct SizeRestrictionTypes
{
    typedef SizeRestrictionCreator Creator;
    typedef SizeParams<128> Params;
    typedef Outcome DefaultOutcome;

    template <typename FileInit, typename Case>
    static void AddSizeCases(TestSuite *test)
    {
        typedef SizeRestrictionTypes Types;
        AddCase<Types, FileInit, FixedSizeLine<0>, Case>(test);
        AddCase<Types, FileInit, LineSizeFromMaxSize<NoOperator>, Case>(test);
        AddCase<Types, FileInit, LineSizeFromMaxSize<MinusOne>, Case>(test);
        AddCase<Types, FileInit, LineSizeFromMaxSize<PlusOne>, Case>(test);
        AddCase<Types, FileInit, LineSizeFromMaxSize<Double>, Case>(test);
        AddCase<Types, FileInit, LineSizeFromMaxSize<Half>, Case>(test);
    }
};

struct DateRestrictionTypes
{
    typedef DateRestrictionCreator Creator;
    typedef NoParams Params;
    typedef FixedOutcome<false> DefaultOutcome;

    template <typename FileInit, typename Case>
    static void AddSizeCases(TestSuite *test)
    {
        typedef DateRestrictionTypes Types;
        AddCase<Types, FileInit, FixedSizeLine<0>, Case>(test);
    }
};

TestSuite *init_unit_test_suite(int, char *[])
{
    TestSuite* test = BOOST_TEST_SUITE("Test suite for Restriction");

    AddFileStateCases<SizeRestrictionTypes, NormalCase>(test);
    AddFileStateCases<DateRestrictionTypes, NormalCase>(test);

    AddCase<DateRestrictionTypes, EmptyFile, FixedSizeLine<10>,
            ChangeDateCase>(test);

    return test;
}

// Helper function implementations

template <typename Types, typename Case>
void AddFileStateCases(TestSuite *test)
{
    test->add(NewCase(Host<typename Types::Creator, typename Types::Params,
                           FileClosed, FixedSizeLine<0>, FixedOutcome<false>,
                           Case>( )));
    test->add(NewCase(Host<typename Types::Creator, typename Types::Params,
                           FileClosed, FixedSizeLine<10>, FixedOutcome<false>,
                           Case>( )));

    Types::template AddSizeCases<EmptyFile, Case>(test);
    Types::template AddSizeCases<FileWithContent<16>, Case>(test);
    Types::template AddSizeCases<FileWithContent<32>, Case>(test);
    Types::template AddSizeCases<FileWithContent<64>, Case>(test);
    Types::template AddSizeCases<FileWithContent<128>, Case>(test);
    Types::template AddSizeCases<FileWithContent<256>, Case>(test);
}

template <typename Types, typename FileInitializer, typename Line,
         typename Case>
void AddCase(TestSuite *test)
{
    test->add(NewCase(Host<typename Types::Creator, typename Types::Params,
                           FileInitializer, Line,
                           typename Types::DefaultOutcome, Case>( )));
}

template <typename T>
boost::unit_test::test_case *NewCase(T caseFunction)
{
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
}

inline size_t Double::operator( )(size_t size) const
{
    return size * 2;
}

inline size_t Half::operator( )(size_t size) const
{
    return size / 2;
}

inline size_t NoOperator::operator( )(size_t size) const
{
    return size;
}

inline size_t MinusOne::operator( )(size_t size) const
{
    assert(size > 0);
    return size - 1;
}

inline size_t PlusOne::operator( )(size_t size) const
{
    return size + 1;
}

TestSetup::TestSetup( )
{
    boost::filesystem::remove_all(FILE_NAME);
}

TestSetup::~TestSetup( )
{
    boost::filesystem::remove_all(FILE_NAME);
}

// Creator class implementations

template <typename T>
inline RestrictionPtr
SizeRestrictionCreator::NewRestriction(const T &host) const
{
    return RestrictionPtr(new SizeRestriction(host.GetParam1( )));
}

template <typename T>
inline RestrictionPtr DateRestrictionCreator::NewRestriction(const T &) const
{
    return RestrictionPtr(new DateRestriction<DateCreatorForTests>);
}

// Params class implementations

template <size_t Size>
inline size_t SizeParams<Size>::GetParam1( ) const
{
    return Size;
}

// FileInitializer class implementations

template <typename T>
inline FilePtr FileClosed::GetFile(const T &) const
{
    class NonOpener : public Opener
    {
        virtual boost::filesystem::path DoOpen(std::filebuf &, Path&)
        {
            return "";
        }
    };

    Path path;
    return FilePtr(NonOpener( ).Open(path));
}

template <typename T>
inline FilePtr EmptyFile::GetFile(const T &) const
{
    Path path;
    path += FILE_NAME;
    return FilePtr(Creator( ).Open(path));
}

template <size_t Size>
template <typename T>
inline FilePtr FileWithContent<Size>::GetFile(const T &) const
{
    Path path;
    path += FILE_NAME;
    FilePtr file(Creator( ).Open(path));

    const std::string CONTENT(Size, 'a');

    BOOST_CHECK(Size == file->Write(CONTENT));

    return file;
}

// LineInitializer class implementations

template <size_t Size>
    template <typename T>
inline size_t FixedSizeLine<Size>::Line(const T &) const
{
    return Size;
}

template <typename Operation>
    template <typename T>
inline size_t LineSizeFromMaxSize<Operation>::Line(const T &host) const
{
    return Operation( )(host.GetParam1( ));
}

template <bool Result>
    template <typename T>
inline bool FixedOutcome<Result>::Expected(const T &) const
{
    return Result;
}

template <typename T>
inline bool Outcome::Expected(const T &host) const
{
    return (host.FILE_SIZE + host.Line(host) > host.GetParam1( ));
}

// TestCase class implementations

template <typename T>
void NormalCase::Test(T &host)
{
    RestrictionPtr restriction(host.NewRestriction(host));
    assert(restriction != 0);

    FilePtr file(host.GetFile(host));

    BOOST_CHECK_EQUAL(host.Expected(host),
                      restriction->IsRestricted(*file, host.Line(host)));
}

template <typename T>
void ChangeDateCase::Test(T &host)
{
    FilePtr file(host.GetFile(host));
    RestrictionPtr restriction(host.NewRestriction(host));

    BOOST_CHECK_EQUAL(host.Expected(host),
                      restriction->IsRestricted(*file, host.Line(host)));

    isNewDate = 1;

    BOOST_CHECK_EQUAL(true, restriction->IsRestricted(*file, host.Line(host)));
    BOOST_CHECK_EQUAL(false, restriction->IsRestricted(*file, host.Line(host)));
}

template <typename Creator,
          typename Params,
          typename FileInitializer,
          typename LineInitializer,
          typename ExpectedOutcome,
          typename TestCase>
void Host<Creator,
         Params,
         FileInitializer,
         LineInitializer,
         ExpectedOutcome,
         TestCase>::operator( )( )
{
    TestSetup setup;
    Test(*this);
}
