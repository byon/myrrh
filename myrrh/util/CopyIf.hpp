// No copyright presented, as the code is copied from implementation by Scott
// Meyers' book Effective C++.

/**
 * @file myrrh/utility/CopyIf.hpp
 * @author Marko Raatikainen
 *
 * This file contains an declaration and implementation of CopyIf function.
 *
 * $Id: CopyIf.hpp 314 2007-05-06 14:33:12Z Byon $
 */

#ifndef MYRRH_UTILITY_COPYIF_H_INCLUDED
#define MYRRH_UTILITY_COPYIF_H_INCLUDED

namespace myrrh
{

namespace util
{

/**
 * STL does dot provide copy_if algorithm. Here's the implementation suggested
 * by Scott Meyers in Effective C++. The function copies all items from one
 * iterator range to given output iterator that fullfill the terms of given
 * predicate.
 * @warning The destination must have enough allocated space to handle the
 *          new objects or the iterator must be able to allocate new space
 *          (like std::inserter).
 * @warning The end iterator must come after the begin iterator in the
 *          destination range.
 * @param begin An iterator that gives access to first object in range
 * @param end An iterator that points to the position after the last to be
 *            copied.
 * @param destinationBegin An iterator that points the position of the
 *                         destination into which the objects will be copied.
 * @param predicate A predicate object, which is used to tell if the current
 *                  object will be copied or not.
 */
template <typename InputIterator,
          typename OutputIterator,
          typename Predicate>
OutputIterator CopyIf(InputIterator begin, InputIterator end,
                      OutputIterator destinationBegin, Predicate predicate);

// Inline implementations

template <typename InputIterator,
          typename OutputIterator,
          typename Predicate>
inline OutputIterator CopyIf(InputIterator begin, InputIterator end,
                             OutputIterator destinationBegin,
                             Predicate predicate)
{
    while (begin != end)
    {
        if (predicate(*begin))
        {
            *destinationBegin++ = *begin;
        }

        ++begin;
    }

    return destinationBegin;
}

}

}

#endif
