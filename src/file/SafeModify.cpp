// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the implementation of class myrrh::file::SafeModify
 *
 * $Id: SafeModify.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/file/SafeModify.hpp"
#include "boost/filesystem/operations.hpp"

namespace myrrh
{

namespace file
{

SafeModify::SafeModify(const boost::filesystem::path &original) :
    ORIGINAL_(original),
    TEMPORARY_(Name(original))
{
    boost::filesystem::remove(TEMPORARY_);
    boost::filesystem::rename(ORIGINAL_, TEMPORARY_);
}

SafeModify::~SafeModify( )
{
    try
    {
        if (boost::filesystem::exists(TEMPORARY_))
        {
            // The changes were not finished, either because an exception
            // occurred or because there was a programming error (client forgot
            // to call Commit( ).
            // If some modifications were made to the file, we must first
            // remove the file, before renaming the original back to its place.
            // Otherwise rename will fail.
            boost::filesystem::remove(ORIGINAL_);
            boost::filesystem::rename(TEMPORARY_, ORIGINAL_);
        }
    }
    catch(...)
    {
        // exceptions are not allowed from destructor -> ignore silently
        // boost::filesystem::exists cannot throw, nor can remove if the
        // class works correctly, but rename can, if:
        // TEMPORARY_ does not exists -> checked before call
        // ORIGINAL_ contains empty path -> impossible, checked in constructor
        // ORIGINAL_ exists -> not possible, removed before call
        // ORIGINAL_ / ".." does not exist -> What does this mean?
    }
}

const std::string SafeModify::FILE_TEMPORARY_PREFIX_(".tmp");

}

}
