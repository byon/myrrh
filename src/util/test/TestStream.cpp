// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for myrrh/util/Stream.hpp
 *
 * $Id: TestStream.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/util/Stream.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/test/unit_test.hpp"

#ifdef WIN32
#pragma warning (pop)
#endif

const std::string MANIPULATOR1_HEADER("Manipulator1 called with '");
const std::string MANIPULATOR2_HEADER("Manipulator2 called with '");
const std::string MANIPULATOR3_HEADER("Manipulator3 called with '");
const std::string MANIPULATOR4_HEADER("Manipulator4 called with '");
const std::string AND_STRING("' and '");

template <typename T>
std::ostream &Manipulator1(std::ostream &stream, const T &argument)
{
    stream << MANIPULATOR1_HEADER << argument << '\'';
    return stream;
}

template <typename T>
std::ostream &Manipulator2(std::ostream &stream, const T &argument)
{
    stream << MANIPULATOR2_HEADER << argument << '\'';
    return stream;
}

template <typename T>
class Manipulator3
{
public:

    std::ostream &operator( )(std::ostream &stream, const T &argument) const
    {
        stream << MANIPULATOR3_HEADER << argument << '\'';
        return stream;
    }
};

template <typename T1, typename T2>
std::ostream &Manipulator1(std::ostream &stream, const T1 &argument1,
                           const T2 &argument2)
{
    stream << MANIPULATOR1_HEADER << argument1 << AND_STRING
           << argument2 << '\'';
    return stream;
}

template <typename T1, typename T2>
std::ostream &Manipulator2(std::ostream &stream, const T1 &argument1,
                           const T2 &argument2)
{
    stream << MANIPULATOR2_HEADER << argument1 << AND_STRING
           << argument2 << '\'';
    return stream;
}

template <typename T1, typename T2>
class Manipulator4
{
public:

    std::ostream &operator( )(std::ostream &stream, const T1 &argument1,
                              const T2 &argument2) const
    {
        stream << MANIPULATOR4_HEADER << argument1 << AND_STRING
               << argument2 << '\'';
        return stream;
    }
};

template <typename Func>
void TestOutput(Func func, const std::string &expected)
{
    std::ostringstream stream;
    func(stream);

    BOOST_CHECK_EQUAL(stream.str( ), expected);
}

void TestPlainManipulator1(std::ostream &stream)
{
    using namespace myrrh::util;
    stream << Manipulate(Manipulator1<int>, 1234) << std::endl;
    stream << Manipulate(Manipulator1<double>, 1234.1234) << '\n'
           << Manipulate(Manipulator1<std::string>, "A string") << std::endl;
}

void TestPlainManipulator2(std::ostream &stream)
{
    using namespace myrrh::util;
    stream << Manipulate(Manipulator2<int>, 1234) << '\n'
           << Manipulate(Manipulator2<double>, 1234.1234) << '\n'
           << Manipulate(Manipulator2<std::string>, "A string") << std::endl;
}

template <template <typename> class Func>
void TestPlainManipulatorClass(std::ostream &stream)
{
    using namespace myrrh::util;
    stream << Manipulate(Func<int>( ), 1234) << '\n'
           << Manipulate(Func<double>( ), 1234.1234) << '\n'
           << Manipulate(Func<std::string>( ), "A string") << std::endl;
}

void TestPlainManipulator1TwoParams(std::ostream &stream)
{
    using namespace myrrh::util;
    stream << Manipulate(Manipulator1<int, int>, 1234, 4321) << std::endl;
    stream << Manipulate(Manipulator1<double, int>, 1234.1234, 4356) << '\n'
           << Manipulate(Manipulator1<std::string, std::string>, "A string",
                         "Another string")
           << std::endl;
}

void TestPlainManipulator2TwoParams(std::ostream &stream)
{
    using namespace myrrh::util;
    stream << Manipulate(Manipulator2<int, int>, 1234, 4321) << '\n'
           << Manipulate(Manipulator2<int, double>, 9876, 1234.1234) << '\n'
           << Manipulate(Manipulator2<int, std::string>, 6789, "A string")
           << std::endl;
}

