// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of function myrrh::util::GenerateOutput and
 * myrrh::util::WriteFailed exception class.
 *
 * $Id: GenerateOutput.hpp 354 2007-09-17 17:39:58Z byon $
 */

#ifndef MYRRH_UTIL_GENERATEOUTPUT_H_INCLUDED
#define MYRRH_UTIL_GENERATEOUTPUT_H_INCLUDED

#include <iosfwd>
#include <stdexcept>

namespace myrrh
{
namespace util
{

// This is used for generating test data. Better designed tests would be
// able to either use string literals specific to the test or use
// std::string constructor with size parameter for large strings. As a result
// the uses of this should be removed and then this could be removed.

/**
 * Generates large size of output from given input stream. The contents of the
 * given input stream is written into the output stream until the required
 * data size is reached. If the input stream has more data than is required,
 * only the required amount is copied.
 * @param input Stream from which the input is read. If the stream contains no
 *              data to be read, the function does nothing.
 * @param output Stream into which the output is read
 * @param outputSize The required size of output
 * @throws WriteFailed, if cannot write everything. Note that the function
 *         provides only basic exception safety. The data, which is already
 *         written, when the error occurs, remains written.
 */
void GenerateOutput(std::istream &input, std::ostream &output,
                    std::streamsize outputSize);

/**
 * Exception class for situations in which the output fails.
 */
class WriteFailed : public std::runtime_error
{
public:

    explicit WriteFailed(const std::string &what);
};

}
}

#endif
