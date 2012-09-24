// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * Contains implementation of test class File
 *
 * $Id: File.cpp 354 2007-09-17 17:39:58Z byon $
 */

#include "File.hpp"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>


File::File(const boost::filesystem::path &path) :
    path_(path)
{
}

const boost::filesystem::path &File::Path( ) const
{
    return path_;
}

std::string File::Content( ) const
{
    std::string result(content_);

    result.erase(std::remove(result.begin( ), result.end( ), '\r'),
                 result.end( ));

    return result;
}

std::size_t File::Size( ) const
{
    return content_.size( );
}

void File::AddLine(const std::string &line)
{
    content_ += line + '\r';
}

// Divide smaller
void File::CropToSize(std::size_t size)
{
    if (size >= content_.size( ))
    {
        return;
    }

    std::size_t removalPoint = content_.size( ) - size;

    const std::size_t NEXT_END_OF_LINE = content_.find('\n', removalPoint);

    if (std::string::npos != NEXT_END_OF_LINE)
    {
        assert(content_[NEXT_END_OF_LINE + 1] == '\r');
        removalPoint = NEXT_END_OF_LINE + 2;
    }

    content_.erase(0, removalPoint);
}
