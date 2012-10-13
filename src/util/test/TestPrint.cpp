// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for Print
 *
 * $Id: TestPrint.cpp 355 2007-09-17 18:48:35Z byon $
 */

#include "myrrh/util/Print.hpp"

#define BOOST_TEST_MODULE TestPrint
#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#define DISABLE_COPY_CONSTRUCTOR_COULD_NOT_BE_GENERATED
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#define DISABLE_USED_WITHOUT_INITIALIZED
#define DISABLE_QUALIFIER_APPLIED_TO_REFERENCE
#include "myrrh/util/Preprocessor.hpp"

#include "boost/lexical_cast.hpp"
#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"
#include "boost/tokenizer.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

#include <deque>
#include <list>
#include <map>
#include <set>
#include <vector>

typedef boost::unit_test::test_suite TestSuite;

namespace
{

class OwnClass
{
public:

    OwnClass( )
    {
    }

    explicit OwnClass(const std::string &id) :
        id_(id)
    {
    }

    friend bool operator==(const OwnClass &left, const OwnClass &right)
    {
        return left.id_ == right.id_;
    }

    friend std::ostream &operator<<(std::ostream &stream, const OwnClass &own)
    {
        stream << HEADER_ << own.id_;
        return stream;
    }

    friend std::istream &operator>>(std::istream &stream, OwnClass &own)
    {
        stream.ignore(static_cast<std::streamsize>(HEADER_.size( )));
        stream >> std::noskipws >> own.id_;
        return stream;
    }

private:

    std::string id_;
    static const std::string HEADER_;
};

const std::string OwnClass::HEADER_("OwnClass:");

template <typename T>
class Generator
{
};

template <>
class Generator<int>
{
public:

    int operator( )( )
    {
        return std::rand( );
    }
};

template <>
class Generator<double>
{
public:

    double operator( )( )
    {
        return std::rand( ) / (std::rand( ) + 1.0);
    }
};

template <>
class Generator<std::string>
{
public:

    std::string operator( )( )
    {
        return "abcdefghijklmnopqrstuvxyzåäö";
    }
};

template <>
class Generator<OwnClass>
{
public:

    OwnClass operator( )( )
    {
        Generator<std::string> stringGenerator;
        return OwnClass(stringGenerator( ));
    }
};

template <typename T1, typename T2>
class PairGenerator
{
public:

    std::pair<T1, T2> operator( )( )
    {
        Generator<T1> generator1;
        Generator<T2> generator2;
        return std::make_pair(generator1( ), generator2( ));
    }
};

template <typename T>
void CheckValues(const T &value1, const std::string &value2)
{
    T convertedValue2(boost::lexical_cast<T>(value2));
    BOOST_CHECK_EQUAL(value1, convertedValue2);
}

template <>
void CheckValues<double>(const double &value1,
                         const std::string &value2)
{
    double convertedValue2(boost::lexical_cast<double>(value2));
    BOOST_CHECK_CLOSE(value1, convertedValue2, 0.001);
}

template <typename T1, typename T2>
std::istream &operator>>(std::istream &is, std::pair<T1, T2> &pair)
{
    char separator = 0;
    is >> pair.first >> separator >> pair.second;
    return is;
}

template <typename T1, typename T2>
void CheckValues(const std::pair<T1, T2> &value1, const std::string &value2)
{
    typedef typename boost::remove_const<T1>::type NonConstT1;
    std::pair<NonConstT1, T2> convertedValue2;
    std::istringstream stream(value2);
    stream >> convertedValue2;
    //T convertedValue2(boost::lexical_cast<T>(value2));
    BOOST_CHECK_EQUAL(value1, convertedValue2);
}

template <typename T>
class IsItemLegal
{
public:

    IsItemLegal(char)
    {
    }

    void operator( )(const T &) const
    {
    }
};

template <>
class IsItemLegal<std::string>
{
public:

    IsItemLegal(char delimiter) :
        DELIMITER_(delimiter)
    {
    }

    void operator( )(const std::string &item) const
    {
        BOOST_CHECK_EQUAL(std::string::npos, item.find(DELIMITER_));
    }
private:

    IsItemLegal &operator=(const IsItemLegal &);

