// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class myrrh::log::policy::Path
 *
 * $Id: Path.hpp 355 2007-09-17 18:48:35Z byon $
 */

#ifndef MYRRH_LOG_POLICY_PATH_HPP_INCLUDED
#define MYRRH_LOG_POLICY_PATH_HPP_INCLUDED

/// @todo Isolate better
#include "boost/shared_ptr.hpp"
#include "boost/filesystem/path.hpp"

#include <iterator>
#include <string>
#include <vector>

namespace myrrh
{

// Forward declarations
namespace file { class ExpressionMatcher; }

namespace log
{

namespace policy
{

// Forward declarations
class PartSum;
class PathPart;
class RestrictionStore;

typedef std::vector<boost::shared_ptr<PathPart> > PartStore;

/**
 * The Path class is used to contain the rules needed to construct names for
 * new files and to match old file names. A Path object is constructed from
 * one or several PathPart objects. Each of the PathPart objects are
 * responsible for one small part in the file path. They can be hard-coded
 * strings, running integers, timestamps, etc.
 *
 * Examples on how to construct new Path objects can be found from
 * src/log/policy/Examples.cpp.
 *
 * New file names can be generated by calling member method Generate( ).
 *
 * For purposes of comparing existing file names to the rules of a Path object,
 * @see Path::Entity.
 *
 * @todo Add tests for copy construction
 */
class Path
{
public:

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
    /// @todo Does this need to be a public class?
    class Entity : public std::iterator<std::forward_iterator_tag, Entity>
    {
    public:

        /**
         * Returns an object that can be checked if a path matches to the rules
         * specified in the entity. @see myrrh::file::ExpressionMatcher.
         */
        file::ExpressionMatcher Matcher( ) const;

        /**
         * A class used to compare two boost::filesystem::path objects into
         * order of preference according to the entity rules.
         *
         * By making std::binary_function a parent class, it is possible to
         * use Comparer as a binary predicate in STL algorithms, sort( ) most
         * specifically.
         */
        /// @todo Check the benefit of binary_function. Could probably be
        ///       implemented with lambda or std::bind.
        class Comparer : public std::binary_function<boost::filesystem::path,
                                                     boost::filesystem::path,
                                                     bool>
        {
        public:

            /**
             * Does the comparison.
             * @param left The object on the left side of comparison
             * @param right The object on the right side of comparison
             * @returns true if the left object is earlier, false otherwise
             */
            bool operator( )(const boost::filesystem::path &left,
                             const boost::filesystem::path &right) const;

        private:

            // Friend access is needed, so Path::Entity can construct
            // Path::Entity::Comparer objects
            friend class Entity;

            /**
             * Constructor
             * Provides no-throw guarantee
             * @param entity The entity that has the rules for comparison
             */
            explicit Comparer(const Path::Entity &entity);

            // Gives access to the entity object
            const Path::Entity *entity_;
        };

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
        // Friend access is needed, so Comparer can access the methods that
        // truly implement the comparison.
        friend class Comparer;

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

    typedef std::vector<Entity> EntityStore;
    typedef EntityStore::const_iterator EntityIterator;

    /**
     * Exception class, which is thrown when errors occur during Path creation
     */
    class Error : public std::runtime_error
    {
    public:
        explicit Error(const std::string &what);
    };

    /**
     * Constructor.
     * @param parentPath Parent path that will be used as a starting place for
     *                   all paths generated by the Path object. If empty path
     *                   is given (as is the default), the paths will be
     *                   constructed from the running directory.
     */
    explicit Path(const boost::filesystem::path &parentPath =
                      boost::filesystem::path( ));

    /**
     * Returns the parent path. May be empty.
     */
    const boost::filesystem::path &ParentPath( ) const;

    /**
     * Generates a new file path from the contained path parts
     * @return A new file path that can be used to open a brand new file. It is
     *         not guaranteened that the path is not already in use.
     */
    boost::filesystem::path Generate( );

    /**
     * Adds new path parts to the path. Note that the method is not planned to
     * be used straight by the user. Instead the user is expected to add
     * objects of PathPart subclasses, which are then implicitly collected
     * to a new PartSum object.
     * @param parts A storage of the new path parts
     * @returns reference to *this
     */
    Path &operator+=(const PartSum &parts);

    /**
     * Adds a new hard coded path part to the path. If the new part contains
     * path separators, the part is separated into distict path entities.
     * @param path The new path part to be added.
     * @returns reference to *this
     */
    Path &operator+=(const std::string &path);

    /**
     * Returns an iterator that points to the first entity in the path.
     */
    // What is this really used for?
    EntityIterator BeginEntity( ) const;

    /**
     * Returns an iterator that marks the end of the entities of the path. Note
     * that it is always illegal to dereference this iterator. It must only be
     * used for checking of end of iteration.
     */
    // What is this really used for?
    EntityIterator EndEntity( ) const;

    /**
     * The entities of the path may contain restrictions for the path. Those
     * restrictions can be added to a restriction store via this method. An
     * example is myrrh::log::policy::Date, which has restriction of type
     * myrrh::log::policy::DateRestriction.
     * @param store The store into which the new restriction will be appended
     */
    void AppendRestrictions(RestrictionStore &store) const;

private:

    /**
     * Combines the path strings generated by the stored entities into a new
     * path and returns it.
     */
    boost::filesystem::path CombineEntities( );

    /**
     * Creates a new entity store, adds the given path parts and returns the
     * result.
     * Provides strong exception safety.
     * @param store The original entity store
     * @param parts The new path parts to be added
     * @returns The resulting new entity store
     */
    static EntityStore AddNewParts(const EntityStore &store,
                                   const PartSum &parts);

    /**
     * Adds new path parts to the given entity store as a new entity.
     * Provides strong exception safety.
     * @param store The store into which the new entity will be added to
     * @param parts The new path parts to be added
     */
    static void AddNewEntity(EntityStore &store, const PartStore &parts);

    /// Disabled assignment operator
    Path &operator=(const Path &);

    /// The parent path
    const boost::filesystem::path PARENT_PATH_;
    /// Storest entities of the path
    EntityStore entityStore_;
};

}
}
}

#endif
