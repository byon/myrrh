// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the declaration of class myrrh::log::Creator.
 *
 * $Id: Creator.hpp 340 2007-09-11 19:51:16Z byon $
 */

#ifndef MYRRH_LOG_POLICY_CREATOR_HPP_INCLUDED
#define MYRRH_LOG_POLICY_CREATOR_HPP_INCLUDED

/// @todo Isolate better
#include "myrrh/log/policy/Opener.hpp"
#include "myrrh/log/policy/Path.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

/**
 * Creator class is used in the myrrh::log::policy library to open a file
 * buffer so that any possible old content is destroyed.
 * Because Creator is a subclass of InitialOpener (in contrast to Opener), it
 * can be used in the context of myrrh::log::policy to do the initial opening
 * of the log file.
 */
class Creator : public InitialOpener
{
public:

    /**
     * Constructor
     */
    Creator( );

private:

    /**
     * Implements the file opening.
     * Provides no-throw guarantee.
     * @param file The file buffer to be opened
     * @param The path of the file opened
     */
    virtual boost::filesystem::path DoOpen(std::filebuf &file, Path& path);

    Creator(const Creator&);
    Creator& operator=(const Creator&);
};

}

}

}

#endif
