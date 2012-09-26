// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the declaraion and implementation of class
 * ExpressionMatcher and function MatchFiles.
 *
 * $Id: MatchFiles.hpp 356 2007-09-18 19:55:21Z byon $
 */

#ifndef MYRRH_FILE_MATCHFILES_HPP_INCLUDED
#define MYRRH_FILE_MATCHFILES_HPP_INCLUDED

#include "myrrh/util/CopyIf.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/regex.hpp"
#include <vector>

namespace myrrh
{
namespace file
{

typedef std::vector<boost::filesystem::path> PathStore;

/**
 * Goes through the files in the given directory and returns the paths to the
 * files that are matched with Matcher predicate. The Matcher must contain
 * an operator( ), which takes a boost::filesystem::path parameter and returns
 * a boolean value. An example is ExpressionMatcher, which can be used in
 * conjunction to this function to find all the files whose file names match
 * a specific regular expression.
 * @param folder Path to the directory that is used to search the files from.
 * @param matcher A predicate that is used to choose the files
 * @return A storage of the paths of the files chosen from the directory.
 * @throws boost::filesystem::filesystem_error if the path to folder does not
 *         point to an existing directory. Note that the folder defined for the
 *         function call is included in the results.
 */
template <typename Matcher>
PathStore MatchFiles(const boost::filesystem::path &folder, Matcher matcher);

/**
 * A predicate useful to check if a path matches a regular expression.
 */
// Check out what was the benefit of using std::unary_function
// A version that recursively looks for the pattern might also be useful
class ExpressionMatcher :
    public std::unary_function<boost::filesystem::path, bool>
{
public:

    /**
     * Constructor
     * @param expression The regular expression used for matching file name
     */
    ExpressionMatcher(const boost::regex &expression);

    /**
     * Does the matching
     * @param path The path to match with the stored regular expression
     * @return true if the last identifier in the given path matches. For
     *         example expression '^File.*' matches to both ./a/b/c/File/ and
     *         ./a/b/c/File.txt.
     */
    bool operator( )(const boost::filesystem::path &path) const;

private:

    // Not const to make default copying and assignment work
    boost::regex expression_;
};

// Inline implementations

template <typename Matcher>
inline PathStore MatchFiles(const boost::filesystem::path &folder,
                            Matcher matcher)
{
    PathStore result;

    using namespace boost::filesystem;
    util::CopyIf(directory_iterator(folder), directory_iterator( ),
                 std::back_inserter(result), matcher);

    return result;
}

inline ExpressionMatcher::ExpressionMatcher(const boost::regex &expression) :
    expression_(expression)
{
}

inline
bool ExpressionMatcher::operator( )(const boost::filesystem::path &path) const
{
    return boost::regex_match(path.leaf( ).string( ), expression_);
}

}

}

#endif
