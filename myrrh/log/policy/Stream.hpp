// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file includes declaration of classes myrrh::log::policy::Buffer and
 * myrrh::log::policy::Stream
 *
 * $Id: Stream.hpp 369 2007-09-20 20:08:51Z byon $
 */

#ifndef MYRRH_LOG_POLICY_STREAM_HPP_INCLUDED
#define MYRRH_LOG_POLICY_STREAM_HPP_INCLUDED

#include "myrrh/util/BufferedStream.hpp"
#include "boost/shared_ptr.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

class Policy;
typedef boost::shared_ptr<Policy> PolicyPtr;

/**
 * This class integrates the myrrh::log::policy component to std::ostream
 * interface. It is not designed to be usable by itself, but through Stream
 * class.
 */
// Move to separate header
class Buffer : public util::BufferedStream
{
public:

    /**
     * Constructor
     * @param policy Contains the policy rules for log writing
     */
    explicit Buffer(PolicyPtr policy);

private:

    /**
     * Implements the actual output.
     * @return 0 If succeeded, otherwise -1.
     */
    virtual int SyncImpl( );

    /// Prevent copying
    Buffer(const Buffer &);
    /// Prevent assignment
    Buffer &operator=(const Buffer &);

    /// Contains the policy rules for log writing
    PolicyPtr policy_;
};

/**
 * This class integrates the myrrh::log::policy component to std::ostream
 * interface. It uses Buffer as buffer, which means that the output is directed
 * to myrrh::log::policy::Policy object.
 *
 * Example of usage (as it is):
 * @code
 *   using namespace myrrh::log::policy;
 *   Stream stream(SizeRestrictedLog( ));
 *   stream << "output with some integers " << 11 << 22  << " in between"
 *          << std::endl;
 * @endcode
 * Example of usage (in co-operation with class myrrh::log::Log):
 * @code
 *   using namespace myrrh::log;
 *   policy::Stream policy::stream(SizeRestrictedLog( ));
 *   Log::OutputGuard guard(Log::Instance( ).AddOutputTarget(*stream));
 *   Info( ) << "output with some integers " << 11 << 22  << " in between";
 * @endcode
 */
class Stream : public std::ostream
{
public:

    /**
     * Constructor.
     * @param policy Contains the policy rules for log writing
     */
    explicit Stream(PolicyPtr policy);

private:

    /// Implements the directing of output to myrrh::log::policy
    Buffer buffer_;

    /// Prevent copying
    Stream(const Stream &);
    /// Prevent assingment
    Stream &operator=(const Stream &);
};

}

}

}

#endif
