// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "myrrh/log/policy/Path.hpp"
#include "myrrh/log/policy/PathEntity.hpp"
#include "myrrh/log/policy/PathPart.hpp"

#include "boost/filesystem/path.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

// Local declarations

namespace
{

bool IsFolder(const PathPartPtr &part);
bool FindFirstFolder(PartStore &store, PartStore::iterator &result);
PartStore PartsUntilFolder(const PartStore &store,
                           PartStore::const_iterator folder,
                           bool foldersFound);
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

// Path::Error class implementations

Path::Error::Error(const std::string &what) :
    std::runtime_error(what)
{
}

// Local implementations

namespace
{

bool IsFolder(const PathPartPtr &part)
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

}

}
}
}
