// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains non inline implementation of myrrh::file::Temporary
 *
 * $Id: Temporary.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/file/Temporary.hpp"
#include "myrrh/file/Eraser.hpp"
#include "boost/filesystem/operations.hpp"

namespace myrrh
{

namespace file
{

class Temporary::Implementation
{
public:

    explicit Implementation(const boost::filesystem::path &path);
    std::ostream &Stream( );
    boost::filesystem::path Path( ) const;

private:

    boost::filesystem::path PATH_;
    Eraser eraser_;
    std::ofstream stream_;
};

Temporary::Temporary(const boost::filesystem::path &path) :
    implementation_(new Implementation(path))
{
}

std::ostream &Temporary::Stream( )
{
    return implementation_->Stream( );
}

boost::filesystem::path Temporary::Path( ) const
{
    return implementation_->Path( );
}

Temporary::PathError::PathError(const std::string what,
                                       const boost::filesystem::path &path) :
    std::runtime_error(what + " '" + path.string( ) + '\'')
{
}

// Refactor the method smaller
Temporary::Implementation::Implementation(const boost::filesystem::path &path) :
    PATH_(path),
    eraser_(path),
    stream_(path.string( ).c_str( ))
{
    if (PATH_.empty( ))
    {
        throw PathError("Invalid path", path);
    }

    if (boost::filesystem::exists(PATH_) &&
        boost::filesystem::is_directory(PATH_))
    {
        throw PathError("Directory path not accepted", path);
    }

    if (!stream_.is_open( ))
    {
        throw PathError("Cannot open file", PATH_);
    }
}

std::ostream &Temporary::Implementation::Stream( )
{
    return stream_;
}

boost::filesystem::path Temporary::Implementation::Path( ) const
{
    return PATH_;
}

}

}
