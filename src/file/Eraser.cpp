// Copyright 2007-2012 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "myrrh/file/Eraser.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

namespace myrrh
{

namespace file
{

Eraser::Eraser(const boost::filesystem::path &path) :
    path_(new boost::filesystem::path(path))
{
    assert(!boost::filesystem::equivalent(path,
                                          boost::filesystem::current_path( )));
}

Eraser::~Eraser( )
{
    if (path_->empty( ))
    {
        return;
    }
    try
    {
        boost::filesystem::remove_all(*path_);
    }
    catch (...)
    {
        // Do not allow exceptions from destructor
    }
}

void Eraser::Release( )
{
    *path_ = "";
}

}

}
