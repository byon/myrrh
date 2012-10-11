// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class myrrh::log::policy::Resizer
 *
 * $Id: Resizer.hpp 357 2007-09-18 20:09:36Z byon $
 */

#ifndef MYRRH_LOG_POLICY_RESIZER_HPP_INCLUDED
#define MYRRH_LOG_POLICY_RESIZER_HPP_INCLUDED

#include "myrrh/log/policy/Opener.hpp"

namespace boost { namespace filesystem { class path; } }

namespace myrrh
{

namespace log
{

namespace policy
{

class Path;

/**
 * This class a way of opening log files for myrrh::log::policy component. The
 * opening for Resizer means that the given file is resized to be of a specific
 * size or smaller, if the last line does not fit in in entirety.
 */
class Resizer : public Opener
{
public:

    /**
     * Constructor.
     * @param sizeLeftAfter This is the maximum size that should be left after
     *                      resize operation. If the last line does not fit
     *                      into the resized file in it's entirety, it is
     *                      cropped out as well.
     */
    explicit Resizer(std::streamsize sizeLeftAfter);

private:

    /**
     * Implements the opening operation
     * @param file The that should be reopened
     * @returns The path to the new file
     */
    virtual boost::filesystem::path DoOpen(std::filebuf &file, Path& path);

    /// Copy construction prevented
    Resizer(const Resizer &);
    /// Assignment prevented
    Resizer &operator=(const Resizer &);

    /// The maximum size that should be left after resize operation
    const std::streamsize SIZE_LEFT_AFTER_;
};

}

}

}

#endif
