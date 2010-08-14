// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of myrrh::file::ReadOnly
 *
 * $Id: ReadOnly.hpp 361 2007-09-19 19:59:44Z byon $
 */

#ifndef MYRRH_FILE_READONLYFILE_HPP_INCLUDED
#define MYRRH_FILE_READONLYFILE_HPP_INCLUDED

#include "boost/filesystem/path.hpp"
#include <string>

namespace myrrh
{
namespace file
{

/**
 * This class has no production class value. It is usable only for unit test
 * purposes, when the test setup requires a read-only file.
 * When object of ReadOnly is created, the file is created and it is set to be
 * readonly. At destruction, the file is removed.
 */
class ReadOnly
{
public:

    ReadOnly(const boost::filesystem::path &path, const std::string &content);

    ~ReadOnly( );

    class SetFailed : public std::runtime_error
    {
    public:

        SetFailed(const boost::filesystem::path &path, int errorCode);
    };

    class AlreadyExists : public std::runtime_error
    {
    public:

        explicit AlreadyExists(const boost::filesystem::path &path);
    };

private:

    ReadOnly(const ReadOnly&);
    ReadOnly operator=(const ReadOnly&);

    boost::filesystem::path path_;
};

}
}

#endif
