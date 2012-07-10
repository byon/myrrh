// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class myrrh::log::policy::Path.
 *
 * $Id: Path.cpp 355 2007-09-17 18:48:35Z byon $
 */

#include "myrrh/log/policy/Path.hpp"
#include "myrrh/log/policy/PathPart.hpp"
#include "myrrh/log/policy/RestrictionStore.hpp"
#include "myrrh/file/MatchFiles.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

// Local declarations

namespace
{

/**
 * Used to determine the result of path part comparison
 */
enum Comparison
{
    LESS = 0,
    EVEN,
    MORE
};

/**
 * Checks if the given path part object is of type Folder
 */
bool IsFolder(const PathPartPtr &part);

/**
 * Checks if the given part store contains folders. If so, iterator to the
 * first folder is stored to result parameter.
 * @param store The store to look through
 * @param result The placeholder for the result. If function returns false, the
 *               iterator must not be dereferenced.
 * @returns true, if there is at least one Folder object in the given store
 */
bool FindFirstFolder(PartStore &store, PartStore::iterator &result);

/**
 * Extracts the path parts from the beginning of given store to the part
 * pointed to by given folder parameter.
 * @param store The store to check
 * @param folder Iterator that points to the first folder in store
 * @param foldersFound true, if this is not the first call and previous calls
 *                     have found folders
 * @throws Path::Error If the resulting path part store is empty. The error
 *                     string depends on the value of parameter foldersFound.
 */
PartStore PartsUntilFolder(const PartStore &store,
                           PartStore::const_iterator folder,
                           bool foldersFound);

/**
 * Helper function for combining regular expressions.
 * @param left Object on the left hand side of addition
 * @param right Object on the right hand side of addition
 * @return The resulting boost::regexp object that combines given objects.
 */
boost::regex operator+(const boost::regex &left, const boost::regex &right);

/**
 * @todo Refactor. The code is a bit too complex to understand
 */
std::string FirstMatch(const boost::regex &expression,
                       const std::string &toMatch);

/**
 * Compares the two given strings according to the rules defined by the given
 * path part. If the start of the strings are equivalent, the strings are
 * updated to the point after the noted equivalence.
 * @param part The part according to which the comparison is made
 * @param left The left hand side of the comparison
 * @param right The right hadn side of the comparison.
 * @return MORE, if the left side is earlier, LESS, if right side is earlier
 *         or EVEN, if the part so far compared is equivalent.
 */
Comparison CompareAndUpdate(const PathPart &part, std::string &left,
                            std::string &right);

}

// Path class implementations

Path::Path(const boost::filesystem::path &parentPath) :
    PARENT_PATH_(parentPath)
{
}

const boost::filesystem::path &Path::ParentPath( ) const
{
    return PARENT_PATH_;
}

boost::filesystem::path Path::Generate( )
{
    return PARENT_PATH_ / CombineEntities( );
}

Path &Path::operator+=(const PartSum &parts)
{
    EntityStore storeCopy(AddNewParts(entityStore_, parts));
    entityStore_.swap(storeCopy);

    return *this;
}

Path &Path::operator+=(const std::string &path)
{
    PartSum sum;
    sum += path;
    *this += sum;

    return *this;
}

Path::EntityIterator Path::BeginEntity( ) const
{
    return entityStore_.begin( );
}

Path::EntityIterator Path::EndEntity( ) const
{
    return entityStore_.end( );
}

boost::filesystem::path Path::CombineEntities( )
{
    boost::filesystem::path result;

    typedef EntityStore::iterator EntityIter;
    for (EntityIter i = entityStore_.begin( ); entityStore_.end( ) != i; ++i)
    {
        result /= boost::filesystem::path(i->Generate( ));
    }

    return result;
}

void Path::AddNewEntity(Path::EntityStore &store, const PartStore &parts)
{
    Entity newEntity;
    newEntity.Add(parts);
    store.push_back(newEntity);
}

Path::EntityStore Path::AddNewParts(const Path::EntityStore &store,
                                    const PartSum &parts)
{
    PartStore partStore(parts.Parts( ));
    assert(!partStore.empty( ));

    EntityStore storeCopy(store);

    PartStore::iterator folder;

    while (FindFirstFolder(partStore, folder))
    {
        PartStore newParts(PartsUntilFolder(partStore, folder,
                                            !storeCopy.empty( )));

        AddNewEntity(storeCopy, newParts);

        partStore.erase(partStore.begin( ), folder + 1);
    }

    if (!partStore.empty( ))
    {
        AddNewEntity(storeCopy, partStore);
    }

    return storeCopy;
}

