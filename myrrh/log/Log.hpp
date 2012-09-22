// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of classes Log, Log::OutputGuard,
 * Log::Verbosity, typedefs Critical, Error, Warn, Notify, Info, Debug and
 * enumeration VerbosityLevel all in namespace myrrh::log.
 *
 * $Id: Log.hpp 355 2007-09-17 18:48:35Z byon $
 *
 */

#ifndef MYRRH_LOG_LOGGER_H_INCLUDED
#define MYRRH_LOG_LOGGER_H_INCLUDED

// Isolate the implementation better
#include "myrrh/log/Header.hpp"
#include "boost/thread/mutex.hpp"
#include <sstream>
#include <vector>

namespace myrrh
{

namespace log
{

// Do not undef on header files, find some other way
#undef ERROR

/**
 * Enum VerbosityLevel defines the numeric levels of verbosity. They resemble
 * the verbosity levels of UNIX syslog facility with the exception of TRACE,
 * which is an additional level, which gets printed only in debug builds.
 */
enum VerbosityLevel
{
    // Why starting from 2?
    CRIT = 2,
    ERROR,
    WARN,
    NOTIFY,
    INFO,
    DEBUG,
    TRACE
};

/**
 * Class Log is a singleton class that is used for centralized output of an
 * application. It does not have public output methods in itself, but has only
 * generic configuration methods in public interface. The actual output is done
 * through nested template class Verbosity, or more specifically by predefined
 * typedefs of Verbosity:
 * <UL>
 *   <LI> myrrh::log::Critical
 *   <LI> myrrh::log::Error
 *   <LI> myrrh::log::Warn
 *   <LI> myrrh::log::Notify
 *   <LI> myrrh::log::Info
 *   <LI> myrrh::log::Debug
 *   <LI> myrrh::log::Trace
 * </UL>
 */
// Singletons are generally speaking a bad practise, find another way
class Log
{
public:

    /**
     * This class is used to make sure that the output stream objects stored
     * inside Log will not get destructed before Log is destroyed. This way
     * it is not possible to use them after destruction.
     *
     * Construction of new OutputGuard objects is made through
     * Log::AddOutputTarget, which adds a new output stream to Log output
     * target. When OutputGuard gets out of scope it automatically removes the
     * correct output stream from Log.
     *
     * OutputGuard has a public copy constructor, which passes the ownership
     * to the new object. The original object has no real reason to be stored
     * and its destruction will not cause changes in output targets.
     */
    // Another option would be to use shared_ptr, with the deletion logic
    // replaced with cleanup
    class OutputGuard
    {
    public:

        /**
         * Copy constructor, passes output target ownership to new object.
         * @param orig The original object to be copied. Once the copy
         *             construction is complete, this object will have no
         *             reason to exist.
         */
        OutputGuard(const OutputGuard &orig);

        /**
         * Assignment operator, passes output target ownership to new object.
         * @param orig The original object to be copied. Once the copy
         *             construction is complete, this object will have no
         *             reason to exist.
         */
        OutputGuard &operator=(const OutputGuard &orig);

        /**
         * Destructor. When OutputGuard gets destructed, Release( ) is called.
         */
        ~OutputGuard( );

        /**
         * Releases the ownership to output stream. After calling of this
         * method the Log no longer writes output to that Output stream. The
         * calling of this method is not mandatory as the method gets
         * automatically called when OutputGuardgets destructed.
         */
        void Release( );

    private:

        /**
         * Constructor, private to allow only Log to have authorization to
         * construct new objects. The given output stream is added to Log's
         * output targets.
         * @param target The output target to be added.
         */
        OutputGuard(std::ostream &target);

        /**
         * Reference to the output target
         */
        mutable std::ostream *target_;

        // Needed so that Log can access constructor
        friend class Log;
    };


    // This declaration needs to be done before declaration of class
    // Verbosity, because gcc will require template parameters otherwise.
    // The friend access is needed by Verbosity, so that it can access the
    // the writing methods of Log.
    friend class Verbosity;

    /**
     * This base class for Verbosity is used so that the TRACE specialization
     * does not have to duplicate VERBOSITY_LIMIT and CHAR_ID static members.
     */
    template <VerbosityLevel Limit, char Id>
    class VerbosityBase
    {
    public:

        /** This constant can be used to access verbosity's numeric
         *  threshold */
        static const VerbosityLevel VERBOSITY_LIMIT = Limit;
        /** This constant can be used to access verbosity's character id */
        static const char CHAR_ID = Id;
    };