    const char DELIMITER_;
};

template <typename T>
void TestRangeResults(const T &begin, const T &end, const std::string &toTest,
                      const std::string &delimiter)
{
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

    boost::char_separator<char> separator(delimiter.c_str( ));
    tokenizer tokens(toTest, separator);
    T containerIterator = begin;

    for (tokenizer::iterator i = tokens.begin( ); i != tokens.end( ); ++i)
    {
        CheckValues(*containerIterator, *i);
        ++containerIterator;
    }

    BOOST_CHECK(containerIterator == end);
}

template <typename T>
void TestPrintRange(const T &begin, const T &end, const std::string &delimiter)
{
    std::ostringstream stream;
    stream << Manipulate(myrrh::util::Print(delimiter), begin, end);
    TestRangeResults(begin, end, stream.str( ), delimiter);
}

template <typename T>
void TestPrintRangeImpl(const T &container, const std::string &delimiter)
{
    BOOST_REQUIRE(delimiter.size( ) == 1);
    IsItemLegal<typename T::value_type> itemChecker(delimiter[0]);

    std::for_each(container.begin( ), container.end( ), itemChecker);

    TestPrintRange(container.begin( ), container.end( ), delimiter);

    const int MIN_SIZE_TO_TEST_HALF_WAY = 3;
    if (container.size( ) < MIN_SIZE_TO_TEST_HALF_WAY)
    {
        return;
    }

    const int HALF_WAY = static_cast<int>(container.size( ) / 2);
    typename T::const_iterator halfWayIterator(container.begin( ));
    std::advance(halfWayIterator, HALF_WAY);
    BOOST_REQUIRE(container.end( ) != halfWayIterator);

    TestPrintRange(halfWayIterator, container.end( ), delimiter);
    TestPrintRange(container.begin( ), halfWayIterator, delimiter);
}

template <typename T>
void TestPrintContainer(const T &container, const std::string &header,
                        const std::string &delimiter)
{
    std::ostringstream stream;
    const std::string SEPARATOR(": ");
    stream << header << SEPARATOR
           << Manipulate(myrrh::util::Print(delimiter), container);

    const std::string OUTPUT(stream.str( ));
    const size_t HEADER_SIZE = header.size( ) + SEPARATOR.size( );
    const std::string TESTED_HEADER(OUTPUT.substr(0, HEADER_SIZE));
    BOOST_CHECK_EQUAL(header + SEPARATOR, TESTED_HEADER);
    const std::string TESTED_DATA(OUTPUT.substr(HEADER_SIZE));

    TestRangeResults(container.begin( ), container.end( ),
                     TESTED_DATA, delimiter);
}

template <typename T>
void TestHeaders(const T &container, const std::string &delimiter)
{
    TestPrintContainer(container, "", delimiter);
    TestPrintContainer(container, "A_header", delimiter);
    TestPrintContainer(container, "a header: ", delimiter);
    TestPrintContainer(container, "diiba daaba duuba daaba", delimiter);
}

template <typename Func, typename T>
void TestDelimiters(Func func, const T &container)
{
    func(container, " ");
    func(container, ",");
    func(container, "*");
}

template <typename T>
T NewSequenceContainer(int size)
{
    T container(size);
    Generator<typename T::value_type> generator;
    std::generate_n(container.begin( ), size, generator);

    return container;
}

template <typename T1, typename T2>
std::map<T1, T2> NewMap(int size)
{
    std::map<T1, T2> map;
    PairGenerator<T1, T2> generator;
    std::generate_n(std::inserter(map, map.begin( )), size, generator);

    return map;
}

template <typename T>
std::set<T> NewSet(int size)
{
    std::set<T> set;
    Generator<T> generator;
    std::generate_n(std::inserter(set, set.begin( )), size, generator);

    return set;
}

template <typename T, typename Function>
void TestRange(int size, Function function)
{
    TestDelimiters(TestPrintRangeImpl<T>, function(size));
}

template <typename T>
void TestWithSequenceContainer(int size)
{
    TestDelimiters(TestHeaders<T>, NewSequenceContainer<T>(size));
}

template <typename T>
void TestRangeContainerTypes(int size)
{
    TestRange<std::deque<T> >(size, NewSequenceContainer<std::deque<T> >);
    TestRange<std::list<T> >(size, NewSequenceContainer<std::list<T> >);
    TestRange<std::vector<T> >(size, NewSequenceContainer<std::vector<T> >);

    // Note: Unfortunately I've not been able to get the printing work with
    //       std::map. It seems though that if I add a <<operator for
    //       std::pair, I'm able to print the std::map contents if I do not
    //       call Print( ) from a template function such as this. It is as
    //       if the type of the iterators would get mixed up with all these
    //       template functions?
    //TestRange<std::map<T, T> >(size, NewMap<T, T>);
    TestRange<std::set<T> >(size, NewSet<T>);
}

std::vector<int> GetTestCounts( )
{
    std::vector<int> testCounts;
    testCounts.push_back(0);
    testCounts.push_back(1);
    testCounts.push_back(2);
    testCounts.push_back(3);
    testCounts.push_back(5);
    testCounts.push_back(10);
    testCounts.push_back(100);

    return testCounts;
}

template <typename T>
void TestContainerTypes(int size)
{
    TestWithSequenceContainer<std::deque<T> >(size);
    TestWithSequenceContainer<std::list<T> >(size);
    TestWithSequenceContainer<std::vector<T> >(size);
}

}

BOOST_AUTO_TEST_SUITE(TestPrint)

BOOST_AUTO_TEST_CASE(DoTestPrintRange)
{
    std::vector<int> testCounts(GetTestCounts( ));
    typedef std::vector<int>::iterator IntIter;
    IntIter begin(testCounts.begin( ));
    IntIter end(testCounts.end( ));

    std::for_each(begin, end, TestRangeContainerTypes<int>);
    std::for_each(begin, end, TestRangeContainerTypes<double>);
    std::for_each(begin, end, TestRangeContainerTypes<std::string>);
}

BOOST_AUTO_TEST_CASE(DoTestPrintContainer)
{
    std::vector<int> testCounts(GetTestCounts( ));
    typedef std::vector<int>::iterator IntIter;
    IntIter begin(testCounts.begin( ));
    IntIter end(testCounts.end( ));

    std::for_each(begin, end, TestContainerTypes<int>);
    std::for_each(begin, end, TestContainerTypes<double>);
    std::for_each(begin, end, TestContainerTypes<std::string>);
    std::for_each(begin, end, TestContainerTypes<OwnClass>);
}

BOOST_AUTO_TEST_SUITE_END( )
