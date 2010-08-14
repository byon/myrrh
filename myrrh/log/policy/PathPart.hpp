// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the declaration of myrrh::log::policy::PathPart and it's
 * subclasses
 *
 * $Id: PathPart.hpp 355 2007-09-17 18:48:35Z byon $
 */

#ifndef MYRRH_LOG_POLICY_PATHPART_H_INCLUDED
#define MYRRH_LOG_POLICY_PATHPART_H_INCLUDED

#include "myrrh/log/policy/RestrictionAppender.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/regex.hpp"
#include "boost/shared_ptr.hpp"
#include <string>
#include <vector>

namespace myrrh
{

namespace log
{

namespace policy
{

// Forward declarations
class PartSum;

/**
 * PathPart is what it claims to be, a part of path. By adding several
 * PathPart objects together, one can compile a Path object. Each PathPart
 * object stores the knowledge how to generate a specific part of a new file
 * path and how to match existing file paths to it. This knowledge is
 * implemented into subclasses.
 *
 * In isolation PathPart classes do not really make that much sense. They are
 * intended to by used in conjunction with myrrh::log::policy::Path objects.
 *
 * @note No virtual destructor needed, because there is already a parent class
 *       that has one.
 *
 * @note Uses NVI interface
 *
 * @note Much of the implementation of PathPart and related classes are small
 *       enough to be inlined. However as the intended use is for creating
 *       paths for log files, the performance is not likely to be an issue.
 *       Log files should be created only quite rarely and the performance of
 *       I/O operation will probably have more effect any way.
 */
class PathPart : public RestrictionAppender
{
public:

    /**
     * Generates a new string that could be used as a part of a path.
     */
    std::string Generate( );

    /**
     * Returns a regular expression that can be used match a part of existing
     * file path to the rules of this particular path part.
     */
    boost::regex GetExpression( ) const;

    /**
     * Compares two boost::filesystem::path objects into order of preference
     * according to the path part rules.
     * @param left The object on the left side of comparison
     * @param right The object on the right side of comparison
     * @returns true if the left object is earlier, false otherwise
     */
    bool IsFirstEarlier(const std::string &left,
                        const std::string &right) const;

private:

    /**
     * Implements the actual path part string generation
     */
    virtual std::string DoGenerate( ) = 0;

    /**
     * Implements the actual regular string generation
     */
    virtual boost::regex DoGetExpression( ) const = 0;

    /**
     * Implements the actual path part comparison
     */
    virtual bool DoIsFirstEarlier(const std::string &left,
                                  const std::string &right) const = 0;
};

typedef boost::shared_ptr<PathPart> PathPartPtr;
typedef std::vector<PathPartPtr> PartStore;

/**
 * A helper class intended to store the PathPart objects that are gathered
 * during the part incrementing. For instance, when the following code is
 * executed:
 * @code
 *   Path path;
 *   path += "folder/" + Index( ) + TimeStamp;
 * @endcode
 * ...the execution order is so that we need to first gather the new Text,
 * Index and TimeStamp objects into one, before we can pass them to Path.
 *
 * @note As seen from previous example, PartSum is not intended to be used
 *       straight by the user. Instead it's use should happen implicitly by the
 *       compiler, when new path parts are added to Part object.
 */
class PartSum
{
public:

    /**
     * Default constructor
     */
    PartSum( );

    /**
     * Constructor
     * @param path The path from which the PartSum will be generated from
     */
    PartSum(const std::string &path);

    /**
     * Adds a new path part to the sum.
     */
    void Add(PathPartPtr part);

    /**
     * Adds a new path part to the sum.
     */
    template <typename T>
    void Add(const T &part);

    /**
     * Adds new path parts to the sum.
     */
    void AddParts(const PartSum &parts);

    /**
     * Returns the stored path parts in one container
     */
    const PartStore &Parts( ) const;

private:

