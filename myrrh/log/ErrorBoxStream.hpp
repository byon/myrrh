// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration of classes ErrorBoxBuffer and ErrorBoxStream.
 *
 * $Id: ErrorBoxStream.hpp 365 2007-09-20 18:14:42Z byon $
 */

#ifndef MYRHH_LOG_ERRORBOXSTREAM_HPP_INCLUDED
#define MYRHH_LOG_ERRORBOXSTREAM_HPP_INCLUDED

#include "myrrh/util/BufferedStream.hpp"

#ifdef WIN32
#else
#error myrrh::log::ErrorBoxStream is implemented only for Windows
#endif

namespace myrrh
{

namespace log
{

/**
 * This class implements std output stream interface for showing error dialogs.
 * It has perhaps little use on its own. When used in conjunction with
 * ErrorBoxStream as output target for myrrh::log::Log, it can be used for
 * automatic displaying of severe errors to user. It also serves as an example
 * for a class deriving from std::streambuf for myrrh::Log usage.
 * @note The current implementation is only for Windows environment.
 * @note The error dialog will block the execution of the application until
 *       user has clicked the dialog away.
 * @warning If the user decides so, the execution of the application will stop
 *          after the dialog has been clicked.
 */
class ErrorBoxBuffer : public util::BufferedStream
{
public:

    /**
     * Constructor.
     * @param caption A text string that will be shown as caption of the error
     *                dialog.
     */
    ErrorBoxBuffer(const std::string &caption);

private:

    /**
     * Synchronizes the buffer contents with the output target. In our case
     * the contents of the buffer will be written into an error dialog shown
     * to user.
     * @return 0 If succeeded, otherwise -1.
     */
    virtual int SyncImpl( );

    ErrorBoxBuffer(const ErrorBoxBuffer &);
    ErrorBoxBuffer &operator=(const ErrorBoxBuffer &);

    /** Caption for the error dialog */
    const std::string caption_;
};

/**
 * The class implements std output stream interface that uses ErrorBoxBuffer.
 * When the contents of stream is flushed, an error message dialog is shown to
 * user containing the stream.
 * @note The error dialog will block the execution of the application until
 *       user has clicked the dialog away.
 * @warning If the user decides so, the execution of the application will stop
 *          after the dialog has been clicked.
 * @note The class is not really usable in any production use. It serves more
 *       as an example. Because of this it is not developed any further, even
 *       though it could be made more customizable.
 */
class ErrorBoxStream : public std::ostream
{
public:

    /**
     * Constructor
     * @param caption A text string that will be shown as caption of the error
     *                dialog.
     */
    ErrorBoxStream(const std::string &caption);

protected:

    /** The buffer that will hold the error text */
    ErrorBoxBuffer buffer_;
};

}

}

#endif
