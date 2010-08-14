// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the declaration of class myrrh::file::SafeModify
 *
 * $Id: SafeModify.hpp 356 2007-09-18 19:55:21Z byon $
 */

#ifndef MYRRH_UTILITY_TEMPORARYFILE_HPP_INCLUDED
#define MYRRH_UTILITY_TEMPORARYFILE_HPP_INCLUDED

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include <string>

namespace myrrh
{

namespace file
{

/**
 * This class is usable for copying parts of an existing file in an
 * exception safe way. The file is first renamed with a temporary file name.
 * Then the needed parts can be copied from the temporary file to a new file
 * that is named with the original name. Once the operation is done, the
 * original file can be deleted. If an exception occurs during the operation,
 * the original file will be renamed with original name (overriding possible
 * changes to the new file), i.e. the file remains unchanged.
 * @note If the operation goes as expected (i.e. no exceptions), the client
 *       must manually call Commit function or all of the changes will be
 *       reverted. This is analogous to making a commit to database after
 *       changes.
 * @note An object of this class is only usable for one file copy.
 */
class SafeModify
{
public:

    /**
     * Constructor.
     * @param original Path to the original file
     * @throws boost::filesystem::filesystem_error if given path is empty or
     *         points to a non-existing file or there is a directory that is
     *         named like result of SafeModify::Name(original).
     */
    SafeModify(const boost::filesystem::path &original);

    /**
     * Destructor. Ensures that if an exception occurs and the changes have
     * not been committed by a call to Commit( ), the changes will be reverted
     * back to the original state.
     */
    ~SafeModify( );

    /**
     * Returns a name of the temporary file built from the original
     * @param original The path from which to build the temporary name
     * @return A valid path to the temporary file. Note that unless an object
     *         of SafeModify with the same parameter has not been
     *         constructed, there is no guarantee that the path points to an
     *         existing file.
     * @throws Whatever exceptions std::string copy constructor or operator+
     *         may throw
     */
    static std::string Name(const boost::filesystem::path &original);

    /**
     * Call this function, when the copying to the new file has been made. If
     * this function has not been called before destruction, the copied file
     * will be reverted back to original state. Cannot throw.
     */
    void Commit( ) const;

private:

    /// Copy constructor declared private to prevent unintended use
    SafeModify(const SafeModify&);
    /// Assignement operator declared private to prevent unintended use
    SafeModify &operator=(SafeModify&);

    /// Path to the original file
    const boost::filesystem::path ORIGINAL_;
    /// Path to the temporary file
    const boost::filesystem::path TEMPORARY_;
    /// Prefix used to build the temporary file name
    static const std::string FILE_TEMPORARY_PREFIX_;
};

// inline implementations

inline void SafeModify::Commit( ) const
{
    // The interface documentation promises that this method cannot throw.
    // The following call CAN throw, but only in situations that are not
    // possible if SafeModify works correctly.
    boost::filesystem::remove(TEMPORARY_);
}

inline std::string SafeModify::Name(const boost::filesystem::path &original)
{
    return original.string( ) + FILE_TEMPORARY_PREFIX_;
}

}

}

#endif
