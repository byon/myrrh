// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the implementation of classes myrrh::log::policy::File
 * and myrrh::log::policy::Opener
 *
 * $Id: Opener.cpp 355 2007-09-17 18:48:35Z byon $
 */

#include "myrrh/log/policy/Opener.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

// File class implementations

File::File(Opener &opener, policy::Path& path) :
    writtenSize_(0),
    PATH_(TryOpening(opener, path, file_))
{
    std::streamsize end = file_.tellp( );
    if (end > 0)
    {
        writtenSize_ = end;
    }
}

std::streamsize File::Write(const std::string &line)
{
    // It can be a programming error, if the file buffer is not open. But it
    // is possible that the file just could not be opened. Because we are
    // following a no-throw policy, we must prepare for the possibility.
    if (!file_.is_open( ))
    {
        return -1;
    }

    const std::streampos ORIG_POS(file_.tellp( ));
    file_ << line;
    file_.flush( );

    const std::streampos DIFFERENCE(file_.tellp( ) - ORIG_POS);
    writtenSize_ += DIFFERENCE;

    return DIFFERENCE;
}

boost::filesystem::path File::TryOpening(Opener &opener, policy::Path &path,
                                         std::ofstream &file)
{
    // The opening is done in this separate function, because it is possible
    // to fail for lack of memory. Any other exceptions are programming errors
    // and thus we'll assert if those are caught.
    try
    {
        return opener.DoOpen(*file.rdbuf( ), path);
    }
    catch (const std::bad_alloc&)
    {
        // No memory, nothing that can be done
    }
    catch (...)
    {
        assert(false && "Exception thrown from Opener::DoOpen implementation");
    }

    return "";
}

bool operator==(const File &left, const File &right)
{
    return (left.PATH_ == right.PATH_);
}

// Opener class implementation

Opener::~Opener( )
{
}

}

}

}
