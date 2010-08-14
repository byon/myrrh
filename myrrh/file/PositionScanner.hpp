// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class myrrh::file::PositionScanner and
 * it's subclasses.
 *
 * $Id: PositionScanner.hpp 360 2007-09-19 18:44:33Z byon $
 */

#ifndef MYRRH_FILE_POSITIONSCANNER_H_INCLUDED
#define MYRRH_FILE_POSITIONSCANNER_H_INCLUDED

#include "boost/shared_ptr.hpp"

#include <ios>
#include <string>

namespace myrrh
{
namespace file
{

/**
 * Declares an interface used for scanning a file stream into a specific point.
 * For example a subclass could scan to file end, to file start, to start of
 * 11th line in file, etc.
 * @note NVI used for interface
 */
class PositionScanner
{
public:

    /**
     * Destructor declared virtual, because this is an interface class
     */
    virtual ~PositionScanner( );

    /**
     * Scans the given stream and returns the point searched for. The position
     * of the stream is reverted after scanning, even if exceptions occur.
     * @param stream The file stream to scan
     * @return The position searched for
     * @throws NotOpen if given stream is not open
     */
    std::streampos Scan(std::ifstream &stream) const;

    /**
     * Exception class, which is used, if the stream passed to Scan is not open
     */
    class NotOpen : public std::runtime_error
    {
    public:
        NotOpen(const std::string &what);
    };

private:

    /**
     * The actual implementation of the scanning. Implemented by subclasses
     * @param stream The file stream to scan
     * @return The position searched for
     * @throws NotOpen if given stream is not open
     */
    virtual std::streampos DoScan(std::ifstream &stream) const = 0;
};

typedef boost::shared_ptr<PositionScanner> PositionScannerPtr;

/**
 * Scans to either start or end of a file stream. Not usable by itself, use the
 * subclasses StartScanner and EndScanner.
 */
class ToEdgeScanner : public PositionScanner
{
protected:

    /**
     * Constructor
     * @param direction Identifies the direction to scan to.
     */
    ToEdgeScanner(std::ios::seekdir direction);

private:

    /**
     * Implements the scanning
     * @param stream The file stream to scan
     * @return The position searched for
     */
    virtual std::streampos DoScan(std::ifstream &stream) const;

    std::ios::seekdir DIRECTION_;
};

/**
 * Scans to the beginning of the file
 */
class StartScanner : public ToEdgeScanner
{
public:

    /**
     * Constructor
     */
    StartScanner( );
};

/**
 * Scans to the end of the file
 */
class EndScanner : public ToEdgeScanner
{
public:

    /**
     * Constructor
     */
    EndScanner( );
};

/**
 * Scans the file stream from beginning to a specified position.
 * The scanning is continued to the start of next line, or the end of file, if
 * there are no more lines.
 */
class ScanFromStart : public PositionScanner
{
public:

    /**
     * Constructor
     * @param point The point to which the scanning is done.
     */
    ScanFromStart(std::streamsize point);

private:

    /**
     * Implements the scanning
     * @param stream The file stream to scan
     * @return The position searched for
     */
    virtual std::streampos DoScan(std::ifstream &stream) const;

    /// Disabled copy constructor
    ScanFromStart(const ScanFromStart &);
    /// Disabled assignment operator
    ScanFromStart &operator=(const ScanFromStart &);

    const std::streamsize POINT_;
};

/**
 * Scans the file stream from end to a specified position.
 * The scanning is continued (or backtracked as we are scanning from end?) to
 * the start of next line, or the end of file, if  there are no more lines.
 */
class ScanFromEnd : public PositionScanner
{
public:

    /**
     * Constructor
     * @param point The point to which the scanning is done.
     */
    ScanFromEnd(std::streamsize bytesFromEnd);

private:

    /**
     * Implements the scanning
     * @param stream The file stream to scan
     * @return The position searched for
     */
    virtual std::streampos DoScan(std::ifstream &stream) const;

    /// Disabled copy constructor
    ScanFromEnd(const ScanFromEnd &);
    /// Disabled assignment operator
    ScanFromEnd &operator=(const ScanFromEnd &);

    const std::streamoff BYTES_FROM_END_;
};

}
}

#endif
