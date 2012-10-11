// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * The file contains implementation of class myrrh::log::policy::Policy
 *
 * $Id: Policy.cpp 355 2007-09-17 18:48:35Z byon $
 */

#include "myrrh/log/policy/Policy.hpp"
#include "myrrh/log/policy/File.hpp"
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

std::streamsize AdjustSize(const std::string &toWrite, std::streamsize written);

#ifdef WIN32
const bool ADJUSTING_NEEDED = true;
#else
const bool ADJUSTING_NEEDED = false;
#endif

/**
 * Class used for adjusting the size passed to File::Write. The default
 * implementation is used if size adjusting is needed, otherwise the
 * specialization is used.
 */
// Is this really worth the template specialization? Are there many places
// where ADJUSTING_NEEDED would be compared?
template <bool AdjustingNeeded = true>
class SizeAdjusterImpl
{
public:

    explicit SizeAdjusterImpl(const std::string &toWrite);
    std::streamsize GetSize( ) const;
    std::streamsize Adjust(std::streamsize written) const;

private:

    SizeAdjusterImpl(const SizeAdjusterImpl &);
    SizeAdjusterImpl &operator=(const SizeAdjusterImpl &);

    const std::string &TEXT_;
    const std::streamsize LINES_;
};

/**
 * The specialization for SizeAdjusterImpl that does no size adjusting.
 */
template <>
class SizeAdjusterImpl<false>
{
public:

    explicit SizeAdjusterImpl(const std::string &toWrite);
    std::streamsize GetSize( ) const;
    std::streamsize Adjust(std::streamsize written) const;

private:

    SizeAdjusterImpl(const SizeAdjusterImpl &);
    SizeAdjusterImpl &operator=(const SizeAdjusterImpl &);

    const std::string &TEXT_;
};

typedef SizeAdjusterImpl<ADJUSTING_NEEDED> SizeAdjuster;

}

// Class implementations

Policy::Policy(Path path, InitialOpenerPtr initialOpener,
               OpenerPtr subsequentOpener) :
    path_(path),
    subsequentOpener_(subsequentOpener),
    file_(initialOpener->Open(path_))
{
    path_.AppendRestrictions(restrictions_);
}

void Policy::AddRestriction(RestrictionPtr restriction)
{
    restrictions_.Add(restriction);
}

// Divide smaller
std::streamsize Policy::Write(const std::string &toWrite)
{
    boost::filesystem::path originalPath(file_->Path( ));
    int counter = 0;
    // On windows the possible line endings will have size of two ("\n\r").
    // Because of this the size may need to be adjusted so that the original
    // text size is returned.
    const SizeAdjuster ADJUSTER(toWrite);

    // Is the file size counting the responsibility of this class? It is
    // only used by the policies which are based on file size.
    while (restrictions_.IsRestricted(*file_, ADJUSTER.GetSize( )))
    {
        // The file needs to be explicitly destructed before opening the next
        // file. This is needed, because the File object owns an open stream
        // to the underlying file. If the new File object needs to access the
        // same file and modify it somehow (like Resizer does), this will fail
        // as there already exists an open stream.
        file_.reset( );
        file_ = subsequentOpener_->Open(path_);
        if (!file_)
        {
            // No memory
            return -1;
        }

        // The loop brings the possibility of infinite loop if the Opener
        // object does not truly open the next file. If the Opener object is
        // supposed only to modify the file somehow (like Resizer does), this
        // option is still legal for one time. The check is not made for
        // the first opening try. It is considered sufficient accommodation for
        // these kind of cases.
        // Because this error is likely a cause of programming error, there is
        // an assertion check. Also in some rare cases the underlying hardware
        // might just be malfunctioning, therefore in release builds this error
        // is silently ignored. As this function has a no-throw guarantee,
        // throwing an exception is not an option.
        if (counter++ != 0 && file_->Path( ) == originalPath)
        {
            assert("Infinite loop noticed in Policy::Write" && false);
            return 0;
        }
    }

    return AdjustSize(toWrite, file_->Write(toWrite));
}

// Local implementations

namespace
{

inline std::streamsize AdjustSize(const std::string &toWrite,
                                  const std::streamsize written)
{
    const std::streamsize TEXT_SIZE =
        static_cast<std::streamsize>(toWrite.size( ));

    if (TEXT_SIZE == written)
    {
        return TEXT_SIZE;
    }

    const std::size_t LINES =
        std::count(toWrite.begin( ), toWrite.end( ), '\n');

    if (static_cast<std::streamsize>(LINES) + TEXT_SIZE == written)
    {
        return TEXT_SIZE;
    }

    return written;
}

template <bool AdjustingNeeded>
inline SizeAdjusterImpl<AdjustingNeeded>::
SizeAdjusterImpl(const std::string &toWrite) :
    TEXT_(toWrite),
    LINES_(static_cast<std::streamsize>(std::count(toWrite.begin( ),
                                                   toWrite.end( ), '\n')))
{
}

template <bool AdjustingNeeded>
inline std::streamsize SizeAdjusterImpl<AdjustingNeeded>::GetSize( ) const
{
    return static_cast<std::streamsize>(TEXT_.size( ) + LINES_);
}

template <bool AdjustingNeeded>
inline std::streamsize
SizeAdjusterImpl<AdjustingNeeded>::Adjust(std::streamsize written) const
{
    if ((TEXT_.size( ) == written) ||
        LINES_ + TEXT_.size( ) == written)
    {
        return TEXT_.size( );
    }

    return written;
}

inline SizeAdjusterImpl<false>::SizeAdjusterImpl(const std::string &toWrite) :
    TEXT_(toWrite)
{
}

inline std::streamsize SizeAdjusterImpl<false>::GetSize( ) const
{
    return static_cast<std::streamsize>(TEXT_.size( ));
}

inline std::streamsize
SizeAdjusterImpl<false>::Adjust(std::streamsize written) const
{
    return written;
}

}

}

}

}
