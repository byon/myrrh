// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class myrrh::file::Resizer
 *
 * $Id: Resizer.hpp 362 2007-09-20 17:48:34Z byon $
 */

#ifndef MYRRH_FILE_RESIZER_H_INCLUDED
#define MYRRH_FILE_RESIZER_H_INCLUDED

#include "boost/shared_ptr.hpp"

namespace boost { namespace filesystem { class path; } }

namespace myrrh
{

namespace file
{

class PositionScanner;
typedef boost::shared_ptr<PositionScanner> PositionScannerPtr;

/**
 * Resizer class is used to resize an existing file to a new size according to
 * the logic specified the given PositionScanner objects. For instance to
 * resize a file to 10 kB from start (+ rest of line, see @ScanFromStart), use
 * the following code:
 * @code
 *   PositionScannerPtr startScanner(new StartScanner);
 *   PositionScannerPtr endScanner(new ScanFromStart(10000));
 *   Resizer resizer("file.txt", startScanner, endScanner);
 *   resizer( );
 * @endcode
 *
 * The logic of finding the correct range to resize to is extracted to separate
 * PositionScanner classes, because then we have a clear logic for resizing
 * the file regardless of the logic of choosing the range. That way the
 * PositionScanner classes are also usable outside the context of resizing.
 */
class Resizer
{
public:

    /**
     * Constructor.
     * @param file Path to the file to be resized
     * @param startScanner Object that defines the beginning position in the
     *                     range in file that will be left into the resized
     *                     file.
     * @param endScanner Object that defines the end position in the range in
     *                   file that will be left into the resized file.
     */
    Resizer(const boost::filesystem::path &file,
            PositionScannerPtr startScanner, PositionScannerPtr endScanner);

    /**
     * Does the resizing.
     * @note The operation is strongly exception-safe. If exceptions occur
     *       during resizing, the original file is left untouched.
     * @throws myrrh::file::PositionScanner::NotOpen, if the file defined in
     *         the constructor does not exist.
     */
    void operator( )( ) const;

    class NoFile : public std::runtime_error
    {
    public:
        explicit NoFile(const boost::filesystem::path &path);
    };

    class CannotOpen : public std::runtime_error
    {
    public:
        explicit CannotOpen(const boost::filesystem::path &path);
    };

private:

    class Implementation;
    boost::shared_ptr<Implementation> implementation_;
};

}

}

#endif
