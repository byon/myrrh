// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Copyif
 *
 * $Id: TestCopyif.cpp 354 2007-09-17 17:39:58Z byon $
 */

#include "myrrh/util/CopyIf.hpp"
#include "myrrh/util/RandomString.hpp"

#define DISABLE_SIGNED_UNSIGNED_MISMATCH
#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <vector>

typedef boost::unit_test::test_suite TestSuite;

template <bool Result, typename T>
bool Always(const T &)
{
    return Result;
}

template <typename T>
class Test
{
public:

    Test(const T &testAgainst) :
        testAgainst_(testAgainst)
    {
    }

    Test(const Test &orig) :
        testAgainst_(orig.testAgainst_)
    {
    }

    bool operator( )(const T &toTest)
    {
        return (toTest < testAgainst_);
    }

private:

    Test &operator=(const Test &);

    T testAgainst_;
};

template <class T>
class Generator;

template <>
class Generator<int>
{
public:

    typedef int Type;

    int operator( )( )
    {
        return std::rand( );
    }
};

template <>
class Generator<std::string>
{
public:

    typedef std::string Type;

    std::string operator( )( )
    {
        return myrrh::util::GetRandomString(20);
    }
};

template <typename T1, typename T2>
class PairGenerator
{
public:

    typedef std::pair<T1, T2> Type;

    std::pair<T1, T2> operator( )( )
    {
        Generator<T1> generator1;
        Generator<T2> generator2;
        return std::make_pair(generator1( ), generator2( ));
    }
};

template <typename SourceIterator, typename Target, typename Predicate>
void TestCopyIf(const SourceIterator &begin, const SourceIterator &end,
                const Target &expected, Predicate predicate)
{
    Target target;
    myrrh::util::CopyIf(begin, end, std::inserter(target, target.begin( )),
                        predicate);

    BOOST_CHECK(target == expected);
}

template <typename Target, typename SourceIterator, typename Predicate>
void GenerateExpectedAndCompare(const SourceIterator &begin,
                                const SourceIterator &end,
                                Predicate predicate)
{
    Target expected;

    for (SourceIterator i = begin; i != end; ++i)
    {
        if (predicate(*i))
        {
            std::inserter(expected, expected.end( )) = *i;
        }
    }

    TestCopyIf(begin, end, expected, predicate);
}

template <typename Target, typename Source, typename Predicate>
void TestRanges(const Source &source, Predicate predicate)
{
    Source::const_iterator begin = source.begin( );
    Source::const_iterator end = source.end( );
    GenerateExpectedAndCompare<Target>(begin, end, predicate);

    const int MIN_SIZE_TO_TEST_HALF_WAY = 3;
    if (source.size( ) < MIN_SIZE_TO_TEST_HALF_WAY)
    {
        return;
    }

    const int HALF_WAY = static_cast<int>(source.size( ) / 2);
    Source::const_iterator halfWayIterator(source.begin( ));
    std::advance(halfWayIterator, HALF_WAY);
    BOOST_REQUIRE(source.end( ) != halfWayIterator);

    GenerateExpectedAndCompare<Target>(begin, halfWayIterator, predicate);
    GenerateExpectedAndCompare<Target>(halfWayIterator, end, predicate);
}

template <typename Target, typename Source = Target,
          typename Gener = Generator<Target::value_type> >
class TestContainer
{
public:

    typedef typename Target::value_type Type;
    typedef Gener TypeGenerator;

    explicit TestContainer(int maxToTest) :
        MAX_TO_TEST_(maxToTest)
    {
    }

    template <typename Predicate>
    void operator( )(Predicate predicate,
                     TypeGenerator generator = Gener( )) const
    {
        for (int i = 0; i < MAX_TO_TEST_; ++i)
        {
            using namespace std;
            Source source;
            generate_n(inserter(source, source.begin( )), i, generator);
            TestRanges<Target>(source, predicate);
        }
    }

private:

    TestContainer(const TestContainer &);
    TestContainer &operator=(const TestContainer &);

    const int MAX_TO_TEST_;
};

template <typename Tester>
void TestPredicate(const Tester &tester)
{
    tester(Always<true, Tester::Type>);
    tester(Always<false, Tester::Type>);
    Tester::TypeGenerator generator;
    tester(Test<Tester::Type>(generator( )));
}

template <typename T>
void TestNonPairContainer(int testCount)
{
    using namespace std;

    TestPredicate(TestContainer<vector<T> >(testCount));
    TestPredicate(TestContainer<vector<T>, list<T> >(testCount));
    TestPredicate(TestContainer<vector<T>, deque<T> >(testCount));
    TestPredicate(TestContainer<list<T> >(testCount));
    TestPredicate(TestContainer<list<T>, vector<T> >(testCount));
    TestPredicate(TestContainer<list<T>, deque<T> >(testCount));
    TestPredicate(TestContainer<deque<T> >(testCount));
    TestPredicate(TestContainer<deque<T>, vector<T> >(testCount));
    TestPredicate(TestContainer<deque<T>, list<T> >(testCount));
}

template <typename T1, typename T2>
void TestMap(int testCount)
{
    typedef std::map<T1, T2> Map;
    TestPredicate(TestContainer<Map, Map, PairGenerator<T1, T2> >(testCount));
}

void TestCopyIf( )
{
    const int TEST_COUNT = 50;
    TestNonPairContainer<int>(TEST_COUNT);
    TestNonPairContainer<std::string>(TEST_COUNT);

    TestMap<int, int>(TEST_COUNT);
    TestMap<std::string, std::string>(TEST_COUNT);
    TestMap<int, std::string>(TEST_COUNT);
    TestMap<std::string, int>(TEST_COUNT);
}

TestSuite *init_unit_test_suite(int, char *[])
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    TestSuite* test =
        BOOST_TEST_SUITE("Test suite for Copyif");
    test->add(BOOST_TEST_CASE(TestCopyIf));
    return test;
}
