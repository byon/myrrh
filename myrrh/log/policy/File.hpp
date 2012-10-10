// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the declaration of classes myrrh::log::policy::File,
 * myrrh::log::policy::Opener and myrrh::log::policy::InitialOpener.
 *
 * $Id: Opener.hpp 354 2007-09-17 17:39:58Z byon $
 */

#ifndef MYRRH_LOG_POLICY_FILE_HPP_INCLUDED
#define MYRRH_LOG_POLICY_FILE_HPP_INCLUDED

#include "boost/shared_ptr.hpp"
#include <string>

namespace boost { namespace filesystem { class path; } }

namespace myrrh
{

namespace log
{

namespace policy
{

class Opener;
class Path;

/**
 * File class encapsulates the functionality relating to writing of data into
 * a physical file and storing the data of the write operations.
 *
 * Note that it is not possible to construct File objects outside from Opener
 * subclasses.
 *
 * By adding boost::equality_comparable<File> as base class, we'll get the
 * inequality operator.
 */
/// Convert into polymorphic class, so the tests do not require IO?
class File
{
public:

    /**
     * Writes the given string to a file.
     * Provides no-throw guarantee
     * @param line The string to be written
     * @return The size written to file
     */
    std::streamsize Write(const std::string &line);

    /**
     * Returns the size that has already been written to the file during the
     * previous write operations (or before opening the file, if we are
     * appending to existing file).
     * @return The size written so far
     */
    std::streamsize WrittenSize( ) const;

    const boost::filesystem::path &Path( ) const;

    /**
     * Comparison operator
     * @param left The File object on the left size of comparison
     * @param left The File object on the right size of comparison
     * @return true, if objects are pointing to the same physical file.
     */
    /// Potentially remove the friend status and move outside of class
    friend bool operator==(const File &left, const File &right);
    friend bool operator!=(const File &left, const File &right);

private:

    // Friend access is needed to Opener, so that the File objects can be
    // constructed.
    friend class Opener;

    File(Opener &opener, policy::Path& path);

    File(const File &);
    File &operator=(const File &);

    class Implementation;

    boost::shared_ptr<Implementation> implementation_;
};

typedef boost::shared_ptr<File> FilePtr;

}

}

}

#endif
