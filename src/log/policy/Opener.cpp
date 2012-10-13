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
#include "myrrh/log/policy/Path.hpp"
#include "myrrh/log/policy/File.hpp"

namespace myrrh
{
namespace log
{
namespace policy
{

Opener::~Opener( )
{
}

FilePtr Opener::Open(Path path)
{
    return FilePtr(new (std::nothrow) File(*this, path));
}

}
}
}
