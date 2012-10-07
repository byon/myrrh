// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class myrrh::file::Temporary
 *
 * $Id: Temporary.hpp 286 2007-03-18 15:04:53Z Byon $
 */

#ifndef MYRRH_FILE_TEMPORARY_HPP_INCLUDED
#define MYRRH_FILE_TEMPORARY_HPP_INCLUDED

#include <iosfwd>
#include <stdexcept>
#include "boost/shared_ptr.hpp"

namespace boost { namespace filesystem { class path; } }

namespace myrrh
{

namespace file
{

/**
 * This class is usable in handling lifetimes of temporary files. At
 * construction, it creates a new temporary file that can be used for any
 * output. Once an object of this class is destructed, it not only closes the
 * file handle, it also deletes the file. This allows easier handling of
 * temporary files, which are needed for instance to make changes to existing
 * files in an exception safe manner. The class has a stream operator that can
 * be used for putting output into the file.
 * @note The class could have been derived from std::ostream, but it may
 *       have been a bit dangerous then to delete the file at Temporary
 *       destructor as std::ostream destructor is not yet called. Bad sides of
 *       this design decision are that we may require more access to the file
 *       stream object than currently is possible and that we cannot pass this
 *       class to algorithms that require std::ostream as a parameter.
 */
class Temporary
{
public:

    /**
     * This exception class is used to indicate that the path to the
     * TemporaryFile is invalid.
     */
    class PathError : public std::runtime_error
    {
    public:
        PathError(const std::string reason,
                  const boost::filesystem::path &path);
    };

    /**
     * Constructor
     * @param path A valid path name to the file to be used as a temporary
     *             file. A file with the same name can exist before this call
     *             in which case it will be written over. The results for
     *             using path to a file that is opened by another process are
     *             undefined (it seems boost::filesystem crashes in such a
     *             situation) @todo Find out the reason. If an empty name is
     *             used in debug build, an assertion will fail.
     * @throws boost::filesystem::filesystem_error, if the path name is empty,
     *         the path points to a directory or the file cannot be opened
     *         for some reason.
     */
    explicit Temporary(const boost::filesystem::path &path);

    /**
     * Gives write access to the underlying stream.
     * @return Reference to a stream opened into the temporary file.
     */
    std::ostream &Stream( );

    /**
     * Returns the path to the temporary file
     */
    boost::filesystem::path Path( ) const;

private:

    /// Disabled copy constructor
    Temporary(const Temporary &);
    /// Disabled assignment operator
    Temporary &operator=(const Temporary &);

    class Implementation;

    boost::shared_ptr<Implementation> implementation_;
};

}

}

#endif
