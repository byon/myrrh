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

enum Comparison
{
    LESS = 0,
    EVEN,
    MORE
};

bool IsFolder(const PathPartPtr &part);
bool FindFirstFolder(PartStore &store, PartStore::iterator &result);
PartStore PartsUntilFolder(const PartStore &store,
                           PartStore::const_iterator folder,
                           bool foldersFound);
boost::regex operator+(const boost::regex &left, const boost::regex &right);

/**
 * @todo Refactor. The code is a bit too complex to understand
 */
std::string FirstMatch(const boost::regex &expression,
                       const std::string &toMatch);
Comparison CompareAndUpdate(const PathPart &part, std::string &left,
                            std::string &right);
}

class Path::Implementation
{
public:
    explicit Implementation(const boost::filesystem::path &parentPath);
    const boost::filesystem::path &ParentPath( ) const;
    boost::filesystem::path Generate( );
    void Add(const PartSum &parts);
    void Add(const std::string &path);
    EntityIterator BeginEntity( ) const;
    EntityIterator EndEntity( ) const;
    void AppendRestrictions(RestrictionStore &store) const;
private:
    boost::filesystem::path CombineEntities( );
    static EntityStore AddNewParts(const EntityStore &store,
                                   const PartSum &parts);
    static void AddNewEntity(EntityStore &store, const PartStore &parts);

    Implementation(const Implementation &);
    Implementation &operator=(const Implementation &);

    const boost::filesystem::path PARENT_PATH_;
    EntityStore entityStore_;
};

// Path class implementations

Path::Path( ) :
    implementation_(new Implementation(boost::filesystem::path( )))
{
}

Path::Path(const boost::filesystem::path &parentPath) :
    implementation_(new Implementation(parentPath))
{
}

const boost::filesystem::path &Path::ParentPath( ) const
{
    return implementation_->ParentPath( );
}

boost::filesystem::path Path::Generate( )
{
    return implementation_->Generate( );
}

Path &Path::operator+=(const PartSum &parts)
{
    implementation_->Add(parts);
    return *this;
}

Path &Path::operator+=(const std::string &path)
{
    implementation_->Add(path);
    return *this;
}

Path::EntityIterator Path::BeginEntity( ) const
{
    return implementation_->BeginEntity( );
}

Path::EntityIterator Path::EndEntity( ) const
{
    return implementation_->EndEntity( );
}

void Path::AppendRestrictions(RestrictionStore &store) const
{
    implementation_->AppendRestrictions(store);
}

Path::Implementation::
Implementation(const boost::filesystem::path &parentPath) :
    PARENT_PATH_(parentPath)
{
}

const boost::filesystem::path &Path::Implementation::ParentPath( ) const
{
    return PARENT_PATH_;
}

boost::filesystem::path Path::Implementation::Generate( )
{
    return PARENT_PATH_ / CombineEntities( );
}

void Path::Implementation::Add(const PartSum &parts)
{
    entityStore_.swap(AddNewParts(entityStore_, parts));
}

void Path::Implementation::Add(const std::string &path)
{
    PartSum sum;
    sum += path;
    Add(sum);
}

Path::EntityIterator Path::Implementation::BeginEntity( ) const
{
    return entityStore_.begin( );
}

Path::EntityIterator Path::Implementation::EndEntity( ) const
{
    return entityStore_.end( );
}

boost::filesystem::path Path::Implementation::CombineEntities( )
{
    boost::filesystem::path result;

    typedef EntityStore::iterator EntityIter;
    for (EntityIter i = entityStore_.begin( ); entityStore_.end( ) != i; ++i)
    {
        result /= boost::filesystem::path(i->Generate( ));
    }

    return result;
}

void Path::Implementation::AddNewEntity(Path::EntityStore &store,
                                        const PartStore &parts)
{
    Entity newEntity;
    newEntity.Add(parts);
    store.push_back(newEntity);
}

// Divide smaller
Path::EntityStore Path::Implementation::
AddNewParts(const Path::EntityStore &store, const PartSum &parts)
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

void Path::Implementation::AppendRestrictions(RestrictionStore &store) const
{
    for (auto i = entityStore_.begin( ); entityStore_.end( ) != i; ++i)
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

// Divide smaller
bool Path::Entity::IsFirstEarlier(const boost::filesystem::path &left,
                                  const boost::filesystem::path &right) const
{
    std::string leftString(left.string( ));
    std::string rightString(right.string( ));

    assert(Matcher( )(leftString));
    assert(Matcher( )(rightString));

    // Why is there a loop, if always comparing only the first?
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
    for (auto i = partStore_.begin( ); partStore_.end( ) != i; ++i)
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

// Use early return
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

// Divide smaller
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
