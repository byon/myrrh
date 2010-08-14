// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of class myrrh::log::policy::Policy
 *
 * $Id: Policy.hpp 369 2007-09-20 20:08:51Z byon $
 */

#ifndef MYRRH_LOG_POLICY_POLICY_HPP_INCLUDED
#define MYRRH_LOG_POLICY_POLICY_HPP_INCLUDED

#include "myrrh/log/policy/RestrictionStore.hpp"
#include "myrrh/log/policy/Opener.hpp"
#include <string>

namespace myrrh
{

namespace log
{

namespace policy
{

/**
 * Policy class is the container of all the rules that can be combined from
 * myrrh::log::policy component for handling of log files. The rules include
 * the logic in which the log files are opened, the logic deciding when to
 * move to another file or to crop the current context shorter and the logic
 * on how to name the log files. By itself the Policy class is not very usable.
 * It is much easier to use it in co-operation with myrrh::log::policy::Buffer
 * and myrrh::log::policy::Stream.
 *
 * The different functionalities of Policy are separated into different
 * classes. The subclasses of Opener know how the file can be opened. There
 * are two different kinds of Opener classes. The InitialOpeners can be used
 * to make the initial (i.e. the first opening during the application
 * execution) opening of a log file. The rest can be used to make the
 * subsequent openings. Note that the subsequent openings are not required to
 * open the same physical file. This allows for build a log policy in which
 * the log writing can move from one file to another when certain conditions
 * are met. It is also possible to resize the file on the subsequent openings.
 *
 * The conditions that define when a log file should be reopened are defined
 * by subclasses of Restriction. Some of the classes used in conjunction with
 * Policy are also RestrictionAppenders, that is they automatically add new
 * Restrictions once they are added to Policy. An example is the Date class
 * (subclass of PathPart) that automatically adds a restriction that once the
 * the date changes, you should reopen the file, because the rules of the path
 * making require the path to contain the current date. This of course means
 * writing to a new file. You can add new restrictions by using method
 * AddRestriction. See it's documentation for more details.
 *
 * The rules on how to name the log files are implemented by subclasses of
 * PathPart and contained in Path class. These rules are passed to the opener
 * objects on construction of Policy object. Each PathPart implementes some
 * logic for creating one small part of a file path. See the documentation
 * of PathPart and it's subclasses for details.
 *
 * All of the operations of Policy provide a no-throw guarantee provided that
 * the classes passed to it provide the same guarantee.
 *
 * You can find examples on how to use the Policy class and related classes
 * from src/log/policy/Examples.cpp.
 */
class Policy
{
public:

    /**
     * Constructor.
     * @param path Contains the rules, which describe how to locate and name
     *             the log files
     * @param initialOpener Contains an object that knows how to open the
     *                      initial file for writing
     * @param subsequentOpener Contains an object that knows how to open a
     *                         file once the (possible) restrictions require it
     */
    Policy(Path path, InitialOpenerPtr initialOpener,
           OpenerPtr subsequentOpener);

    /**
     * Adds a new restriction to the log policy. A restriction means a
     * condition that requires the target file to be "reopened". For example,
     * if there is a max size of log files, there should be a restriction that
     * checks if the new data to be written fits into the file or not. Some of
     * the restrictions are implicitly provided by the other objects used in
     * conjunction with Policy class. There can be any number of restrictions.
     * The restrictions are checked before each log writing. If any
     * restrictions apply, the file is reopened for writing.
     * @param restriction Defines a logic for restricting log file usage
     */
    void AddRestriction(RestrictionPtr restriction);

    /**
     * Writes the given text to a log file. If any of the contained
     * restrictions apply, the log file is reopened and the text is written to
     * the "new file". It is possible that this is the same file, just cropped
     * to a smaller size. Note that the text is always written in entirety to
     * one physical file.
     * @param toWrite The text to be written to the log
     * @returns The size written to log. If this equals the size of given text,
     *          the operation was successfull.
     */
    std::streamsize Write(const std::string &toWrite);

private:

    /// Disabled copying
    Policy(const Policy& policy);
    /// Disabled assignment
    Policy& operator=(const Policy&);

    Path path_;
    /// Stores the stored restrictions
    RestrictionStore restrictions_;
    /// The object that implements subsequent openers
    OpenerPtr subsequentOpener_;
    /// The currently open log object
    FilePtr file_;
};

typedef boost::shared_ptr<Policy> PolicyPtr;

}

}

}

#endif
