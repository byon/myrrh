// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * File: myrrh/utility/Print.hpp
 * Author: Marko Raatikainen
 *
 * Contains declaration of class myrrh::util::Print
 *
 * $Id: Print.hpp 355 2007-09-17 18:48:35Z byon $
 */

#ifndef MYRRH_UTILITY_PRINT_HPP_INCLUDED
#define MYRRH_UTILITY_PRINT_HPP_INCLUDED

#include "myrrh/util/Stream.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

namespace myrrh
{

namespace util
{

/**
 * This is a helper class for printing the contents of STL ranges and
 * containers into output stream. Also other ranges and containers can be
 * printed if they follow the STL iterator/container interfaces. The class can
 * be used by itself, but it is more natural to use like a stream manipulator.
 * This can be accomplished by using the class together with
 * myrrh::util::Manipulate. For example like this:
 *
 * @code
 *     std::vector<int> intVector;
 *     std::map<std::string, std::string> stringMap;
 *     /// ... fill in the data structures
 *     std::cout << Manipulate(myrrh::util::Print( ), intVector) << '\n'
 *               << Manipulate(myrrh::util::Print( ), stringMap.begin( ),
 *                             stringMap.end( ))
 *               << std::endl;
 * @endcode
 *
 * The output is written in one line. The items are separated by a delimiter
 * that is user definable, but that has a default value of one whitespace. The
 * output is not followed by an end of line.
 *
 * @warning I've not been able to build test driver that works with std::map
 *          containers. For some non-template situation the code compiles even
 *          with std::map, but as there has been no test driver, I would not
 *          trust the code to work.
 */
// Do I use the range printing version enough to warrant the implementation?
// For the container printing I could have a template function instead.
// Actually the whole class is not used anywhere, just remove this.
class Print
{
public:

    /**
     * Constructor.
     * @param Delimiter The string that separates the container items from
     *                  each other. Default value is one whitespace.
     */
    explicit Print(const std::string &delimiter = " ");

    /**
     * Outputs the contents of given range to given output stream. Delimiter
     * defined as constructor parameter is used, except after the last item in
     * the range.
     * @param stream The output stream, where the output will be targeted
     * @param begin First iterator in range
     * @param end Iterator, which is one after the last in range
     * @throws Any exception that can be thrown by output
     * @returns stream parameter -> can be used in sequential << operators
     */
    template <typename Iterator>
    std::ostream &operator( )(std::ostream &stream, const Iterator &begin,
                              const Iterator &end) const;

    /**
     * Outputs the contents of given container to given output stream.
     * Delimiter defined as constructor parameter is used, except after the
     * last item in the container.
     * @param stream The output stream, where the output will be targeted
     * @param container The container to be printed to output
     * @throws Any exception that can be thrown by output
     * @returns stream parameter -> can be used in sequential << operators
     */
    template <typename Container>
    std::ostream &operator( )(std::ostream &stream,
                              const Container &container) const;

private:

    /**
     * Implements the range printing. The functor operators forward the
     * responsibility to this function.
     * @param stream The output stream, where the output will be targeted
     * @param begin First iterator in range
     * @param end Iterator, which is one after the last in range
     * @throws Any exception that can be thrown by output
     */
    template <typename Iterator>
    std::ostream &PrintRange(std::ostream &stream, const Iterator &begin,
                             const Iterator &end) const;

    /// Disabled assignment operator
    Print &operator=(const Print &);

    /// The delimiter string
    const std::string DELIMITER_;
};

// Inline implementations

Print::Print(const std::string &delimiter) :
    DELIMITER_(delimiter)
{
}

template <typename Iterator>
inline std::ostream &Print::operator( )(std::ostream &stream,
                                        const Iterator &begin,
                                        const Iterator &end) const
{
    return PrintRange(stream, begin, end);
}

template <typename Container>
inline std::ostream &Print::operator( )(std::ostream &stream,
                                        const Container &container) const
{
    return PrintRange(stream, container.begin( ), container.end( ));
}

template <typename Iterator>
inline std::ostream &Print::PrintRange(std::ostream &stream,
                                       const Iterator &begin,
                                       const Iterator &end) const
{
    // Could be extracted to be smaller
    if (begin != end)
    {
        typedef typename Iterator::value_type Value;
        typedef std::ostream_iterator<Value> OutputIterator;

        // We print the items except the last. The last will be printed
        // separately, because then we will avoid the unneeded delimiter
        // at end.
        Iterator lastNode(end);
        --lastNode;

        std::copy(begin, lastNode,
                  OutputIterator(stream, DELIMITER_.c_str( )));

        stream << *lastNode;
    }

    return stream;
}

}

}

#endif
