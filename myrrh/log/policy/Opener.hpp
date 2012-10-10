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

#ifndef MYRRH_LOG_POLICY_OPENER_HPP_INCLUDED
#define MYRRH_LOG_POLICY_OPENER_HPP_INCLUDED

#include "boost/shared_ptr.hpp"

#include <fstream>
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
/// Move to own header
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
    // constructed. Because of this friend connection, the File and Opener
    // classes must not be extracted to separate files. Otherwise it could be
    // possible to breach the class encapsulation.
    friend class Opener;

    /**
     * Constructor.
     * Provides no-throw guarantee, assuming that the Opener subclass is
     * implemented correctly (i.e. does not throw exceptions).
     * @param The opener object that will be used to open the file.
     */
    File(Opener &opener, policy::Path& path);

    static boost::filesystem::path TryOpening(Opener &opener,
                                              policy::Path &path,
                                              std::ofstream &file);

    /// Disabled copy constructor
    File(const File &);
    /// Disabled assignment operator
    File &operator=(const File &);

    // Stream opened to the file
    std::ofstream file_;
    // Current size of the file
    std::streamsize writtenSize_;
    // Path to the file
    const boost::shared_ptr<boost::filesystem::path> path_;
};

typedef boost::shared_ptr<File> FilePtr;

/**
 * An NVI interface class that provides a way to create new File objects. The
 * actual opening mechanism must be implemened in subclasses.
 */
class Opener
{
public:

    /**
     * Virtual destructors are required in interface classes
     */
    virtual ~Opener( );

    /**
     * Opens a new File object that can be written to.
     * Provides no-throw guarantee.
     * @param path Contains the rules, which describe how to locate and name
     *             the new file.
     * @returns A shared_ptr containing the new File object. Can return 0, if
     *          there is not enough memory to create the object.
     */
    FilePtr Open(Path path);

private:

    // Friend access is needed by File class to get access to DoOpen method.
    // Because of this friend connection, the File and Opener classes must not
    // be extracted to separate files. Otherwise it could be possible to breach
    // the class encapsulation.
    friend class File;

    virtual boost::filesystem::path DoOpen(std::filebuf &file, Path& path) = 0;
};

typedef boost::shared_ptr<Opener> OpenerPtr;

/**
 * The purpose of this class is to separate Openers that are able to create
 * new files from Openers that require the file to exist before operating on
 * it. The first ones should derive from this class and the rest directly from
 * Opener.
 * With this explicit division we can catch possible errors during compilation
 * when an object of class that is that not able to create a new file is passed
 * to algorithm that requires that capability (currently this references to the
 * constructor of Policy).
 */
class InitialOpener : public Opener
{
};

typedef boost::shared_ptr<InitialOpener> InitialOpenerPtr;

}

}

}

#endif
