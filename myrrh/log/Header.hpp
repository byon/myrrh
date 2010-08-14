// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the declaration of classes Header and TimestampHeader in
 * namespace myrrh::log.
 *
 * $Id: Header.hpp 355 2007-09-17 18:48:35Z byon $
 */

#ifndef MYRRH_LOG_HEADER_HPP_INCLUDED
#define MYRRH_LOG_HEADER_HPP_INCLUDED

#include <iosfwd>
#include <memory>

namespace myrrh
{
namespace log
{

/**
 * This is an interface that is used to write headers of output lines. By
 * header we mean the beginning of each line before the actual log line. There
 * exists a default implementation (myrrh::log::TimestampHeader), but that can
 * be replaced by a customized one deriving a new class from this and then
 * passing it to Log::SetHeader.
 */
class Header
{
public:

    /**
     * Interface to writing header. This method will be called once for each
     * line written through myrrh::log.
     * @param stream The header output should be written into this object
     * @param id This a character id of the verbosity level
     */
    virtual void Write(std::ostream &stream, char id) = 0;
};

typedef std::auto_ptr<Header> HeaderPtr;

/**
 * This is the default line header implementation. It writes a timestamp with
 * millisecond precision followed by the verbosity character identifier in
 * the following format:
 *
 * YYYY.MM.DD hh:mm:ss:SSS I, where the identifiers represent the following:
 * <UL>
 *  <LI> YYYY Year
 *  <LI> MM   Month
 *  <LI> DD   Day of month
 *  <LI> hh   Hour
 *  <LI> mm   Minutes
 *  <LI> ss   seconds
 *  <LI> SSS  milliseconds
 *  <LI> I    The verbosity identifier
 * </UL>
 */
class TimestampHeader : public Header
{
public:

    /**
     * Interface to writing header.
     * @param stream The header output should be written into this object
     * @param id This a character id of the verbosity level
     * @warning Not thread safe
     */
    virtual void Write(std::ostream &stream, char id);
};

}

}


#endif
