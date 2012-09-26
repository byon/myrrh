// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration and implementation of class
 * myrrh::file::Eraser
 *
 * $Id: Eraser.hpp 286 2007-03-18 15:04:53Z Byon $
 */

#ifndef MYRRH_FILE_ERASER_HPP_INCLUDED
#define MYRRH_FILE_ERASER_HPP_INCLUDED

#include "boost/shared_ptr.hpp"

namespace boost { namespace filesystem { class path; } }

namespace myrrh
{

namespace file
{

/**
 * This class is usable when you wish to ensure strong exception safety when
 * handling files/directories. For instance if you have a function that will
 * need to create a file with some content and exceptions may occur in between
 * of write operations. If strong exception safety is required, you need to
 * delete the file in case of an exception. This class provides an utility to
 * do this.
 *
 * At construction, a path to the file is given to object of this class. If
 * everything goes without error, the user calls Release method once he is
 * done with his operations. The destructor does nothing. If the Release method
 * is not called, it is assumed an error occured and the file will be erased
 * at destructor. The error may be either an user error (forgotten call to
 * Release) or an occurred exception.
 *
 * Of course it may be the user's intention that the file gets erased always.
 * This is useful for example when testing the above mentioned example that
 * creates a file. It is preferable for test drivers to clean up the files
 * they create and this class can be used to automate this.
 *
 * @see Temporary
 * @see SafeModify
 */
class Eraser
{
public:

    /**
     * Constructor.
     * @warning Be careful what paths you give as parameter. The target will
     *          get deleted unless protected by the OS. If target is a
     *          directory, it will be deleted with its entire content.
     * @param path Path to the file or directory to be erased unless released.
     */
    explicit Eraser(const boost::filesystem::path &path);

    /**
     * Destructor
     */
    ~Eraser( );

    /**
     * Signals that erasing is no longer required. Call this method when you
     * have successfully done everything you need to the target.
     */
    void Release( );

private:

    /// Disabled copy constructor
    Eraser(const Eraser &);
    /// Disabled assignment operator
    Eraser operator=(const Eraser &);

    boost::shared_ptr<boost::filesystem::path> path_;
};

}

}

#endif
