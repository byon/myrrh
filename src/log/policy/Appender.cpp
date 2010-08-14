// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class myrrh::log::Appender
 *
 * $Id: Appender.cpp 340 2007-09-11 19:51:16Z byon $
 */

#include "myrrh/log/policy/Appender.hpp"
#include "myrrh/log/policy/Path.hpp"
#include "myrrh/file/MatchFiles.hpp"

#define DISABLE_SIGNED_UNSIGNED_MISMATCH
#include "myrrh/util/Preprocessor.hpp"

#include "boost/filesystem/convenience.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

#include "boost/filesystem/operations.hpp"

#include <cassert>

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
 * Uses the rules in the given Path object to find an existing file that best
 * matches the file into which the appending will be done. If no existing file
 * matches, a new file path is created.
 * Provides no-throw guarantee. The function exists purely to hide any
 * exceptions possibly thrown from SelectPathToUse.
 * @param path Gives the rules for identifying the correct file to append to,
 *             if such exists.
 * @return Path to an existing file that best matches the rules in Path object.
 *         If no such file exists, a new path is returned.
 */
boost::filesystem::path SelectPathToUseHideErrors(Path &path);

/**
 * Uses the rules in the given Path object to find an existing file that best
 * matches the file into which the appending will be done. If no existing file
 * matches, a new file path is created.
 * @param path Gives the rules for identifying the correct file to append to,
 *             if such exists.
 * @return Path to an existing file that best matches the rules in Path object.
 *         If no such file exists, a new path is returned.
 */
boost::filesystem::path SelectPathToUse(Path &path);

/**
 * If the given Path object contains a parent path, it is returned. Otherwise
 * returns ".".
 */
boost::filesystem::path SelectParentPath(const Path &path);

/**
 * Creates the required path directory structure.
 * Provides no-throw guarantee
 * @param Path to the directory to be created
 * @return true, if the creation was possible
 */
bool CreateDirectoryTree(const boost::filesystem::path &directory);

}

// Class implementations

Appender::Appender( )
{
}

boost::filesystem::path Appender::DoOpen(std::filebuf &file, Path& path)
{
    const boost::filesystem::path PATH(SelectPathToUseHideErrors(path));
    CreateDirectoryTree(PATH.branch_path( ));
    using namespace std;
    file.open(PATH.string( ).c_str( ), ios::out | ios::app | ios::ate);

    return PATH;
}

// Local implementations
namespace
{

boost::filesystem::path SelectPathToUseHideErrors(Path &path)
{
    try
    {
        return SelectPathToUse(path);
    }
    catch (...)
    {
    }

    return path.Generate( );
}

boost::filesystem::path SelectPathToUse(Path &path)
{
    boost::filesystem::path folder(SelectParentPath(path));
    if (!boost::filesystem::exists(folder))
    {
        return path.Generate( );
    }

    for (Path::EntityIterator i(path.BeginEntity( ));
         path.EndEntity( ) != i;
         ++i)
    {
        using namespace file;
        PathStore matches(MatchFiles(folder, i->Matcher( )));
        if (matches.empty( ))
        {
            break;
        }

        PathStore::iterator match(std::max_element(matches.begin( ),
                                                   matches.end( ),
                                                   i->GetComparer( )));

        if (!boost::filesystem::is_directory(*match))
        {
            if (i + 1 != path.EndEntity( ))
            {
                break;
            }

            return *match;
        }

        folder = *match;
    }

    return path.Generate( );
}

boost::filesystem::path SelectParentPath(const Path &path)
{
    const boost::filesystem::path PARENT_PATH(path.ParentPath( ));
    if (PARENT_PATH.empty( ))
    {
        return ".";
    }

    return PARENT_PATH;
}

bool CreateDirectoryTree(const boost::filesystem::path &directory)
{
    try
    {
        boost::filesystem::create_directories(directory);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

}

}

}

}
