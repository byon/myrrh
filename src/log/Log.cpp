// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "myrrh/log/Log.hpp"
#include <algorithm>
#include <cassert>
#include <functional>

namespace myrrh
{

namespace log
{

namespace
{

// Local declarations

void WriteToTargets(const std::string &line, VerbosityLevel verbosity,
                    Log::OutputTargets &targets);
void WriteLine(const std::string &line, VerbosityLevel verbosity,
               Log::OutputTarget &target);
void WriteLine(const std::string &line, std::streambuf& buffer);

}

// Log class implementations

// Note that we must use the nothrow version of new to allocate memory here,
// because the singleton object could be initialized for the first time when
// writing log entries. There is no-throw guarantee, so we must not throw
// exceptions here either.
// It is assumed (and tested for VS2005) that std::vector default constructor
// does not allocate memory and thus cannot throw exceptions (this refers to
// the construction of targets_ member variable).

/// @todo It would be better to separate the initialization from the actual
///       logging. Then the initialization could report errors with exceptions.
Log::Log( ) :
    verbosity_(INFO),
    header_(new (std::nothrow) TimestampHeader)
{
}

Log &Log::Instance( )
{
    static Log logInstance;
    return logInstance;
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

VerbosityLevel Log::GetVerbosity( ) const
{
    return verbosity_;
}

bool Log::IsWritable(VerbosityLevel verbosity) const
{
    return (verbosity_ >= verbosity);
}

void Log::SetHeader(HeaderPtr header)
{
    if (!header.get( ))
    {
        header.reset(new TimestampHeader( ));
    }

    header_ = header;
}

void Log::WriteHeader(char id)
{
    line_.str("");
    // In very rare situations it might be that there was not enough memory
    // to allocate the default header object.
    if (header_.get( ))
    {
        header_->Write(line_, id);
    }
}

void Log::RemoveOutputTarget(std::ostream &toRemove)
{
    auto finder = [&](const OutputTarget& t)
        { return toRemove.rdbuf( ) == t.first; };
    auto first = std::remove_if(targets_.begin( ), targets_.end( ), finder);
    targets_.erase(first, targets_.end( ));
}

void Log::Write(VerbosityLevel verbosity)
{
    try
    {
        WriteToTargets(line_.str( ), verbosity, targets_);
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

void WriteToTargets(const std::string &line, VerbosityLevel verbosity,
                    Log::OutputTargets &targets)
{
    auto writer = [&](Log::OutputTarget& t){ WriteLine(line, verbosity, t); };
    std::for_each(targets.begin( ), targets.end( ), writer);
}

void WriteLine(const std::string &line, VerbosityLevel verbosity,
               Log::OutputTarget& target)
{
    if (verbosity <= target.second)
    {
        WriteLine(line, *target.first);
    }
}

void WriteLine(const std::string &line, std::streambuf& buffer)
{
    const std::streamsize SIZE = static_cast<std::streamsize>(line.size( ));
    if ((buffer.sputn(line.c_str( ), SIZE) != SIZE) ||
        (buffer.sputc('\n') != '\n') ||
        (buffer.pubsync( ) < 0))
    {
        /// @todo Design some error reporting mechanism. Now the errors are
        ///       silently ignored, because we need this method to have
        ///       no-throw guarantee.
    }
}

}

}

}
