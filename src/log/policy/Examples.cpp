// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * Contains implementations of a number of example functions that each create a
 * certain type of myrrh::log::policy::Policy object.
 *
 * $Id: Examples.cpp 369 2007-09-20 20:08:51Z byon $
 */

#include "myrrh/log/policy/Examples.hpp"
#include "myrrh/log/policy/Restriction.hpp"
#include "myrrh/log/policy/Appender.hpp"
#include "myrrh/log/policy/Creator.hpp"
#include "myrrh/log/policy/Resizer.hpp"
#include "myrrh/log/policy/Path.hpp"
#include "myrrh/log/policy/PathPart.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

// Local declarations

namespace
{

std::string DefaultName( );
std::string DefaultPrefix( );
std::string DefaultPostfix( );
PolicyPtr AddSizeRestriction(Path path, InitialOpenerPtr initialOpener,
                             OpenerPtr subsequentOpener, std::size_t maxSize);

}

// Public function implementations

PolicyPtr DatedFolderLog( )
{
    Path path;
    path += Date( ) + "/" + DefaultName( );

    InitialOpenerPtr opener(new Appender);

    return PolicyPtr(new Policy(path, opener, opener));
}

PolicyPtr SizeRestrictedLog(std::size_t maxSize)
{
    Path path;
    path += DefaultName( );

    InitialOpenerPtr initialOpener(new Appender);
    OpenerPtr subsequentOpener(
        new Resizer(static_cast<std::streamsize>(maxSize / 2)));

    return AddSizeRestriction(path, initialOpener, subsequentOpener, maxSize);
}

PolicyPtr SizeRestrictedDatedFolderLog(std::size_t maxSize)
{
    Path path;
    path += Date( ) + "/" + DefaultName( );

    InitialOpenerPtr initialOpener(new Appender);
    OpenerPtr subsequentOpener(
        new Resizer(static_cast<std::streamsize>(maxSize / 2)));

    return AddSizeRestriction(path, initialOpener, subsequentOpener, maxSize);
}

PolicyPtr SizeRestrictedLogs(std::size_t maxSize)
{
    Path path;
    path += DefaultPrefix( ) + Time( ) + DefaultPostfix( );

    InitialOpenerPtr initialOpener(new Appender);
    OpenerPtr subsequentOpener(new Creator);

    return AddSizeRestriction(path, initialOpener, subsequentOpener, maxSize);
}

PolicyPtr SizeRestrictedDatedFolderLogs(std::size_t maxSize)
{
    Path path;
    path += Date( ) + "/" + DefaultPrefix( ) + Time( ) + DefaultPostfix( );

    InitialOpenerPtr initialOpener(new Appender);
    OpenerPtr subsequentOpener(new Creator);

    return AddSizeRestriction(path, initialOpener, subsequentOpener, maxSize);
}

// Local implementations

namespace
{

std::string DefaultName( )
{
    return DefaultPrefix( ) + DefaultPostfix( );
}

std::string DefaultPrefix( )
{
    return "myrrh";
}

std::string DefaultPostfix( )
{
    return ".log";
}

PolicyPtr AddSizeRestriction(Path path, InitialOpenerPtr initialOpener,
                             OpenerPtr subsequentOpener, std::size_t maxSize)
{
    PolicyPtr policy(new Policy(path, initialOpener, subsequentOpener));

    RestrictionPtr restriction(new SizeRestriction(maxSize));
    policy->AddRestriction(restriction);

    return policy;
}

}

}

}

}