void Path::AppendRestrictions(RestrictionStore &store) const
{
    for (EntityIterator i = entityStore_.begin( );
         entityStore_.end( ) != i;
         ++i)
    {
        i->AppendRestrictions(store);
    }
}

// Path::Entity class implementations

Path::Entity::Entity( )
{
}

void Path::Entity::Add(const PartStore &parts)
{
    assert(!parts.empty( ));
    partStore_.insert(partStore_.end( ), parts.begin( ), parts.end( ));
}

std::string Path::Entity::Generate( )
{
    std::string result;

    typedef PartStore::const_iterator PartIter;
    for (PartIter i = partStore_.begin( ); partStore_.end( ) != i; ++i)
    {
        result += (*i)->Generate( );
    }

    return result;
}

file::ExpressionMatcher Path::Entity::Matcher( ) const
{
    boost::regex result;

    typedef PartStore::const_iterator PartIter;
    for (PartIter i = partStore_.begin( ); partStore_.end( ) != i; ++i)
    {
        result = result + (*i)->GetExpression( );
    }

    return file::ExpressionMatcher(result);
}

Path::Entity::Comparer Path::Entity::GetComparer( ) const
{
    return Path::Entity::Comparer(*this);
}

bool Path::Entity::IsFirstEarlier(const boost::filesystem::path &left,
                                  const boost::filesystem::path &right) const
{
    std::string leftString(left.string( ));
    std::string rightString(right.string( ));

    assert(Matcher( )(leftString));
    assert(Matcher( )(rightString));

    typedef PartStore::const_iterator PartIter;
    for (PartIter i = partStore_.begin( ); partStore_.end( ) != i; ++i)
    {
        assert(!leftString.empty( ));
        assert(!rightString.empty( ));
        switch (CompareAndUpdate(**i, leftString, rightString))
        {
        case LESS:
            return true;
        case EVEN:
            break;
        case MORE:
            return false;
        default:
            assert(false && "Unidentified Comparison enum value");
        }
    }

    return true;
}

void Path::Entity::AppendRestrictions(RestrictionStore &store) const
{
    for (PartStore::const_iterator i = partStore_.begin( );
         partStore_.end( ) != i;
         ++i)
    {
        (*i)->AppendRestrictions(store);
    }
}

// Path::Entity::Comparer class implementations

Path::Entity::Comparer::Comparer(const Path::Entity &entity) :
    entity_(&entity)
{
}

bool Path::Entity::Comparer::
operator( )(const boost::filesystem::path &left,
            const boost::filesystem::path &right) const
{
    return entity_->IsFirstEarlier(left, right);
}

// Path::Error class implementations

Path::Error::Error(const std::string &what) :
    std::runtime_error(what)
{
}

// Local implementations

namespace
{

inline bool IsFolder(const PathPartPtr &part)
{
    return typeid(*part) == typeid(Folder);
}

bool FindFirstFolder(PartStore &store, PartStore::iterator &result)
{
    result = std::find_if(store.begin( ), store.end( ), IsFolder);
    return store.end( ) != result;
}

PartStore PartsUntilFolder(const PartStore &store,
                           PartStore::const_iterator folder,
                           bool foldersFound)
{
    PartStore newParts(store.begin( ), folder);
    if (newParts.empty( ))
    {
        if (!foldersFound)
        {
            throw Path::Error("Folder not allowed as first path part");
        }

        throw Path::Error("Two unseparated Folders not allowed in path");
    }

    return newParts;
}

inline boost::regex operator+(const boost::regex &left,
                              const boost::regex &right)
{
    return boost::regex(left.str( ) + right.str( ));
}

std::string FirstMatch(const boost::regex &expression,
                       const std::string &toMatch)
{
    boost::sregex_iterator matches(toMatch.begin( ), toMatch.end( ),
                                   expression);
    assert(matches != boost::sregex_iterator( ));
    assert(matches->begin( ) != matches->end( ));

    return *matches->begin( );
}

Comparison CompareAndUpdate(const PathPart &part, std::string &left,
                            std::string &right)
{
    const boost::regex EXPRESSION(part.GetExpression( ));

    const std::string LEFT_MATCH(FirstMatch(EXPRESSION, left));
    const std::string RIGHT_MATCH(FirstMatch(EXPRESSION, right));

    if (part.IsFirstEarlier(LEFT_MATCH, RIGHT_MATCH))
    {
        return LESS;
    }

    if (part.IsFirstEarlier(RIGHT_MATCH, LEFT_MATCH))
    {
        return MORE;
    }

    left = left.substr(LEFT_MATCH.size( ));
    right = right.substr(RIGHT_MATCH.size( ));

    return EVEN;
}

}

}
}
}