template <template <typename, typename> class Func>
void TestPlainManipulatorClassTwoParams(std::ostream &stream)
{
    using namespace myrrh::util;
    stream << Manipulate(Func<int, short>( ), 1234, static_cast<short>(4567))
           << '\n'
           << Manipulate(Func<double, double>( ), 1234.1234, 1234.1) << '\n'
           << Manipulate(Func<std::string, std::string>( ), "A string",
                         "Another string")
           << std::endl;
}

void TestItemsBeforeManipulator1(std::ostream & stream)
{
    using namespace myrrh::util;
    stream << "First a string followed by an integer " << 2345
           << " and another string followed by the manipulator "
           << Manipulate(Manipulator1<int>, 1234) << std::endl;
}

void TestItemsBeforeManipulator2(std::ostream & stream)
{
    using namespace myrrh::util;
    stream << "First a string followed by an integer " << 2345
           << " and another string followed by the manipulator "
           << Manipulate(Manipulator2<int>, 1234) << std::endl;
}

void TestItemsAfterManipulator1(std::ostream & stream)
{
    using namespace myrrh::util;
    stream << Manipulate(Manipulator1<int>, 1234)
           << "Manupulator followed by a string without whitespace in between"
           << std::endl;
}

void TestItemsAfterManipulator2(std::ostream & stream)
{
    using namespace myrrh::util;
    stream << Manipulate(Manipulator2<int>, 1234)
           << "Manupulator followed by a string without whitespace in between"
           << std::endl;
}

BOOST_AUTO_TEST_SUITE(DynamicSuite)

void TestManipulatorOne( )
{
    using namespace myrrh::util;

    const std::string MANIPULATOR1_RESULT(
        "Manipulator1 called with '1234'\n"
        "Manipulator1 called with '1234.12'\n"
        "Manipulator1 called with 'A string'\n");
    const std::string MANIPULATOR2_RESULT(
        "Manipulator2 called with '1234'\n"
        "Manipulator2 called with '1234.12'\n"
        "Manipulator2 called with 'A string'\n");
    const std::string MANIPULATOR3_RESULT(
        "Manipulator3 called with '1234'\n"
        "Manipulator3 called with '1234.12'\n"
        "Manipulator3 called with 'A string'\n");
    const std::string AFTER_MANIPULATOR1_RESULT(
        "Manipulator1 called with '1234'Manupulator followed by a string "
        "without whitespace in between\n");
    const std::string AFTER_MANIPULATOR2_RESULT(
        "Manipulator2 called with '1234'Manupulator followed by a string "
        "without whitespace in between\n");

    TestOutput(TestPlainManipulator1, MANIPULATOR1_RESULT);
    TestOutput(TestPlainManipulator2, MANIPULATOR2_RESULT);
    TestOutput(TestPlainManipulatorClass<Manipulator3>, MANIPULATOR3_RESULT);
    TestOutput(TestItemsAfterManipulator1, AFTER_MANIPULATOR1_RESULT);
    TestOutput(TestItemsAfterManipulator2, AFTER_MANIPULATOR2_RESULT);
}

void TestManipulatorTwo( )
{
    const std::string MANIPULATOR1_RESULT(
        "Manipulator1 called with '1234' and '4321'\n"
        "Manipulator1 called with '1234.12' and '4356'\n"
        "Manipulator1 called with 'A string' and 'Another string'\n");
    const std::string MANIPULATOR2_RESULT(
        "Manipulator2 called with '1234' and '4321'\n"
        "Manipulator2 called with '9876' and '1234.12'\n"
        "Manipulator2 called with '6789' and 'A string'\n");
    const std::string MANIPULATOR4_RESULT(
        "Manipulator4 called with '1234' and '4567'\n"
        "Manipulator4 called with '1234.12' and '1234.1'\n"
        "Manipulator4 called with 'A string' and 'Another string'\n");

    TestOutput(TestPlainManipulator1TwoParams, MANIPULATOR1_RESULT);
    TestOutput(TestPlainManipulator2TwoParams, MANIPULATOR2_RESULT);
    TestOutput(TestPlainManipulatorClassTwoParams<Manipulator4>,
               MANIPULATOR4_RESULT);
}

BOOST_AUTO_TEST_SUITE_END( )
