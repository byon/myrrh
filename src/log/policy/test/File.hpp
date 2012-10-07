// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * Contains declaration for test class File
 *
 * $Id: File.hpp 356 2007-09-18 19:55:21Z byon $
 */

#ifndef MYRRH_LOG_POLICY_TEST_EXAMPLES_FILE_HPP_INCLUDED
#define MYRRH_LOG_POLICY_TEST_EXAMPLES_FILE_HPP_INCLUDED

// isolate better
#include "boost/filesystem/path.hpp"
#include <string>
#include <vector>

class File
{
public:

    explicit File(const boost::filesystem::path &path);

    const boost::filesystem::path &Path( ) const;
    std::string Content( ) const;
    void AddLine(const std::string &line);
    std::size_t Size( ) const;
    void CropToSize(std::size_t size);

private:

    boost::filesystem::path path_;
    std::string content_;
};

typedef std::vector<File> Files;

#endif
