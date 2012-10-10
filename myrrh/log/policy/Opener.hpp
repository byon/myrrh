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

class File;
class Path;

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