    /**
     * This class takes care of logic needed of deducing which output should
     * be written and which not. It also takes care of any needed resource
     * allocation/deallocation (more specifically flushing of buffer and
     * taking care of thread safety) needed and writing of headers to output.
     * Verbosity is not meant to be used directly, instead the predefined
     * typedefs are to be used.
     *
     * Verbosity is used by constructing, using and destructing it on the same
     * (logical) c++ line. For example like this:
     * @code
     *     int i = 36;
     *     myrrh::Info( ) << "Value of integer is " << i;
     * @endcode
     *
     * When Verbosity is constructed, it first writes the header into output.
     *
     * When Verbosity gets destructed, it automatically writes an end of line
     * and flushes the buffer.
     *
     * @note There is a specialization of this class for the TRACE verbosity
     *       level. When that is used, the output is printed only in debug
     *       builds.
     */
    /// This class should be moved to a header of it's own, with isolated
    /// implementation on actual writing. This is the actual interface to
    /// logging, which means the header where it is contained will be
    /// included in many, many places. The header should be fast to build,
    /// and thus the other details here should be in other header files.
    ///
    /// The VerbosityLevel enumeration prevents users from customizing the
    /// levels
    template <VerbosityLevel Limit, char Id>
    class Verbosity : public VerbosityBase<Limit, Id>
    {
    public:

        /**
         * Constructor, writes header and does any resource allocation, like
         * locking mutexes for thread safety. Note that the constructor does
         * nothing, if the current verbosity is too high for the Verbosity
         * class' Limit. So there should be very small performance cost, when
         * nothing needs to be written.
         */
        Verbosity( );

        /**
         * Destructor
         */
        ~Verbosity( );

        /**
         * Input operator for data that needs to be written to the output.
         * Nothing is done however if current verbosity is too high.
         * @param data The data to be written.
         * @return *this, to allow chain use of the operator
         */
        template <typename T>
        Verbosity &operator <<(const T &data);

        /**
         * Input operator for stream manipulators.
         * @param manipulator The manipulator function object.
         * @return *this, to allow chain use of the operator
         */
        Verbosity & operator<<(std::ios_base& (manipulator)(std::ios_base&));

    private:

        /**
         * Creates lock object. This has been implemented in a separate
         * function, because the usage is too long to be readable in the
         * constructor initializer list. Note that the method may return 0,
         * if there is not enough memory to create the lock object. This is ok,
         * we'll just not be able to write anything.
         */
        static boost::mutex::scoped_lock *GetLock( );

        /** Lock for guarding concurrent writing. It also is used to check
         *  if the current verbosity level allows us to write. If lock_ is
         *  zero, we are not allowed to write. */
        boost::mutex::scoped_lock *lock_;
    };

#ifdef NDEBUG

    /**
     * This specialization of the Verbosity class is to be compiled only for
     * release builds. The purpose is to have the lowest verbosity level to be
     * printed out only in debug builds. The implementation of this
     * specialization is empty and the use should be optimized so that there
     * is no performance cost.
     */
    // I no longer think this is a good idea. I will want to have the
    // ability to trace also on release builds.
    template <>
    class Verbosity<TRACE, 'T'> : public VerbosityBase<TRACE, 'T'>
    {
    public:

        /**
         * Input operator, which does nothing. Should get optimized to no-op.
         * @param data Not used in this specialization
         * @note The definition is included here with declaration, because I
         *       could not get the compilation work if it were at end of file.
         */
        template <typename T>
        Verbosity &operator <<(const T &/*data*/)
        {
            return *this;
        }

        typedef std::ios_base& (*Manipulator)(std::ios_base&);

        Verbosity & operator<<(Manipulator)
        {
            return *this;
        }
    };

#endif

    /**
     * Returns the singleton instance of Log
     * @return Reference to the one and only instance of Log
     * @warning First usage is not thread-safe!
     */
    static Log &Instance( );

    /**
     * Adds a new Output stream for the Log's Output targets.
     * @note The caller must test the usability of the output target stream,
     *       before it is passed into this method. The log writing is
     *       implemented with a no-throw guarantee, so that it can be done
     *       from destructors. Because of this there is no way to report errors
     *       that occur during writing.
     * @param target The output stream to be added. Note that the writing to
     *               the stream must ensure a no-throw guarantee.
     * @param verbosity An optional verbosity level that can tighten the
     *                  amount of output for this output target. The setting
     *                  of this parameter has effect only if the global
     *                  verbosity level is looser than value set here.
     *                  By default the value is set so that there is no
     *                  additional verbosity restriction.
     * @return A new OutputGuard object. When the object gets destructed the
     *         output stream is removed from Log's output targets.
     * @warning Not thread-safe!
     */
    OutputGuard AddOutputTarget(std::ostream &target,
                                VerbosityLevel verbosity = TRACE);

    /**
     * Removes all of the output targets from log.
     * @warning Not thread-safe!
     */
    void RemoveAllOutputTargets( );

    /**
     * Sets the verbosity level for Log. No output will be written unless it
     * is lower or equal than this level. For example, if the verbosity is set
     * to NOTIFY, then output with INFO, DEBUG or TRACE levels will not be
     * written. All other output is written.
     */
    void SetVerbosity(VerbosityLevel newVerbosity);

    /**
     * Returns the current global verbosity level
     * @return The current global verbosity level
     */
    VerbosityLevel GetVerbosity( ) const;

    /**
     * Checks is the given verbosity level writable when compared to current
     * global verbosity level.
     * @param verbosity The verbosity level to check for
     * @return true if verbosity level is writeble, false otherwise.
     */
    bool IsWritable(VerbosityLevel verbosity) const;

