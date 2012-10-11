// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MYRRH_LOG_POLICY_PATHENTITY_HPP_INCLUDED
#define MYRRH_LOG_POLICY_PATHENTITY_HPP_INCLUDED

#include "myrrh/log/policy/Path.hpp"

#include "boost/shared_ptr.hpp"

#include <iterator>
#include <functional>
#include <string>
#include <vector>

namespace boost { namespace filesystem { class path; } }

namespace myrrh
{

// Forward declarations
namespace file { class ExpressionMatcher; }

namespace log
{

namespace policy
{

// Forward declarations
class PathPart;
class RestrictionStore;

typedef std::vector<boost::shared_ptr<PathPart> > PartStore;

/**
 * A Path::Entity object combines the path parts that make up a rule for
 * one directory or file name in a path. For instance in a path
 * 'folder01012007/file1234.log' the first entity could consist of PathPart
 * objects of type Text and Date. The second entity could consist of
 * PathPart objects of type Text, Index and Text.
 *
 * The entities of a Path object can be iterated and for each entity
 * object one can access a Path::Entity::Comparer object and a
 * file::ExpressionMatcher object. With the first it is possible to sort
 * boost::filesystem::path objects in the time order as specified by the
 * rules of the entity. With the second it is possible to check, if a
 * an existing file path matches to the entity. Combining these two objects
 * it is possible to find the file that is the latest (according to the
 * entity rules, the naming of the files determines the result) in a
 * in a directory. This is used to determine the file for appending new
 * content at program start-up.
 *
 * By making std::iterator a parent class it is possible to use Entity
 * as iterator in STL algorithms.
 */
class Path::Entity : public std::iterator<std::forward_iterator_tag, Entity>
{
public:

    typedef std::function<bool (const boost::filesystem::path&,
                                const boost::filesystem::path)> Comparer;

    /**
     * Returns an object that can be checked if a path matches to the rules
     * specified in the entity. @see myrrh::file::ExpressionMatcher.
     */
     file::ExpressionMatcher Matcher( ) const;

    /**
     * Returns an object that can be used to sort two
     * boost::filesystem::path objects into order of preference according
     * to the entity rules. The object can be passed to STL algorithms,
     * like std::sort( ).
     */

    Comparer GetComparer( ) const;

    void AppendRestrictions(RestrictionStore &store) const;

private:

    // Friend access is needed, so Path can construct Entity objects,
    // add new path parts and to Generate new path strings
    friend class Path;

    /**
     * Default constructor
     */
    Entity( );

    /**
     * Adds PathPart objects to Path::Entity
     * @param parts A storage containing the new part objects. It is
     *              considered a programming error to pass an empty
     *              storage.
     */
    void Add(const PartStore &parts);

    /**
     * Generates a string that represents the entity in a new file path
     */
    std::string Generate( );

    /**
     * Compares two boost::filesystem::path objects into
     * order of preference according to the entity rules.
     * @param left The object on the left side of comparison
     * @param right The object on the right side of comparison
     * @returns true if the left object is earlier, false otherwise
     */
    bool IsFirstEarlier(const boost::filesystem::path &left,
                        const boost::filesystem::path &right) const;

    PartStore partStore_;
};

}
}
}

#endif
