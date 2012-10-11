// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the implementation of myrrh::log::policy::PathPart and
 * it's subclasses
 *
 * $Id: PathPart.cpp 338 2007-09-10 17:36:21Z byon $
 */

#include "myrrh/log/policy/PathPart.hpp"
#include "myrrh/log/policy/Restriction.hpp"
#include "myrrh/log/policy/RestrictionStore.hpp"

#include "boost/filesystem/path.hpp"
#include "boost/regex.hpp"

#define DISABLE_ASSIGNMENT_OPERATOR_COULD_NOT_BE_GENERATED
#define DISABLE_TYPE_CONVERSION_LOSS_OF_DATA
#define DISABLE_COPY_CONSTRUCTOR_COULD_NOT_BE_GENERATED
#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#define DISABLE_DEPRECATED_FUNCTION_WARNING
#define DISABLE_UNINITIALIZED_LOCAL_VARIABLE
#include "myrrh/util/Preprocessor.hpp"
#include "boost/date_time/special_defs.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/local_time/local_time_types.hpp"
#include "boost/algorithm/string/replace.hpp"
#pragma warning(pop)

#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace myrrh
{

namespace log
{

namespace policy
{

// Local declarations

namespace
{

/**
 * Constructs a regular expression string that matches timestamps
 */
std::string TimeExpression( );

/**
 * Constructs a regular expression string that matches timestamp fractions
 */
std::string FractionString( );

/**
 * Returns the current process id as string
 */
std::string GetProcessId( );

/**
 * A helper function for creating new PartSum object from a PathPart object
 * @param A PathPart object to be added to the new PartSum object.
 * @returns A new PartSum object containing given PathPart object
 */
template <typename T>
PartSum NewPartSum(const T &part);

}

// PathPart class implementation

std::string PathPart::Generate( )
{
    return DoGenerate( );
}

boost::regex PathPart::GetExpression( ) const
{
    return DoGetExpression( );
}

bool PathPart::IsFirstEarlier(const std::string &left, const std::string &right) const
{
    assert(boost::regex_match(left, GetExpression( )));
    assert(boost::regex_match(right, GetExpression( )));

    return DoIsFirstEarlier(left, right);
}

// PartSum class implementation

PartSum::PartSum( )
{
}

PartSum::PartSum(const std::string &path)
{
    *this += path;
}

void PartSum::Add(PathPartPtr part)
{
    // Sanity checking only in debug builds. Same object should not be added
    // twice.
    assert(std::find(parts_.begin( ), parts_.end( ), part) == parts_.end( ));

    parts_.push_back(part);
}

const PartStore &PartSum::Parts( ) const
{
    return parts_;
}

void PartSum::AddParts(const PartSum &parts)
{
    const PartStore &newParts = parts.Parts( );
    parts_.insert(parts_.end( ), newParts.begin( ), newParts.end( ));
}

// Text class implementation

Text::Text(const std::string &text) :
    text_(text)
{
    if (text.find_first_of("/\\") != std::string::npos)
    {
        throw IllegalText("Folder separators ('/' or '\\') not allowed");
    }
}

Text::Text(const Text &orig) :
    text_(orig.text_)
{
}

std::string Text::DoGenerate( )
{
    return text_;
}

boost::regex Text::DoGetExpression( ) const
{
    std::string copy(text_);
    boost::replace_all(copy, ".", "\\.");
    return boost::regex(copy);
}

void Text::AppendRestrictions(RestrictionStore &)
{
}

bool Text::DoIsFirstEarlier(const std::string &, const std::string &) const
{
    return false;
}

Text::operator PartSum( ) const
{
    return NewPartSum(*this);
}

// Text::IllegalText class implementation

Text::IllegalText::IllegalText(const std::string &what) :
    std::runtime_error(what)
{
}

// Folder class implementation

std::string Folder::DoGenerate( )
{
    return "/";
}

boost::regex Folder::DoGetExpression( ) const
{
    return boost::regex("/");
}

void Folder::AppendRestrictions(RestrictionStore &)
{
}

bool Folder::DoIsFirstEarlier(const std::string &left,
                              const std::string &right) const
{
    return left < right;
}

Folder::operator PartSum( ) const
{
    return NewPartSum(*this);
}

// Date class implementation

std::string Date::DoGenerate( )
{
    using namespace boost::gregorian;
    const date TODAY(day_clock::local_day( ));

    return to_iso_string(TODAY);
}

boost::regex Date::DoGetExpression( ) const
{
    static const std::string DAY("(0[1-9]|[12]\\d|3[01])");
    static const std::string MONTH("(0[1-9]|1[0-2])");
    static const std::string YEAR("\\d{4}");

    return boost::regex(YEAR + MONTH + DAY);
}

void Date::AppendRestrictions(RestrictionStore &store)
{
    RestrictionPtr restriction(new DateRestriction<>);
    store.Add(restriction);
}

bool Date::DoIsFirstEarlier(const std::string &left,
                            const std::string &right) const
{
    return left < right;
}

Date::operator PartSum( ) const
{
    return NewPartSum(*this);
}

class Time::TimePrivate
{
public:

    TimePrivate( ) :
        counter_(0)
    {
    }

    int UniqueIdentifier(const boost::posix_time::time_duration &now)
    {
        if (now == lastCheck_)
        {
            ++counter_;
        }
        else
        {
            counter_ = 0;
        }

        lastCheck_ = now;

        return counter_;
    }

private:

    boost::posix_time::time_duration lastCheck_;
    int counter_;
};

Time::Time( ) :
    private_(new TimePrivate)
{
}

Time::Time(const Time &orig) :
    private_(new TimePrivate(*orig.private_))
{
}

Time::~Time( )
{
}

Time &Time::operator=(const Time &orig)
{
    private_.reset(new TimePrivate(*orig.private_));
    return *this;
}

std::string Time::DoGenerate( )
{
    using namespace boost::posix_time;
    time_duration now(microsec_clock::local_time( ).time_of_day( ));

    static const int DIGITS_IN_HOURS = 2;
    static const int DIGITS_IN_MINUTES = 2;
    static const int DIGITS_IN_SECONDS = 2;

    // The separate parts of timestamp (hours, minutes, seconds or second
    // fractions (milli or nano seconds depending of build environment) are
    // padded with zeroes if they do not fill the entire width of their part.
    // The fractions of second are separated from rest of timestamp by a
    // dash ('-').
    // After the fractions come the unique identifier. It is needed, because
    // the generating may have been some on the same fragment for two files.
    // If that happens, the first file will get overwritten.
    std::ostringstream stream;
    stream << std::setfill('0')
           << std::setw(DIGITS_IN_HOURS)
           << now.hours( )
           << std::setfill('0')
           << std::setw(DIGITS_IN_MINUTES)
           << now.minutes( )
           << std::setfill('0')
           << std::setw(DIGITS_IN_SECONDS)
           << now.seconds( )
           << '-'
           << std::setw(time_duration::num_fractional_digits( ))
           << std::setfill('0')
           << now.fractional_seconds( )
           << '-'
           << private_->UniqueIdentifier(now);

    return stream.str( );
}

boost::regex Time::DoGetExpression( ) const
{
    return boost::regex(TimeExpression( ));
}

void Time::AppendRestrictions(RestrictionStore &)
{
}

bool Time::DoIsFirstEarlier(const std::string &left,
                            const std::string &right) const
{
    return left < right;
}

Time::operator PartSum( ) const
{
    return NewPartSum(*this);
}

// Index class implementation

Index::Index( ) :
    counter_(1)
{
}

std::string Index::DoGenerate( )
{
    const std::string RESULT(boost::lexical_cast<std::string>(counter_));
    ++counter_;
    return RESULT;
}

boost::regex Index::DoGetExpression( ) const
{
    return boost::regex("\\d+");
}

void Index::AppendRestrictions(RestrictionStore &)
{
}

bool Index::DoIsFirstEarlier(const std::string &left,
                             const std::string &right) const
{
    return boost::lexical_cast<int>(left) < boost::lexical_cast<int>(right);
}

Index::operator PartSum( ) const
{
    return NewPartSum(*this);
}

// ProcessId class implementation

ProcessId::ProcessId( ) :
    pid_(GetProcessId( ))
{
}

std::string ProcessId::DoGenerate( )
{
    return pid_;
}

boost::regex ProcessId::DoGetExpression( ) const
{
    return boost::regex(pid_);
}

void ProcessId::AppendRestrictions(RestrictionStore &)
{
}

bool ProcessId::DoIsFirstEarlier(const std::string &, const std::string &) const
{
    return false;
}

ProcessId::operator PartSum( ) const
{
    return NewPartSum(*this);
}

/// Divide smaller
PartSum &operator+=(PartSum &left, const std::string &right)
{
    static const std::string SEPARATORS("/\\");
    std::size_t position = right.find_first_of(SEPARATORS);
    std::size_t previousPosition = 0;

    while (std::string::npos != position)
    {
        if (previousPosition != position)
        {
            const std::size_t SIZE = position - previousPosition;
            left.Add(Text(right.substr(previousPosition, SIZE)));
        }

        left.Add(Folder( ));
        previousPosition = position +1;
        position = right.find_first_of(SEPARATORS, previousPosition);
    }

    if (previousPosition < right.size( ))
    {
        left.Add(Text(right.substr(previousPosition)));
    }

    return left;
}

PartSum operator+(const std::string &left, const PartSum &right)
{
    return PartSum(left) + right;
}

PartSum operator+(const PartSum &left, const std::string &right)
{
    return left + PartSum(right);
}

PartSum operator+(const PartSum &left, const PartSum &right)
{
    PartSum result(left);
    result.AddParts(right);
    return result;
}

// Local function implementations

namespace
{

std::string TimeExpression( )
{
    const std::string HOURS("([01]\\d|2[0-3])");
    const std::string MINUTES("([0-5]\\d)");
    const std::string SECONDS("([0-5]\\d)");
    const std::string SECOND_FRACTIONS("\\d{" + FractionString( ) + "}");
    const std::string UNIQUE_IDENTIFIER("\\d+");
    const char SEPARATOR = '-';

    return HOURS + MINUTES + SECONDS + SEPARATOR + SECOND_FRACTIONS +
           SEPARATOR + UNIQUE_IDENTIFIER;
}

std::string FractionString( )
{
    using namespace boost::posix_time;
    size_t FRACTION_SIZE = time_duration::num_fractional_digits( );
    return boost::lexical_cast<std::string>(FRACTION_SIZE);
}

/// @todo This is now duplicated both into production and test code
std::string GetProcessId( )
{
#ifdef WIN32
    const int PID(_getpid( ));
#else
    const int PID(getpid( ));
#endif

    return boost::lexical_cast<std::string>(PID);
}

template <typename T>
inline PartSum NewPartSum(const T &part)
{
    PartSum sum;
    sum.Add(part);
    return sum;
}

}
}
}
}
