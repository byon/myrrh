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

namespace
{

const std::string FILE_TEMPORARY_PREFIX(".tmp");

}

class SafeModify::Implementation
{
public:
    explicit Implementation(const boost::filesystem::path &original);
    ~Implementation( );
    static std::string Name(const boost::filesystem::path &original);
    void Commit( ) const;

private:
    const boost::filesystem::path ORIGINAL_;
    const boost::filesystem::path TEMPORARY_;
};

SafeModify::SafeModify(const boost::filesystem::path &original) :
    implementation_(new Implementation(original))
{
}

void SafeModify::Commit( ) const
{
    implementation_->Commit( );
}

std::string SafeModify::Name(const boost::filesystem::path &original)
{
    return Implementation::Name(original);
}

SafeModify::Implementation::
Implementation(const boost::filesystem::path &original) :
    ORIGINAL_(original),
    TEMPORARY_(Name(original))
{
    boost::filesystem::remove(TEMPORARY_);
    boost::filesystem::rename(ORIGINAL_, TEMPORARY_);
}

SafeModify::Implementation::~Implementation( )
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

void SafeModify::Implementation::Commit( ) const
{
    // The interface documentation promises that this method cannot throw.
    // The following call CAN throw, but only in situations that are not
    // possible if SafeModify works correctly.
    boost::filesystem::remove(TEMPORARY_);
}

std::string SafeModify::Implementation::
Name(const boost::filesystem::path &original)
{
    return original.string( ) + FILE_TEMPORARY_PREFIX;
}

}

}
