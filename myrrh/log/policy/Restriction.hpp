// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class myrrh::log::policy::Restriction
 *
 * $Id: Restriction.hpp 354 2007-09-17 17:39:58Z byon $
 */

#ifndef MYRRH_LOG_POLICY_RESTRICTION_H_INCLUDED
#define MYRRH_LOG_POLICY_RESTRICTION_H_INCLUDED

// Isolate better

// The pragmas are required to remove warnings from boost::date_time
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4996)
#endif

#include "boost/date_time/gregorian/gregorian_types.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

#include "boost/shared_ptr.hpp"
#include <string>

namespace myrrh
{

namespace log
{

namespace policy
{

// Forward declaration
class File;

/**
 * Restriction class is an interface for restricting the use of a log file
 * after a specific condition. Each subclass is intended for checking of one
 * of these conditions. The objects of Restriction subclasses are usable to
 * define these conditions to myrrh::log::policy::Policy.
 */
class Restriction
{
public:

    /**
     * Virtual destructors are required for interface classes
     */
    virtual ~Restriction( );

    /**
     * Checks if the specific condition is met and the file usage should be
     * restricted.
     * @param file The file to be checked
     * @param toWrite The size of the text that will be written next to the
     *                file.
     * @returns true If the file should be restricted, false otherwise
     */
    /// Consider passing the string to be written instead of size. Then the
    /// real size could be calculated (line endings) in those restrictions
    /// that really require it.
    virtual bool IsRestricted(const File &file, std::size_t toWrite) const = 0;
};

typedef boost::shared_ptr<Restriction> RestrictionPtr;

/**
 * This file implements the size restriction. If the file size plus the size
 * to be written exceeds a the max size, the file should be restricted. This
 * class can be used to tell myrrh::log::policy::Policy class to restrict the
 * log files sizes. The resulting action (resizing or starting a new file) is
 * the responsibility of other classes.
 */
class SizeRestriction : public Restriction
{
public:

    /**
     * Constructor
     * @param maxSize The maximum size for the restriction
     */
    explicit SizeRestriction(std::size_t maxSize);

    /**
     * Checks if the size of the text to be written fits into file.
     * @param file The file to be checked
     * @param toWrite The size of the text that will be written next to the
     *                file.
     * @returns true If the size does not fit into the file, false otherwise
     */
    virtual bool IsRestricted(const File &file, std::size_t toWrite) const;

private:

    /// Copy construction is prevented
    SizeRestriction(const SizeRestriction &);
    /// Assignment is prevented
    SizeRestriction &operator=(const SizeRestriction &);

    /// The maximum size of a file
    const std::size_t MAX_SIZE_;
};

class DateCreator
{
public:
    typedef boost::gregorian::date Date;
    static boost::gregorian::date NewDate( );
};

/**
 * Defines the file restricted after every date change.
 */
template <typename Creator = DateCreator>
class DateRestriction : public Restriction
{
public:

    typedef typename Creator::Date Date;

    /**
     * Constructor
     */
    DateRestriction( );

    /**
     * Checks if the the date has changed since the last check
     * @param file The file to be checked
     * @param toWrite The size of the text that will be written next to the
     *                file. Ignored.
     * @returns true If the date has changed, false otherwise
     */
    virtual bool IsRestricted(const File &file, std::size_t toWrite) const;

private:

    /// Copy construction is prevented
    DateRestriction(const DateRestriction &);
    /// Assignment is prevented
    DateRestriction &operator=(const DateRestriction &);

    mutable Date date_;
};

// Inline implementations

// Actually there is no need to inline this -> move to cpp and remove the
// boost date_time header inclusion
inline boost::gregorian::date DateCreator::NewDate( )
{
    return boost::gregorian::day_clock::local_day( );
}

template <typename Creator>
inline DateRestriction<Creator>::DateRestriction( ) :
    date_(Creator::NewDate( ))
{
}

template <typename Creator>
inline bool DateRestriction<Creator>::
IsRestricted(const File &, std::size_t) const
{
    auto today = Creator::NewDate( );
    if (date_ == today)
    {
        return false;
    }

    date_ = today;
    return true;
}

}

}

}

#endif