    /// Stores the path parts
    PartStore parts_;
};

/**
 * This PathPart subclass can be used to create parts of path that have hard
 * coded textual (in broad sense, numbers are are accepted as well, the hard
 * coding is the real key here) representation. If path separators ('/' or '\'
 * are used, the created object is actually converted to several subobjects:
 * one Folder object for each path separator and one Text object for each part
 * that has no separators. For instance, string "folder/subFolder/file.txt"
 * will be converted to three Text objects ("folder", "subFolder" and
 * "file.txt" and two Folder objects. The conversion is done implicitly without
 * user action.
 * @note Not intended for explicit use. Declared here to be testable. Use
 * instead string literals (or std::strings) that are passed to Path object.
 * Explicit usage is of course possible, but requires more typing. Also the
 * implicit conversion to several Text/Folder objects does not occur.
 *
 * Example use:
 * @code
 *   Path path;
 *   // Implicitly creates the Text and Folder objects into path
 *   path += "folder/subFolder/file.txt";
 * @endcode
 * Previous example with explicit object construction:
 * @code
 *   Path path;
 *   path += Text("folder") + Folder( ) + Text("subFolder") + Folder +
 *           Text("file.txt");
 * @endcode
 */
class Text : public PathPart
{
public:

    /**
     * Constructor
     * @param text The text from which the
     * @throws Text::IllegalText, if given parameter contains folder separators
     */
    explicit Text(const std::string &text);

    /**
     * Copy constructor
     * @param orig The original Text object
     */
    Text(const Text &orig);

    /**
     * Text objects have no restriction, so does nothing.
     * @param store The store for the possible restrictions 
     */
    void AppendRestrictions(RestrictionStore &store);

    /**
     * Allows implicit conversion to PartSum. Part of the mechanism that allows
     * the adding of path parts in same statement.
     * @return New PartSum object that contains this copy of this object.
     */
    operator PartSum( ) const;

    /**
     * Exception class, which is thrown if illegal text is given to constructor
     */
    class IllegalText : public std::runtime_error
    {
    public:
        /**
         * Constructor
         * @param what Describes the error
         */
        IllegalText(const std::string &what);
    };

private:

    /**
     * Implements the actual path part string generation
     */
    virtual std::string DoGenerate( );

    /**
     * Implements the actual regular string generation
     */
    virtual boost::regex DoGetExpression( ) const;

    /**
     * Implements the actual path part comparison
     */
    virtual bool DoIsFirstEarlier(const std::string &left,
                                  const std::string &right) const;

    // Left non-const so that class is automatically assignable
    std::string text_;
};

/**
 * This PathPart subclass can be used to create folder separators into paths.
 * The use is easier through string literals or std::string objects. See 
 * documentation of Text for more explanation and examples of use.
 * @note Not intended for explicit use. Declared here to be testable. Use
 * instead string literals (or std::strings) that are passed to Path object.
 * Explicit usage is of course possible, but requires more typing.
 */
class Folder : public PathPart
{
public:

    /**
     * Folder objects have no restriction, so does nothing.
     * @param store The store for the possible restrictions 
     */
    void AppendRestrictions(RestrictionStore &store);

    /**
     * Allows implicit conversion to PartSum. Part of the mechanism that allows
     * the adding of path parts in same statement.
     * @return New PartSum object that contains this copy of this object.
     */
    operator PartSum( ) const;

private:

    /**
     * Implements the actual path part string generation
     */
    virtual std::string DoGenerate( );

    /**
     * Implements the actual regular string generation
     */
    virtual boost::regex DoGetExpression( ) const;

    /**
     * Implements the actual path part comparison
     */
    virtual bool DoIsFirstEarlier(const std::string &left,
                                  const std::string &right) const;
};

/**
 * This PathPart subclass can be used to create parts of path that have the
 * current date in them. The format used in in the result is "YYYYMMDD", where
 * YYYY is the year, MM the month and DD the day.
 *
 * Example (creates file name with date as base and ".txt" as postfix):
 * @code
 *   Path path;
 *   path += Date( ) + ".log";
 * @endcode
 * Example (folder with date and hard coded file name):
 * @code
 *   Path path;
 *   path += Date( ) + "/file.log";
 * @endcode
 */
class Date : public PathPart
{
public:

