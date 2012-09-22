// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * Contains declarations of a number of example functions that each create a
 * certain type of myrrh::log::policy::Policy object.
 *
 * The functions are rather restricted (i.e. not configurable) because they are
 * meant to server mainly as examples. If more complexity is required (to
 * change the parent folder or the log names etc.) the user must use the
 * classes declared in myrrh::log::policy directly.
 * It was thought that creating an "easy-to-use" interface on top of the
 * policy classes would only add code that needs maintaining without bringing
 * much true usability benefit.
 *
 * $Id: Examples.hpp 369 2007-09-20 20:08:51Z byon $
 */

#ifndef MYRRH_LOG_POLICY_EXAMPLES_HPP_INCLUDED
#define MYRRH_LOG_POLICY_EXAMPLES_HPP_INCLUDED

// The include header is not strictly needed here. However as this file is not
// useful without including the policy header as well, it is acceptable to
// include it here in the name of ease of use.
#include "myrrh/log/policy/Policy.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

/**
 * Constructs a log policy that always uses the same file. If the file exists
 * at startup, it is overwritten. The file size is not restricted. The file is
 * written to the execution directory of the program with name "myrrh.log".
 * @return A policy object that can be used for writing a log file.
 */
PolicyPtr DatedFolderLog( );

/**
 * Constructs a log policy that always uses the same file. If the file exists
 * at startup, the new entries are appended to the end. If the file reaches
 * maximum size, it is resized to half of the maximum size. The file is
 * written to the execution directory of the program with name "myrrh.log".
 * @param maxSize Maximum size in kilobytes for the file
 * @return A policy object that can be used for writing a log file.
 */
PolicyPtr SizeRestrictedLog(std::size_t maxSize);

/**
 * As SizeRestrictedLog, excepth the log file is written to a subfolder named
 * after current date.
 * @param maxSize Maximum size in kilobytes for the file
 * @return A policy object that can be used for writing a log file.
 */
PolicyPtr SizeRestrictedDatedFolderLog(std::size_t maxSize);

/**
 * As SizeRestrictedLog, except a new log file is started once maximum size is
 * reached a new file is started. The files are identified from each other by
 * time stamps.
 *
 * @todo Check whether the timestamp format documentation is still valid
 * The files are named by the following format:
 * myrrhHHMMSS-IIIIII-n, where
 *  HH is hours
 *  MM is minutes
 *  SS is secons
 *  IIIIII is microseconds
 *  n is running counter, usually 0
 * An example is "myrrh173218-704322-0.log", which is written at ca. 17:32 in
 * the afternoon.
 *
 * @param maxSize Maximum size in kilobytes for the file
 * @return A policy object that can be used for writing a log file.
 */
PolicyPtr SizeRestrictedLogs(std::size_t maxSize);

/**
 * As SizeRestrictedDatedFolderLog, except a new log file is started once
 * maximum size is reached a new file is started. The files are identified from
 * each other by time stamps.
 *
 * @todo Check whether the timestamp format documentation is still valid
 * The files are named by the following format:
 * YYYYmmDD/myrrhHHMMSS-IIIIII-n, where
 *  YYYY is year
 *  mm is months
 *  DD is days
 *  HH is hours
 *  MM is minutes
 *  SS is secons
 *  IIIIII is microseconds
 *  n is running counter, usually 0
 * An example is "20070916/myrrh173825-414322-0.log", which is written at
 * ca. 17:38 in the afternoon of Sebtember 16th 2007.
 *
 * @param maxSize Maximum size in kilobytes for the file
 * @return A policy object that can be used for writing a log file.
 */
PolicyPtr SizeRestrictedDatedFolderLogs(std::size_t maxSize);
}

}

}

#endif
