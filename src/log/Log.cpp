// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * The file contains the non-inline implementation of classes Log,
 * Log::IsOutputTarget, Log::WriteException, Log::WriteLine and
 * Log::OutputGuard.
 *
 * $Id: Log.cpp 355 2007-09-17 18:48:35Z byon $
 */

#include "myrrh/log/Log.hpp"
#include <algorithm>
#include <cassert>
#include <ctime>
#include <functional>
#include <sstream>

namespace myrrh
{

namespace log
{

namespace
{

// Local declarations

/**
 * A helper functor class, which is used to write the current line to
 * given stream buffer.
 */
class WriteLine : public std::unary_function<Log::OutputTarget, void>
{
public:

    /**
     * Constructor.
     * @param The line to be written
     * @param verbosity The verbosity level of the line to be written.
     */
    WriteLine(const std::string &line, VerbosityLevel verbosity);

    /**
     * Copy constructor
     * @param orig The original object
     */
    WriteLine(const WriteLine &orig);

    /**
     * Writes the line to given stream buffer
     * @param buffer The buffer into which the line will be written
     * @return void
     */
    result_type operator( )(argument_type buffer);

private:

    /// Assignment disabled
    WriteLine &operator=(const WriteLine &);

    /**
     * Does the actual work of writing the buffer.
     */
    void Write(std::streambuf *buffer);

    /** The line to be written */
    const std::string &line_;
    /** Verbosity level of the line to be written */
    const VerbosityLevel verbosity_;
};

/**
 * A predicate functor for checking is the given output target the one that
 * is searched for. Can be used for in STL algorithms when handling
 * Log::OutputTarget.
 */
class IsOutputTarget : public std::unary_function<Log::OutputTarget, bool>
{
public:

    /**
     * Constructor.
     * @param The line to be written
     */
    IsOutputTarget(const std::ostream &toCompare);

    IsOutputTarget(const IsOutputTarget &orig);

    /**
     * Does the comparison
     * @param buffer The buffer into which the line will be written
     * @return true if comparison succeeds, false otherwise.
     */
    result_type operator( )(argument_type buffer);

private:

    /// Assignment disabled
    IsOutputTarget &operator=(const IsOutputTarget &);

    /** The line to be written */
    const std::ostream &toCompare_;
};

}

// Log class implementations

// Note that we must use the nothrow version of new to allocate memory here,
// because the singleton object could be initialized for the first time when
// writing log entries. There is no-throw guarantee, so we must not throw
// exceptions here either.
// It is assumed (and tested for VS2005) that std::vector default constructor
// does not allocate memory and thus cannot throw exceptions (this refers to
// the construction of targets_ member variable).
Log::Log( ) :
    verbosity_(INFO),
    header_(new (std::nothrow) TimestampHeader)
{
}

Log::OutputGuard Log::AddOutputTarget(std::ostream &target,
                                      VerbosityLevel verbosity)
{
    targets_.push_back(std::make_pair(target.rdbuf( ), verbosity));
    return OutputGuard(target);
}

void Log::RemoveAllOutputTargets( )
{
    targets_.clear( );
}

void Log::SetVerbosity(VerbosityLevel newVerbosity)
{
    verbosity_ = newVerbosity;
}

void Log::SetHeader(HeaderPtr header)
{
    if (!header.get( ))
    {
        header.reset(new TimestampHeader( ));
    }

    header_ = header;
}

void Log::RemoveOutputTarget(std::ostream &target)
{
    IsOutputTarget test(target);
    targets_.erase(
        std::remove_if(targets_.begin( ), targets_.end( ), test),
        targets_.end( ));
}

/// @note This method is not inlined, even though inlining might give some
///       performance boost. The reason is that the implementation uses
///       WriteLine class, which is an implementation detail. If this method
///       would be inlined, the implementation of WriteLine would need to be
///       exposed as well.
///       The previous reason holds even better after the addition of the
///       exception suppression.
void Log::Write(VerbosityLevel verbosity)
{
    try
    {
        std::for_each(targets_.begin( ), targets_.end( ),
                      WriteLine(line_.str( ), verbosity));
    }
    catch (const std::bad_alloc&)
    {
        // No memory to finish the write operation. There is nothing that can
        // be done and we have no-throw guarantee, so we just ignore the
        // situation.
    }
    catch (...)
    {
        assert(false && "Exception here is programming error");
    }
}

// Log::OutputGuard class implementations

Log::OutputGuard::OutputGuard(std::ostream &target) :
    target_(&target)
{
}

Log::OutputGuard::OutputGuard(const Log::OutputGuard &orig) :
    target_(orig.target_)
{
    // Note that we do not follow smart pointer logic here (i.e. call
    // orig.Release), because that way OutputGuard would never then have true
    // ownership of output target as the only way to construct the class
    // outside of Log is through AddOutputTarget, which returns new object by
    // value.
    orig.target_ = 0;
}

Log::OutputGuard &Log::OutputGuard::operator=(const Log::OutputGuard &orig)
{
    target_ = orig.target_;
    orig.target_ = 0;

    return *this;
}

void Log::OutputGuard::Release( )
{
    if (target_)
    {
        target_->flush( );
        Log::Instance( ).RemoveOutputTarget(*target_);
        target_ = 0;
    }
}

Log::OutputGuard::~OutputGuard( )
{
    Release( );
}

// Local implementations

namespace
{

inline WriteLine::WriteLine(const std::string &line,
                            VerbosityLevel verbosity) :
    line_(line),
    verbosity_(verbosity)
{
}

inline WriteLine::WriteLine(const WriteLine &orig) :
    line_(orig.line_),
    verbosity_(orig.verbosity_)
{
}

inline void WriteLine::Write(std::streambuf *buffer)
{
    const std::streamsize SIZE = static_cast<std::streamsize>(line_.size( ));
    if ((buffer->sputn(line_.c_str( ), SIZE) != SIZE) ||
        (buffer->sputc('\n') != '\n') ||
        (buffer->pubsync( ) < 0))
    {
        /// @todo Design some error reporting mechanism. Now the errors are
        ///       silently ignored, because we need this method to have
        ///       no-throw guarantee.
    }
}

inline WriteLine::result_type
WriteLine::operator( )(WriteLine::argument_type target)
{
    if (verbosity_ <= target.second)
    {
        Write(target.first);
    }
}

IsOutputTarget::IsOutputTarget(const std::ostream &toCompare) :
    toCompare_(toCompare)
{
}

IsOutputTarget::IsOutputTarget(const IsOutputTarget &orig) :
    toCompare_(orig.toCompare_)
{
}

inline IsOutputTarget::result_type
IsOutputTarget::operator( )(IsOutputTarget::argument_type target)
{
    return target.first == toCompare_.rdbuf( );
}

}

}

}
