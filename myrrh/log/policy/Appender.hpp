// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class myrrh::log::Appender
 *
 * $Id: Appender.hpp 340 2007-09-11 19:51:16Z byon $
 */

#ifndef MYRRH_LOG_POLICY_APPENDER_HPP_INCLUDED
#define MYRRH_LOG_POLICY_APPENDER_HPP_INCLUDED

#include "myrrh/log/policy/Opener.hpp"

namespace boost { namespace filesystem { class path; } }

namespace myrrh
{

namespace log
{

namespace policy
{

// Forward declarations
class Path;

/**
 * Appender class is used in the myrrh::log::policy library to open a file
 * buffer so that new entries are appended to it.
 * Because Appender is a subclass of InitialOpener (in contrast to Opener), it
 * can be used in the context of myrrh::log::policy to do the initial opening
 * of the log file.
 */
class Appender : public InitialOpener
{
public:

    /**
     * Constructor
     */
    Appender( );

private:

    /**
     * Implements the file opening. Provides no-throw guarantee.
     * @param file The file buffer to be opened
     * @param The path of the file opened
     */
    virtual boost::filesystem::path DoOpen(std::filebuf &file, Path& path);

    /// Disabled copy constructor
    Appender(const Appender &);
    /// Disabled assignment operator
    Appender &operator=(const Appender &);
};

}

}

}

#endif
