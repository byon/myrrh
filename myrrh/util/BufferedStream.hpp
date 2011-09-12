// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration and implementation of class
 * myrrh::log::BufferedStream
 *
 * $Id: BufferedStream.hpp 365 2007-09-20 18:14:42Z byon $
 */

#ifndef MYRHH_UTIL_BUFFEREDSTREAM_HPP_INCLUDED
#define MYRHH_UTIL_BUFFEREDSTREAM_HPP_INCLUDED

#include <iosfwd>
#include <string>
#include <cstdio>

namespace myrrh
{

namespace util
{

/**
 * BufferedStream is a base class for classes that implement a buffered output
 * stream. It is meant to simplify sublassing from std::streambuf. All of the
 * subclasses need to handle the buffering in some way. This class provides
 * one solution that is reusable.
 * @note The current implementation just uses a std::string for storing the
 *       buffer. Other ways would probably improve performance. The current
 *       implementation was fast and safe and was thus selected.
 */
class BufferedStream : public std::streambuf
{

protected:

    /**
     * Stores the given character into our own buffer. The accumulation of
     * the characters will finally end up as the complete error string. Eof
     * will be ignored.
     * @param character The character to be added to the buffer
     * @return The given character
     */
    virtual int_type overflow(int_type character);

    /**
     * Writes a buffer of characters into our own buffer. The buffer will
     * finally hold the completed error string.
     * @param text The buffer of characters to be written
     * @param length The length of text
     * @return The count of characters written
     */
    virtual std::streamsize xsputn(const char *text, std::streamsize length);

    /**
     * Takes care of the output implementation. Currently this method
     * only takes care of the buffer handling and then passes the work to
     * SyncImpl( ).
     * @return 0 If succeeded, otherwise -1.
     */
    virtual int sync( );

    /**
     * Returns the currents contents of the buffer
     */
    const std::string &GetBuffer( ) const;

private:

    /**
     * Implements the actual output. Subclasses need to implement this method.
     * @return 0 If succeeded, otherwise -1.
     */
    virtual int SyncImpl( ) = 0;

    /** Our buffer for the error text */
    std::string text_;
};

// Inline implementations

inline std::streambuf::int_type
BufferedStream::overflow(std::streambuf::int_type character)
{
    if (EOF != character)
    {
        text_ += static_cast<char>(character);
    }

    return character;
}

inline std::streamsize BufferedStream::xsputn(const char *text,
                                              std::streamsize length)
{
    text_.append(text, length);

    return length;
}

inline int BufferedStream::sync( )
{
    if (!text_.length( ))
    {
        // There's nothing to do
        return 0;
    }

    if (!SyncImpl( ))
    {
        // Success, clear our buffer
        text_.clear( );
        return 0;
    }

    return -1;
}

inline const std::string &BufferedStream::GetBuffer( ) const
{
    return text_;
}

}

}

#endif
