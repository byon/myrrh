// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class myrrh::file::Copy.
 *
 * $Id: Copy.hpp 354 2007-09-17 17:39:58Z byon $
 *
 */

#ifndef MYRRH_FILE_COPY_HPP_INCLUDED
#define MYRRH_FILE_COPY_HPP_INCLUDED

// Forward declaration should be enough
#include "myrrh/file/PositionScanner.hpp"
// Apparently not needed
#include "boost/filesystem/path.hpp"
// Not actually used?
#include "boost/shared_ptr.hpp"
#include <ios>

namespace myrrh
{
namespace file
{

/**
 * The class is intented for partial copying of a file to a new file. Although
 * it is possible to copy the entire file using myrrh::file::Copy, but it is
 * simpler to use boost::filesystem::copy_file function for that purpose.
 *
 * The amount copied depends on the result of given PositionScanner objects.
 * Thus the logic of determining what to copy is isolated from the logic of
 * actual copying.
 *
 * @note If you need to copy the entire file, it is much easier to use
 *       boost::filesystem::copy_file.
 */
class Copy
{
public:

    /**
     * Constructor. Operation is undefined if either of the given parameters
     * is null (debug builds will assert). Does not throw exceptions.
     * @warning The input stream must be opened using std::ios::binary or the
     *          result will not be exact. This is because windows carriage
     *          return is not counted into copied size otherwise.
     * @param startScanner Object that knows the beginning point of copying
     * @param endScanner Object that knows the end point of copying
     */
    Copy(PositionScannerPtr startScanner, PositionScannerPtr endScanner);

    /**
     * Copies the range defined by start and end scanners (given as
     * constructor parameters) from input stream to output stream.
     * @param input Stream from which input is read from
     * @param output The copying is done into this stream
     * @throws myrrh::file::Copy::StreamNotOpen, if the output stream is not
     *         open.
     * @throws myrrh::file::Copy::OutOfBounds, if the scan point from start
     *         scanner is after the scan point from end scanner
     * @throws myrrh::file::PositionScanner::NotOpen, the given input stream
     *         is not open.
     */
    void operator( )(std::ifstream &input, std::ofstream &output) const;

    /**
     * Exception class which is thrown, if given output file stream is not open
     */
    class StreamNotOpen : public std::runtime_error
    {
    public:

        StreamNotOpen(const std::string &what);
    };

    /**
     * Exception class which is thrown, if the range resulting from scanning
     * the input stream by position scanners is invalid (i.e. start is after
     * end)
     */
    class OutOfBounds : public std::runtime_error
    {
    public:

        OutOfBounds(const std::string &what);
    };

private:

    PositionScannerPtr startScanner_;
    PositionScannerPtr endScanner_;
};

}
}

#endif
