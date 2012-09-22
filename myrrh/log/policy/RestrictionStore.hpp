// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class
 * myrrh::log::policy::RestrictionStore
 *
 * $Id: RestrictionStore.hpp 286 2007-03-18 15:04:53Z Byon $
 */

#ifndef MYRRH_LOG_POLICY_RESTRICTIONSTORE_HPP_INCLUDED
#define MYRRH_LOG_POLICY_RESTRICTIONSTORE_HPP_INCLUDED

#include <vector>

// Forward declarations
namespace boost { template <typename T> class shared_ptr; }

namespace myrrh
{

namespace log
{

namespace policy
{

// Forward declarations
class File;
class Restriction;
typedef boost::shared_ptr<Restriction> RestrictionPtr;

/**
 * This class is used to store Restriction objects and to check if any of the
 * contained restrictions apply to the current conditions.
 */
class RestrictionStore
{
    typedef std::vector<RestrictionPtr> Restrictions;
public:

    /**
     * Adds a new restriction in to the store.
     * @param restriction The new restriction to be added
     */
    void Add(RestrictionPtr restriction);

    /**
     * Checks the contained restrictions, if any of them apply.
     * @param file The file that is checked for restrictions
     * @param toWrite The size of text written next to log
     * @returns true, if at least one restriction apply.
     */
    bool IsRestricted(const File &file, std::size_t toWrite) const;

    /**
     * Returns the count of stored restrictions
     * @note Useful only for testing
     */
    std::size_t Count( ) const;

private:

    /**
     * Stores the restrictions
     */
    Restrictions restrictions_;
};

}

}

}

#endif
