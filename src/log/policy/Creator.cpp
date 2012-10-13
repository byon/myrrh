// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the implementation of class myrrh::log::Creator.
 *
 * $Id: Creator.cpp 340 2007-09-11 19:51:16Z byon $
 */

#include "myrrh/log/policy/Creator.hpp"
#include "myrrh/log/policy/Path.hpp"

#include "boost/filesystem/convenience.hpp"

#include <cassert>

namespace myrrh
{

namespace log
{

namespace policy
{

Creator::Creator( )
{
}

/// Divide smaller
boost::filesystem::path Creator::DoOpen(std::filebuf &file, Path& path)
{
    const boost::filesystem::path PATH(path.Generate( ));
    assert(!PATH.empty( ));

    try
    {
        if (PATH.has_parent_path( ))
        {
            // These days there is a no-throw version available
            boost::filesystem::create_directories(PATH.parent_path( ));
        }
        file.open(PATH.string( ).c_str( ), std::ios::out | std::ios::trunc);
    }
    catch (...)
    {
    }

    return PATH;
}

}

}

}
