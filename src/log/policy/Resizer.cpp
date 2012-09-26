// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class myrrh::log::policy::Resizer
 *
 * $Id: Resizer.cpp 360 2007-09-19 18:44:33Z byon $
 */

#include "myrrh/log/policy/Resizer.hpp"
#include "myrrh/log/policy/Path.hpp"
#include "myrrh/file/Resizer.hpp"
#include "myrrh/file/PositionScanner.hpp"

#include "boost/filesystem/operations.hpp"

#define DISABLE_SIGNED_UNSIGNED_MISMATCH
#include "myrrh/util/Preprocessor.hpp"

#include "boost/filesystem/convenience.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

namespace myrrh
{

namespace log
{

namespace policy
{

using namespace file;

// Local declarations

namespace
{

file::Resizer NewResizer(const boost::filesystem::path &path,
                         std::streamsize sizeLeftAfter);
}

// Class implementations

Resizer::Resizer(std::streamsize sizeLeftAfter) :
    SIZE_LEFT_AFTER_(sizeLeftAfter)
{
}

// Divide smaller
boost::filesystem::path Resizer::DoOpen(std::filebuf &file, Path& path)
{
    const boost::filesystem::path PATH(path.Generate( ));
    assert(!PATH.empty( ));

    if (boost::filesystem::exists(PATH))
    {
        // The myrrh::file::Resizer throws if the file does not exist. It is
        // not an error in this case. Normally myrrh::log::Resizer is used
        // to resize the content of a file in a fixed location. But it could
        // also be that the file location is changed by some logic (for
        // instance if the folder is named by the current date). Therefore we
        // must be able to handle this kind of situation and just open a new
        // file. Also, if we were to throw here, the no-throw guarantee would
        // be broken.
        file::Resizer resizer(NewResizer(PATH, SIZE_LEFT_AFTER_));
        resizer( );
    }
    else
    {
        try
        {
            // The creation of the directory path may throw, we will silently
            // ignore the possible exceptions.
            if (PATH.has_parent_path( ))
            {
                // There is also a version with no-throw guarantee
                boost::filesystem::create_directories(PATH.parent_path( ));
            }
        }
        catch (...)
        {
            return PATH;
        }
    }

    using namespace std;
    file.open(PATH.string( ).c_str( ), ios::out | ios::app | ios::ate);

    return PATH;
}

// Local implementations

namespace
{

inline file::Resizer NewResizer(const boost::filesystem::path &path,
                                std::streamsize sizeLeftAfter)
{
    PositionScannerPtr startScanner(new ScanFromEnd(sizeLeftAfter));
    PositionScannerPtr endScanner(new EndScanner);
    return file::Resizer(path, startScanner, endScanner);
}

}

}

}

}
