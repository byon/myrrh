// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class
 * myrrh::log::policy::RestrictionAppender
 *
 * $Id: RestrictionAppender.hpp 286 2007-03-18 15:04:53Z Byon $
 */

#ifndef MYRRH_LOG_POLICY_RESTRICTIONAPPENDER_H_INCLUDED
#define MYRRH_LOG_POLICY_RESTRICTIONAPPENDER_H_INCLUDED

namespace myrrh
{

namespace log
{

namespace policy
{

// Forward declaration
class RestrictionStore;

/**
 * This class is an interface to classes that can add new restrictions to
 * restriction stores. The restriction in this context means a condition that
 * requires the myrrh::log::policy::Policy class to reopen the current log
 * file. Such a restriction could be for instance a maximum size for a log file
 * that should not be exceeded.
 * Currently RestrictionAppender interface is implemented only by PathPart
 * classes. The reasoning is that some PathPart subclasses have a logic that
 * by nature add a new restriction. An example is Date class. If the log file
 * path contains the current date, then a that log file should be restricted
 * once a new date is entered.
 */
class RestrictionAppender
{
public:

    /**
     * Virtual destructor is needed for interface classes
     */
    virtual ~RestrictionAppender( );

    /**
     * Possibly adds one or more restrictions to the given store. It is
     * entirely legal to not add anything or to add several restrictions.
     * @param store The store into which possible restrictions are added
     */
    virtual void AppendRestrictions(RestrictionStore &store) = 0;
};

// Inline implementations

inline RestrictionAppender::~RestrictionAppender( )
{
}

}

}

}

#endif
