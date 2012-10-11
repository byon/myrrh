// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "myrrh/log/policy/PathEntity.hpp"
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
boost::regex operator+(const boost::regex &left, const boost::regex &right);

/**
 * @todo Refactor. The code is a bit too complex to understand
 */
std::string FirstMatch(const boost::regex &expression,
                       const std::string &toMatch);
Comparison CompareAndUpdate(const PathPart &part, std::string &left,
                            std::string &right);
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
    using namespace boost::filesystem;
    return [&](const path& left, const path& right)
    {
        return this->IsFirstEarlier(left, right);
    };
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

// Local implementations

namespace
{

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