    /**
     * Date objects have a restriction. When the date changes, the file path
     * should also be changed. Therefore a new DateRestriction object is
     * appended to given store.
     * @param store The store for the possible restrictions 
     */
    void AppendRestrictions(RestrictionStore &store);

    /**
     * Allows implicit conversion to PartSum. Part of the mechanism that allows
     * the adding of path parts in same statement.
     * @return New PartSum object that contains this copy of this object.
     */
    operator PartSum( ) const;

private:

    /**
     * Implements the actual path part string generation
     */
    virtual std::string DoGenerate( );

    /**
     * Implements the actual regular string generation
     */
    virtual boost::regex DoGetExpression( ) const;

    /**
     * Implements the actual path part comparison
     */
    virtual bool DoIsFirstEarlier(const std::string &left,
                                  const std::string &right) const;
};

/**
 * This PathPart subclass can be used to create parts of path that have the
 * time of creation in them. The format used in in the result is
 * "HHMMSS-sss-iii", in which HH is the hour, MM the minutes, SS the seconds,
 * sss the fractions of seconds and iii a unique identifier.
 * @note The granularity of the fractions of seconds depends on the used
 *       platform. They can be milliseconds or nanoseconds.
 * @note The identifier is a running number. So it is not restricted to be of
 *       any specific size. Large numbers are quite unlikely.
 *
 * Example (creates file name with timestamp as base and ".txt" as postfix):
 * @code
 *   Path path;
 *   path += Time( ) + ".log";
 * @endcode
 *
 * Example (folder with date and timestamp base file name):
 * @code
 *   Path path;
 *   path += Date( ) + "/" + Time( ) + ".log";
 * @endcode
 */
class Time : public PathPart
{
public:

    /**
     * Default constructor.
     */
    Time( );

    /**
     * Copy constructor
     * @param orig The original Time object
     */
    Time(const Time &orig);

    /**
     * Destructor. Needed so that auto_ptr member can be destructed.
     */
    ~Time( );

    /**
     * Assignment operator
     */
    Time &operator=(const Time &orig);

    /**
     * Time objects have no restriction, so does nothing.
     * @param store The store for the possible restrictions 
     */
    void AppendRestrictions(RestrictionStore &store);

    /**
     * Allows implicit conversion to PartSum. Part of the mechanism that allows
     * the adding of path parts in same statement.
     * @return New PartSum object that contains this copy of this object.
     */
    operator PartSum( ) const;

private:

    /**
     * Implements the actual path part string generation
     */
    virtual std::string DoGenerate( );

    /**
     * Implements the actual regular string generation
     */
    virtual boost::regex DoGetExpression( ) const;

    /**
     * Implements the actual path part comparison
     */
    virtual bool DoIsFirstEarlier(const std::string &left,
                                  const std::string &right) const;

    /// Private implementation class forward declaration
    class TimePrivate;
    /// Storage of private implementation
    std::auto_ptr<TimePrivate> private_;
};

/**
 * This PathPart subclass can be used to create parts of path that have a
 * running number in them. For instance the user might want to have a maximum
 * size for log files and start a  new file once the maximum size is exceeded.
 * The files are identified by a increasing counter, like "file1.log",
 * "file2.log", "file3.log", etc. The counter starts from 1.
 *
 * Example (creates file name with index in base and ".txt" as postfix):
 * @code
 *   Path path;
 *   path += "file" + Index( ) + ".log";
 * @endcode
 */
class Index : public PathPart
{
public:

    /**
     * Default constructor
     */
    Index( );

    /**
     * Index objects have no restriction, so does nothing.
     * @param store The store for the possible restrictions 
     */
    void AppendRestrictions(RestrictionStore &store);