    /**
     * Sets a new new line header writer. The given object is responsible for
     * writing the line headers (a bit of string attached to start of each
     * line). There exists a default implementation (TimestampHeader),
     * which inserts a timestamp and character id of the used verbosity level.
     * If user wishes to revert back to default header, he can call this
     * method without giving parameters.
     * @param header A new header object contained in std::auto_ptr. As decreed
     *               by std::auto_ptr policy the ownership passes to Log.
     *               If no header is given, the default header implementation
     *               is used.
     * @warning Not thread-safe!
     */
    void SetHeader(HeaderPtr header = HeaderPtr( ));

    typedef std::pair<std::streambuf *, VerbosityLevel> OutputTarget;
    typedef std::vector<OutputTarget> OutputTargets;

private:

    /**
     * Constructor, declared private for Singleton pattern use.
     */
    Log( );

    /**
     * Removes an output target from Log's output targets.
     * @param target The stream to be removed as target.
     * @warning Not thread-safe!
     */
    void RemoveOutputTarget(std::ostream &target);

    /**
     * Writes the header of the log entry.
     * @param id A character identifier of the verbosity level
     * @warning Not thread safe, uses CRT's localtime( )
     */
    void WriteHeader(char id);

    /**
     * Writes the given data to the output targets of Log.
     */
    void Write(VerbosityLevel verbosity);

    Log(const Log &);
    const Log &operator=(const Log &);

    /** Storage of output targets */
    OutputTargets targets_;
    /** Current verbosity level */
    // Why is this volatile? Volatile is not sufficient to guarantee
    // thread-safety, if that is the purpose.
    volatile VerbosityLevel verbosity_;
    /** Line that is currently being written containing the header */
    std::ostringstream line_;
    /** Mutex that guards concurrent writing access */
    // The mutex is now global to all. Wouldn't it be better, if it was
    // specific to one output target?
    boost::mutex mutex_;
    /** Knows how to write the header of each line */
    HeaderPtr header_;
};

// Type definitions for uniform verbosity usage. The user should use these,
// not the Verbosity class directly.
typedef log::Log::Verbosity<CRIT, 'C'> Critical;
typedef log::Log::Verbosity<ERROR, 'E'> Error;
typedef log::Log::Verbosity<WARN, 'W'> Warn;
typedef log::Log::Verbosity<NOTIFY, 'N'> Notify;
typedef log::Log::Verbosity<INFO, 'I'> Info;
typedef log::Log::Verbosity<DEBUG, 'D'> Debug;
typedef log::Log::Verbosity<TRACE, 'T'> Trace;

// Inline implementations
// Move to cpp and isolate better

inline Log &Log::Instance( )
{
    static Log logInstance;
    return logInstance;
}

inline VerbosityLevel Log::GetVerbosity( ) const
{
    return verbosity_;
}

inline bool Log::IsWritable(VerbosityLevel verbosity) const
{
    return (verbosity_ >= verbosity);
}

inline void Log::WriteHeader(char id)
{
    line_.str("");
    // In very rare situations it might be that there was not enough memory
    // to allocate the default header object.
    if (header_.get( ))
    {
        header_->Write(line_, id);
    }
}

// The lock is now reserved for the duration of the verbosity object. If
// locking really is needed, it would be better to have it only around the
// actual writing.
template <VerbosityLevel Limit, char Id>
inline Log::Verbosity<Limit, Id>::Verbosity( ) :
    lock_(GetLock( ))
{
    if (lock_)
    {
        Log::Instance( ).WriteHeader(Id);
    }
}

template <VerbosityLevel Limit, char Id>
inline Log::Verbosity<Limit, Id>::~Verbosity( )
{
    if (lock_)
    {
        Log::Instance( ).Write(Limit);
        delete lock_;
    }
}

template <VerbosityLevel Limit, char Id>
    template <typename T>
inline Log::Verbosity<Limit, Id> &
Log::Verbosity<Limit, Id>::operator <<(const T &data)
{
    if (lock_)
    {
        Log::Instance( ).line_ << data;
    }

    return *this;
}

template <VerbosityLevel Limit, char Id>
inline Log::Verbosity<Limit, Id> &
Log::Verbosity<Limit, Id>::operator<<(
    std::ios_base& (manipulator)(std::ios_base&))
{
    if (lock_)
    {
        // I no longer understand what happens here, but looks too complex
        // with all the casts to be safe.
        (*manipulator)(*(std::ios_base *)&(Log::Instance( ).line_));
    }
    return (*this);
}

template <VerbosityLevel Limit, char Id>
inline boost::mutex::scoped_lock *Log::Verbosity<Limit, Id>::GetLock( )
{
    if (Log::Instance( ).IsWritable(Limit))
    {
        // Using the nothrow version of new, because we have a nothrow
        // guarantee in the Verbosity constructor, where this method is called.
        typedef boost::mutex::scoped_lock Lock;
        return new (std::nothrow) Lock(Log::Instance( ).mutex_);
    }

    return 0;
}

}

}

#endif
