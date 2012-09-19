// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * @file myrrh/util/Stream.hpp
 * @author Marko Raatikainen
 *
 * This file contains the declaration and implementation of a few utility
 * classes and functions that can be used as a basis for user implemented
 * stream manipulators that take arguments. Manipulators that do not take
 * arguments are simple and do not require this kind of utility classes.
 *
 * There are two utility classes ManipulatorOne and ManipulatorTwo. One can
 * implement a manipulator function by implementing a function
 * signature as either of the following options (depending on the count of
 * needed arguments):
 * @code
 * std::ostream &FunctionWithOneArgument(std::ostream &stream,
 *                                       const ArgumentType &argument);
 * std::ostream &FunctionWithTwoArgument(std::ostream &stream,
 *                                       const ArgumentType1 &argument1,
 *                                       const ArgumentType2 &argument2);
 * @endcode
 *
 * It is also possible to implement the manipulator as a functor. Then one can
 * use the same signature for the functor funtion, except that the function
 * must be "consted". Also the functor class must be copy constructible.
 *
 * The use of these self made functions is easiest with the utility function
 * Manipulate, which has two versions (first for one arguments, the second for
 * two arguments). For instance like this:
 * @code
 * std::cout << Manipulate(FunctionWithOneArgument, 1234) << "\n"
 *           << Manipulate(FunctionWithTwoArgument, 1234, "argument2")
 *           << std::endl;
 * @endcode
 *
 * If it is considered too verbose to have to call Manipulate with the actual
 * function pointer as argument, one can implement a helper function that is
 * specific to the manipulator function. All it needs is to create a new
 * instance of ManipulatorOne or ManipulatorTwo class and pass the arguments
 * to it and then return it from the helper function. Example can be seen from
 * the implementations of Manipulate functions. As the actual implementator
 * function is known, there is no need to pass it as an argument.
 *
 * The implementation is based on ideas from web page of Cay S. Horstmann
 * (http://www.horstmann.com/cpp/iostreams.html).
 *
 * $Id: Stream.hpp 355 2007-09-17 18:48:35Z byon $
 *
 */

#ifndef MYRRH_UTIL_STREAM_H_INCLUDED
#define MYRRH_UTIL_STREAM_H_INCLUDED

#include <ostream>

namespace myrrh
{

namespace util
{

// Need to check whether this idea is really usable. Possibly scrap this too
// as unneeded. At least the implementation is not used anywhere
// If still needed, probably might be better to guide to use std::bind instead
// of these classes, which are tied to specific argument count.
// Seems to be used only in Print, which is to be scrapped. Feel free to remove
// unless there is a valid use case for users.

/**
 * This class can be used as a helper class in implementing a user defined
 * output stream manipulators that take one argument (for instance std::setw).
 * For example of usage @see myrrh/util/Stream.hpp documentation at the
 * beginning of the file.
 *
 */
template <typename Function, typename Argument>
class ManipulatorOne
{
public:

    /**
     * Constructor.
     * @param function The actual functor (function pointer or function object)
     *                 that implements the stream manipulation. The functor
     *                 must return std::ostream& and take two parameters
     *                 of types std::ostream& and a user defined template
     *                 parameter. If the functor is implemented by a function
     *                 object, the operator( ) must be declard const.
     * @param argument The argument to be passed to the stream manipulator
     *                 functor.
     */
    ManipulatorOne(Function function, const Argument &argument);

    /**
     * Output stream operator.
     * @note The definition is included here with declaration, because I
     *       could not get the compilation work if it were at end of file.
     * @param stream The stream to put the output into
     * @param manipulator The manipulator that will manipulate the stream
     * @return The output stream to allow chain use of the operator
     */
    friend std::ostream &operator<<(std::ostream &stream,
                                    const ManipulatorOne &manipulator)
    {
        return manipulator(stream);
    }

private:

    /**
     * Calls the manipulator function with the arguments
     * @param stream The stream to put the output into
     * @return The output stream to allow chain use of the operator
     */
    // What's the point of having private operator? Operators are just
    // syntactic sugar to make the class use more readable. Inside the class
    // there is a need to always dereference the this object. The result
    // will not be pretty.
    // Modify to normal method
    std::ostream &operator( )(std::ostream &stream) const;

    /// Assignment operator disabled
    ManipulatorOne &operator=(const ManipulatorOne &);

    /// The functor (function pointer or function object) that does the
    /// actual manipulation
    const Function function_;
    /// Argument to pass to the function_ manipulator
    const Argument &argument_;
};

template <typename Function, typename ArgumentOne, typename ArgumentTwo>
class ManipulatorTwo
{
public:

    /**
     * Constructor.
     * @param function The actual functor (function pointer or function object)
     *                 that implements the stream manipulation. The functor
     *                 must return std::ostream& and take three parameters
     *                 of types std::ostream& and two user defined template
     *                 parameters. If the functor is implemented by a function
     *                 object, the operator( ) must be declard const.
     * @param argumentOne First argument to be passed to the stream manipulator
     *                    functor.
     * @param argumentTwo Second argument to be passed to the stream
     *                    manipulator functor.
     */
    ManipulatorTwo(Function function, const ArgumentOne &argumentOne,
                   const ArgumentTwo &argumentTwo);

    /**
     * Output stream operator.
     * @note The definition is included here with declaration, because I
     *       could not get the compilation work if it were at end of file.
     * @param stream The stream to put the output into
     * @param manipulator The manipulator that will manipulate the stream
     * @return The output stream to allow chain use of the operator
     */
    friend std::ostream &operator<<(std::ostream &stream,
                                    const ManipulatorTwo &manipulator)
    {
        return manipulator(stream);
    }

private:

    /**
     * Calls the manipulator function with the arguments
     * @param stream The stream to put the output into
     * @return The output stream to allow chain use of the operator
     */
    std::ostream &operator( )(std::ostream &stream) const;

    /// Assignment operator disabled
    ManipulatorTwo &operator=(const ManipulatorTwo &);

    /// The functor (function pointer or function object) that does the
    /// actual manipulation
    const Function function_;
    /// First argument to pass to the function_ manipulator
    const ArgumentOne &argumentOne_;
    /// Second argument to pass to the function_ manipulator
    const ArgumentTwo &argumentTwo_;
};

/**
 * Utility function that can be used to passing a manipulator function that
 * takes one argument to the output stream. Usage example:
 * @code
 *   std::cout << Manipulate(FunctionWithOneArgument, 1234) << std::endl;
 * @endcode
 * @param function The actual functor (function pointer or function object)
 *                 that implements the stream manipulation. The functor
 *                 must return std::ostream& and take two parameters
 *                 of types std::ostream& and a user defined template
 *                 parameter. If the functor is implemented by a function
 *                 object, the operator( ) must be declard const.
 * @param argument The argument to be passed to the stream manipulator
 *                 functor.
 * @return A ManipulatorOne object that can be passed to an output stream
 */
template <typename Function, typename Argument>
ManipulatorOne<Function, Argument> Manipulate(Function function,
                                              const Argument &argument);

/**
 * Utility function that can be used to passing a manipulator function that
 * takes two arguments to the output stream. Usage example:
 * @code
 * std::cout << Manipulate(FunctionWithTwoArguments, 1234, "argument2")
 *           << std::endl;
 * @endcode
 * @param function The actual functor (function pointer or function object)
 *                 that implements the stream manipulation. The functor
 *                 must return std::ostream& and take two parameters
 *                 of types std::ostream& and a user defined template
 *                 parameter. If the functor is implemented by a function
 *                 object, the operator( ) must be declard const.
 * @param argumentOne The argument to be passed to the stream manipulator
 *                    functor.
 * @param argumentTwo Second argument to be passed to the stream
 *                    manipulator functor.
 * @return A ManipulatorTwo object that can be passed to an output stream
 */
template <typename Function, typename ArgumentOne, typename ArgumentTwo>
ManipulatorTwo<Function, ArgumentOne, ArgumentTwo>
Manipulate(Function function, const ArgumentOne &argumentOne,
           const ArgumentTwo &argumentTwo);


// Inline implementations


template <typename Function, typename Argument>
inline ManipulatorOne<Function, Argument>::
ManipulatorOne(Function function, const Argument &argument) :
    function_(function),
    argument_(argument)
{
}

template <typename Function, typename Argument>
inline std::ostream &
ManipulatorOne<Function, Argument>::operator( )(std::ostream &stream) const
{
    return function_(stream, argument_);
}

template <typename Function, typename ArgumentOne, typename ArgumentTwo>
inline ManipulatorTwo<Function, ArgumentOne, ArgumentTwo>::
ManipulatorTwo(Function function, const ArgumentOne &argumentOne,
               const ArgumentTwo &argumentTwo) :
    function_(function),
    argumentOne_(argumentOne),
    argumentTwo_(argumentTwo)
{
}

template <typename Function, typename ArgumentOne, typename ArgumentTwo>
inline std::ostream &ManipulatorTwo<Function, ArgumentOne, ArgumentTwo>::
operator( )(std::ostream &stream) const
{
    return function_(stream, argumentOne_, argumentTwo_);
}

template <typename Function, typename Argument>
inline ManipulatorOne<Function, Argument> Manipulate(Function function,
                                              const Argument &argument)
{
    return ManipulatorOne<Function, Argument>(function, argument);
}

template <typename Function, typename ArgumentOne, typename ArgumentTwo>
inline ManipulatorTwo<Function, ArgumentOne, ArgumentTwo>
Manipulate(Function function, const ArgumentOne &argumentOne,
           const ArgumentTwo &argumentTwo)
{
    return ManipulatorTwo<Function, ArgumentOne, ArgumentTwo>(
        function, argumentOne, argumentTwo);
}

}

}

#endif