    /**
     * Allows implicit conversion to PartSum. Part of the mechanism that allows
     * the adding of path parts in same statement.
     * @return New PartSum object that contains this copy of this object.
     */
    operator PartSum( ) const;

private:

    /**
     * Implements the actual path part string generation
     */
    virtual std::string DoGenerate( );
    /**
     * Implements the actual regular string generation
     */
    virtual boost::regex DoGetExpression( ) const;

    /**
     * Implements the actual path part comparison
     */
    virtual bool DoIsFirstEarlier(const std::string &left,
                                  const std::string &right) const;

    /// The counter of current index
    size_t counter_;
};

/**
 * This PathPart subclass can be used to create parts of path that have the id
 * of the process in them. This can be useful, if the user wants to explicitly
 * see what files were written by which execution of the application.
 *
 * Example (creates path with folder named by process id and indexed file
 * name):
 * @code
 *   Path path;
 *   path += ProcessId( ) + "/file" + Index( ) + ".log";
 * @endcode
 */
class ProcessId : public PathPart
{
public:

    /**
     * Constructor
     */
    ProcessId( );

    /**
     * ProcessId objects have no restriction, so does nothing.
     * @param store The store for the possible restrictions 
     */
    void AppendRestrictions(RestrictionStore &store);

    /**
     * Allows implicit conversion to PartSum. Part of the mechanism that allows
     * the adding of path parts in same statement.
     * @return New PartSum object that contains this copy of this object.
     */
    operator PartSum( ) const;

private:

    /**
     * Implements the actual path part string generation
     */
    virtual std::string DoGenerate( );

    /**
     * Implements the actual regular string generation
     * @note The current process id is returned as regular expression. The
     *       point is to match only if the process is the same as in previous
     *       writing. It makes no sense to return less strict options.
     */
    virtual boost::regex DoGetExpression( ) const;

    /**
     * Implements the actual path part comparison
     */
    virtual bool DoIsFirstEarlier(const std::string &left,
                                  const std::string &right) const;

    /// The current process id. Left non-const so that class is automatically
    /// assignable
    std::string pid_;
};

/**
 * Operator for adding a std::string and a PartSum object together. The result
 * will have created one or more Text and/or Folder objects (see documentation
 * of Text) from the std::string to the beginning and added the PartSum objects
 * to the end.
 * @param left The std::string object on the left side of operation
 * @param right The PartSum object on the right side of operation
 * @returns A new PartSum object containing the result
 */
PartSum operator+(const std::string &left, const PartSum &right);

/**
 * Operator for adding a PartSum and a std::string object together. The result
 * will have created one or more Text and/or Folder objects (see documentation
 * of Text) from the std::string to the end and added the PartSum objects
 * to the before them.
 * @param left The PartSum object on the left side of operation
 * @param right The std::string object on the right side of operation
 * @returns A new PartSum object containing the result
 */
PartSum operator+(const PartSum &left, const std::string &right);

/**
 * Operator for adding two PartSum objects together. The result has combined
 * the path part objects together so that the objects from the left side
 * PartSum will be in the beginning.
 * @param left The PartSum object on the left side of operation
 * @param right The PartSum object on the right side of operation
 * @returns A new PartSum object containing the result
 */
PartSum operator+(const PartSum &left, const PartSum &right);

/**
 * Operator for adding new Text and/or Folder objects (see documentation of
 * Text) to an existing PartSum object. The new objects will be added to the
 * end of possibly existing path part objects stored in the PartSum object.
 * @param left The PartSum object that is target for the addition
 * @param right The std::string from which the new PathPart objects are created
 * @returns Reference to the original PartSum object
 */
PartSum &operator+=(PartSum &left, const std::string &right);

// Inline implementations

template <typename T>
inline void PartSum::Add(const T &part)
{
    PathPartPtr partPtr(new T(part));
    Add(partPtr);
}

}

}

}

#endif
