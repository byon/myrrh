// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * The file contains non-inline implementation of classes ErrorBoxBuffer and
 * ErrorBoxStream.
 *
 * $Id: ErrorBoxStream.cpp 365 2007-09-20 18:14:42Z byon $
 */

#include "myrrh/log/ErrorBoxStream.hpp"
#include <windows.h>

namespace myrrh
{

namespace log
{

ErrorBoxBuffer::ErrorBoxBuffer(const std::string &caption) :
    caption_(caption)
{
}

int ErrorBoxBuffer::SyncImpl( )
{
    std::string errorText(GetBuffer( ));
    errorText += "\n Choose Ok to continue regardless of error, Cancel to "
                 "exit application";

    int userResponse =
        MessageBox(0, // This message box has no parent
                   errorText.c_str( ), // The actual message
                   caption_.c_str( ), // Caption to the message box
                   MB_ICONERROR | // A stop sign appears with dialog
                   MB_OKCANCEL); // The dialog contains ok and cancel buttons

    switch (userResponse)
    {
    case IDOK:
        // Nothing to do, try to continue application run
        break;
    case IDCANCEL:
        exit(EXIT_FAILURE);
        break;
    default:
        return -1;
    }

    return 0;
}

ErrorBoxStream::ErrorBoxStream(const std::string &caption) :
    std::ostream(0),
    buffer_(caption)
{
    rdbuf(&buffer_);
}

}

}
