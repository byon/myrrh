// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class
 * myrrh::log::policy::RestrictionStore
 *
 * $Id: RestrictionStore.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/log/policy/RestrictionStore.hpp"
#include "myrrh/log/policy/Restriction.hpp"

#include <cassert>

namespace myrrh
{

namespace log
{

namespace policy
{

// Class implementations

void RestrictionStore::Add(RestrictionPtr restriction)
{
    assert(restriction);
    restrictions_.push_back(restriction);
}

bool RestrictionStore::IsRestricted(const File &file,
                                    std::size_t toWrite) const
{
    for (auto i = restrictions_.begin( ); restrictions_.end( ) != i; ++i)
    {
        if ((*i)->IsRestricted(file, toWrite))
        {
            return true;
        }
    }

    return false;
}

std::size_t RestrictionStore::Count( ) const
{
    return restrictions_.size( );
}

}

}

}
