/**
 * This file contains the unit test(s) for Repeat
 *
 * $Id: TestRepeat.cpp 358 2007-09-19 16:46:33Z byon $
 */

#include "myrrh/util/Repeat.hpp"
#include "myrrh/util/Preprocessor.hpp"

#define BOOST_TEST_MODULE TestRepeat
#include "boost/test/unit_test.hpp"

static int gFirstRepeatFunctionCalled = 0;
void FirstRepeatFunction( )
{
    ++gFirstRepeatFunctionCalled;
}

static int gSecondRepeatFunctionCalled = 0;
void SecondRepeatFunction( )
{
    ++gSecondRepeatFunctionCalled;
}

template <typename Func>
void TestFuncCall(Func func, int testCount, int &toCompare)
{
    toCompare = 0;
    myrrh::util::Repeat<Func> repeat(func);
    repeat(testCount);
    BOOST_CHECK_EQUAL(toCompare, testCount);
}

class Functor
{
public:

    void operator( )( )
    {
        ++mTestCount;
    }

    static int mTestCount;
};

class Functor2
{
public:

    Functor2( ) :
        mTestCount(0)
    {
    }

    void operator( )( )
    {
        ++mTestCount;
    }

    int GetTestCount( ) const
    {
        return mTestCount;
    }

private:

    int mTestCount;
};

/// @note There's probably some generic pimpl implementation in boost. I've
///       implemented this one here just for practise.
template <typename T>
class PimplFunctor
{
public:

    PimplFunctor(T *functor) :
        mFunctor(functor)
    {
    }

    void operator( )( )
    {
        (*mFunctor)( );
    }

    const T *operator->( ) const
    {
        return mFunctor.get( );
    }

private:

    boost::shared_ptr<T> mFunctor;
};

int Functor::mTestCount = 0;

BOOST_AUTO_TEST_SUITE(TestRepeat)

BOOST_AUTO_TEST_CASE(TestFreeFunctions)
{
    TestFuncCall(FirstRepeatFunction, 0, gFirstRepeatFunctionCalled);
    TestFuncCall(FirstRepeatFunction, 1234, gFirstRepeatFunctionCalled);
    TestFuncCall(FirstRepeatFunction, 100000, gFirstRepeatFunctionCalled);

    TestFuncCall(SecondRepeatFunction, 1000000, gSecondRepeatFunctionCalled);
}

BOOST_AUTO_TEST_CASE(TestFunctor)
{
    TestFuncCall(Functor( ), 2000000, Functor::mTestCount);
}

BOOST_AUTO_TEST_CASE(TestPimplFunctor)
{
    const int PIMPL_FUNCTOR_TEST_COUNT = 3000000;
    PimplFunctor<Functor2> functor(new Functor2);

    myrrh::util::Repeat<PimplFunctor<Functor2> > repeat(functor);
    repeat(PIMPL_FUNCTOR_TEST_COUNT);

    BOOST_CHECK_EQUAL(PIMPL_FUNCTOR_TEST_COUNT, functor->GetTestCount( ));
}

BOOST_AUTO_TEST_SUITE_END( )
