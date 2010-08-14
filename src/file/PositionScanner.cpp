// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class myrrh::file::PositionScanner and
 * it's subclasses.
 *
 * $Id: PositionScanner.cpp 360 2007-09-19 18:44:33Z byon $
 */

#include "myrrh/file/PositionScanner.hpp"

#include <fstream>
#include <limits>

namespace myrrh
{
namespace file
{

namespace
{

/**
 * Checks if the given file stream is open, otherwise throws an exception.
 * @throws PositionScanner::NotOpen, if stream is not open
 */
void CheckStream(const std::ifstream &stream);

/**
 * Returns the end position of the given file stream, i.e. it's size.
 */
std::streampos EndPosition(std::ifstream &stream);

/**
 * Seeks the position of beginning of next start of line in the given file
 * stream from the given point. The position is returned. If there are no
 * new lines after the given point, returns the point at end of file.
 * @param stream Stream to be seeked
 * @param toSeek The point to seek to
 * @return The on which the new line starts or the file ends
 */
std::streampos SeekNextLineStart(std::ifstream &stream, std::streampos toSeek);

/**
 * Stores the position of a stream and reverts it after destruction. The
 * purpose of the class is to act as exception safe mechanism for the stream
 * position reversion.
 */
class StreamStateReset
{
public:

    /**
     * Constructor
     */
    StreamStateReset(std::ifstream &stream);

    /**
     * Destructor
     */
    ~StreamStateReset( );

private:

    /// Disabled copy constructor
    StreamStateReset(const StreamStateReset &);
    /// Disabled assignment operator
    StreamStateReset &operator=(const StreamStateReset &);

    std::ifstream &stream_;
    const std::streampos POSITION_;
};

}

PositionScanner::~PositionScanner( )
{
}

std::streampos PositionScanner::Scan(std::ifstream &stream) const
{
    CheckStream(stream);
    StreamStateReset stateReset(stream);
    return DoScan(stream);
}

PositionScanner::NotOpen::NotOpen(const std::string &what) :
    std::runtime_error(what)
{
}

ToEdgeScanner::ToEdgeScanner(std::ios::seekdir direction) :
    DIRECTION_(direction)
{
}

std::streampos ToEdgeScanner::DoScan(std::ifstream &stream) const
{
    stream.seekg(0, DIRECTION_);

    return stream.tellg( );
}

StartScanner::StartScanner( ) :
    ToEdgeScanner(std::ios::beg)
{
}

EndScanner::EndScanner( ) :
    ToEdgeScanner(std::ios::end)
{
}

ScanFromStart::ScanFromStart(std::streamsize point) :
    POINT_(point)
{
}

std::streampos ScanFromStart::DoScan(std::ifstream &stream) const
{
    const std::streampos END_POS = EndPosition(stream);

    if (POINT_ >= END_POS)
    {
        return END_POS;
    }

    return SeekNextLineStart(stream, POINT_);
}

ScanFromEnd::ScanFromEnd(std::streamsize bytesFromEnd) :
    BYTES_FROM_END_(bytesFromEnd)
{
}

std::streampos ScanFromEnd::DoScan(std::ifstream &stream) const
{
    const std::streampos END_POS = EndPosition(stream);

    if (BYTES_FROM_END_ >= END_POS)
    {
        return END_POS;
    }

    return SeekNextLineStart(stream, END_POS - BYTES_FROM_END_);
}

namespace
{

inline void CheckStream(const std::ifstream &stream)
{
    if (!stream.is_open( ))
    {
        throw PositionScanner::NotOpen("Scanning failed, file not open");
    }
}

inline std::streampos EndPosition(std::ifstream &stream)
{
    stream.seekg(0, std::ios::end);
    return stream.tellg( );
}

inline StreamStateReset::StreamStateReset(std::ifstream &stream) :
    stream_(stream),
    POSITION_(stream.tellg( ))
{
}

inline StreamStateReset::~StreamStateReset( )
{
    stream_.seekg(POSITION_);
}

inline std::streampos SeekNextLineStart(std::ifstream &stream,
                                        std::streampos toSeek)
{
    stream.seekg(toSeek);
    stream.ignore(std::numeric_limits<int>::max( ), '\n');

    return stream.tellg( );
}

}

}
}
