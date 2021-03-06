// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of myrrh::file::ReadOnly
 *
 * $Id: ReadOnly.cpp 361 2007-09-19 19:59:44Z byon $
 */

#include "myrrh/file/ReadOnly.hpp"
#include "boost/filesystem/operations.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/lexical_cast.hpp"
#pragma warning(pop)

#include <fstream>

namespace myrrh
{
namespace file
{

// Local declarations

namespace
{

void SetReadOnly(const boost::filesystem::path &path);
void RemoveReadOnly(const boost::filesystem::path &path);
std::string ErrorString(const boost::filesystem::path &path, int errorCode);

}

// Public implementations

ReadOnly::ReadOnly(const boost::filesystem::path &path,
                   const std::string &content) :
    path_(new boost::filesystem::path(path))
{
    if (!boost::filesystem::exists(*path_))
    {
        std::ofstream file(path_->string( ).c_str( ));
        file << content;
    }

    SetReadOnly(*path_);
}

ReadOnly::~ReadOnly( )
{
    RemoveReadOnly(*path_);
    try
    {
        boost::filesystem::remove(*path_);
    }
    catch(...)
    {
        // Ignoring exceptions in destructor
    }
}

ReadOnly::SetFailed::SetFailed(const boost::filesystem::path &path,
                               int errorCode) :
    std::runtime_error(ErrorString(path, errorCode))
{
}

ReadOnly::AlreadyExists::AlreadyExists(const boost::filesystem::path &path) :
    std::runtime_error(path.string( ) + "already exists")
{
}

// Local implementations

namespace
{

void SetReadOnly(const boost::filesystem::path &path)
{
    using namespace boost::filesystem;
    permissions(path, remove_perms | owner_write);
}

void RemoveReadOnly(const boost::filesystem::path &path)
{
    using namespace boost::filesystem;
    assert(exists(path));
    permissions(path, add_perms | owner_write);
}

std::string ErrorString(const boost::filesystem::path &path, int errorCode)
{
    return "Failed to set file '" + path.string( ) + "' as read-only, error " +
           boost::lexical_cast<std::string>(errorCode);
}

}

}
}
