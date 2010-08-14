// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains the unit test(s) for RestrictionStore
 *
 * $Id: TestRestrictionStore.cpp 340 2007-09-11 19:51:16Z byon $
 */

#include "myrrh/log/policy/RestrictionStore.hpp"
#include "myrrh/log/policy/Restriction.hpp"
#include "myrrh/log/policy/Opener.hpp"

#define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
#include "myrrh/util/Preprocessor.hpp"

#include "boost/shared_ptr.hpp"
#include "boost/filesystem/path.hpp"
#define BOOST_AUTO_TEST_MAIN
#include "boost/test/auto_unit_test.hpp"

#ifdef WIN32
#pragma warning(pop)
#endif

using namespace myrrh::log::policy;

// Local declarations

namespace
{

template <bool Restricted>
class CodedRestriction : public Restriction
{
public:
    virtual bool IsRestricted(const File &file, std::size_t toWrite) const;
};

class DummyOpener : public Opener
{
    virtual boost::filesystem::path DoOpen(std::filebuf &file, Path& path);
};

}

// Test implementations

BOOST_AUTO_TEST_CASE(DefaultConstruction)
{
    RestrictionStore store;
    BOOST_CHECK_EQUAL(std::size_t(0), store.Count( ));
}

BOOST_AUTO_TEST_CASE(AddingRestriction)
{
    RestrictionStore store;
    RestrictionPtr restriction(new CodedRestriction<false>);
    store.Add(restriction);
    BOOST_CHECK_EQUAL(std::size_t(1), store.Count( ));
}

BOOST_AUTO_TEST_CASE(AddingManyRestrictions)
{
    RestrictionStore store;
    const std::size_t COUNT = 30;

    for (std::size_t i = 0; i < COUNT; ++i)
    {
        RestrictionPtr restriction(new CodedRestriction<false>);
        store.Add(restriction);
    }

    BOOST_CHECK_EQUAL(COUNT, store.Count( ));
}

BOOST_AUTO_TEST_CASE(CheckingIsRestricted)
{
    RestrictionStore store;

    for (std::size_t i = 0; i < 30; ++i)
    {
        RestrictionPtr notRestricted(new CodedRestriction<false>);
        store.Add(notRestricted);
    }

    Path path;

    BOOST_CHECK(!store.IsRestricted(*DummyOpener( ).Open(path), 0));

    RestrictionPtr restricted(new CodedRestriction<true>);
    store.Add(restricted);

    BOOST_CHECK(store.IsRestricted(*DummyOpener( ).Open(path), 0));
}

BOOST_AUTO_TEST_CASE(RestrictionGetsCalled)
{
    static int callTimes = 0;
    struct OwnRestriction : public Restriction
    {

    private:
        virtual bool IsRestricted(const File &, std::size_t) const
        {
            ++callTimes;
            return false;
        }
    };

    RestrictionStore store;

    RestrictionPtr restriction(new OwnRestriction);
    store.Add(restriction);

    Path path;
    BOOST_CHECK_EQUAL(0, callTimes);
    BOOST_CHECK(!store.IsRestricted(*DummyOpener( ).Open(path), 0));
    BOOST_CHECK_EQUAL(1, callTimes);
}

// Local implementations

namespace
{

template <bool Restricted>
bool CodedRestriction<Restricted>::IsRestricted(const File &,
                                                std::size_t) const
{
    return Restricted;
}

boost::filesystem::path DummyOpener::DoOpen(std::filebuf &, Path&)
{
    return "";
}

}
