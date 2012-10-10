// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the implementation of classes myrrh::log::policy::File
 * and myrrh::log::policy::Opener
 *
 * $Id: Opener.cpp 355 2007-09-17 18:48:35Z byon $
 */

#include "myrrh/log/policy/Opener.hpp"
#include "myrrh/log/policy/Path.hpp"
#include "boost/filesystem/path.hpp"

#include <cassert>

namespace myrrh
{
namespace log
{
namespace policy
{

namespace
{

boost::filesystem::path TryOpening(Opener &opener, policy::Path &path,
                                   std::ofstream &file);
}

class File::Implementation
{
public:

    Implementation(Opener &opener, policy::Path& path);

    std::streamsize Write(const std::string &line);
    std::streamsize WrittenSize( ) const;
    const boost::filesystem::path &Path( ) const;
    bool Compare(const Implementation &other);

private:

    static boost::filesystem::path TryOpening(Opener &opener,
                                              policy::Path &path,
                                              std::ofstream &file);

    std::ofstream file_;
    std::streamsize writtenSize_;
    const boost::filesystem::path PATH_;
};

File::File(Opener &opener, policy::Path& path) :
    implementation_(new Implementation(opener, path))
{
}

std::streamsize File::Write(const std::string &line)
{
    return implementation_->Write(line);
}

bool operator==(const File &left, const File &right)
{
    return left.implementation_->Compare(*right.implementation_);
}

bool operator!=(const File &left, const File &right)
{
    return !(left == right);
}

std::streamsize File::WrittenSize( ) const
{
    return implementation_->WrittenSize( );
}

const boost::filesystem::path &File::Path( ) const
{
    return implementation_->Path( );
}

// File class implementations

File::Implementation::Implementation(Opener &opener, policy::Path& path) :
    writtenSize_(0),
    PATH_(TryOpening(opener, path, file_))
{
    std::streamsize end = file_.tellp( );
    if (end > 0)
    {
        writtenSize_ = end;
    }
}

std::streamsize File::Implementation::Write(const std::string &line)
{
    // It can be a programming error, if the file buffer is not open. But it
    // is possible that the file just could not be opened. Because we are
    // following a no-throw policy, we must prepare for the possibility.
    if (!file_.is_open( ))
    {
        return -1;
    }

    const std::streampos ORIG_POS(file_.tellp( ));
    file_ << line;
    file_.flush( );

    const std::streampos DIFFERENCE(file_.tellp( ) - ORIG_POS);
    writtenSize_ += DIFFERENCE;

    return DIFFERENCE;
}

bool File::Implementation::Compare(const Implementation &other)
{
    return PATH_ == other.PATH_;
}

std::streamsize File::Implementation::WrittenSize( ) const
{
    assert(writtenSize_ >= 0);
    return writtenSize_;
}

const boost::filesystem::path &File::Implementation::Path( ) const
{
    return PATH_;
}

boost::filesystem::path File::Implementation::
TryOpening(Opener &opener, policy::Path &path, std::ofstream &file)
{
    // The opening is done in this separate function, because it is possible
    // to fail for lack of memory. Any other exceptions are programming errors
    // and thus we'll assert if those are caught.

    /// How about just requiring no-throw quarantee from the DoOpen
    /// implementations?
    try
    {
        return opener.DoOpen(*file.rdbuf( ), path);
    }
    catch (const std::bad_alloc&)
    {
        // No memory, nothing that can be done
    }
    catch (...)
    {
        assert(false && "Exception thrown from Opener::DoOpen implementation");
    }

    return "";
}

// Opener class implementation

Opener::~Opener( )
{
}

FilePtr Opener::Open(Path path)
{
    return FilePtr(new (std::nothrow) File(*this, path));
}

}
}
}
