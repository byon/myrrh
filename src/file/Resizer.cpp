// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class myrrh::file::Resizer
 *
 * $Id: Resizer.cpp 362 2007-09-20 17:48:34Z byon $
 */

#include "myrrh/file/Resizer.hpp"

#include "myrrh/file/SafeModify.hpp"
#include <string>
#include <fstream>

namespace myrrh
{
namespace file
{

// Local function declarations

namespace
{

void CopyFromTemporary(const Copy &copier,
                       const boost::filesystem::path &fileName);
template <typename Stream>
void Open(Stream &stream, const boost::filesystem::path &path,
          const std::string &errorString);
template <typename Stream>
std::ios::openmode GetOpenMode( );
template <>
std::ios::openmode GetOpenMode<std::ifstream>( );

}

// Class implementation

Resizer::Resizer(const boost::filesystem::path &file,
                 PositionScannerPtr startScanner,
                 PositionScannerPtr endScanner) :
    file_(file),
    copier_(startScanner, endScanner)
{
}

void Resizer::operator( )( ) const
{
    if (!boost::filesystem::exists(file_))
    {
        throw NoFile(file_);
    }

    // file::SafeModify ensures that the original file is reverted if an
    // exception occurs.
    file::SafeModify tmpFile(file_);

    CopyFromTemporary(copier_, file_);

    tmpFile.Commit( );
}

Resizer::NoFile::NoFile(const boost::filesystem::path &path) :
    std::runtime_error("File '" + path.string( ) + "' does not exist")
{
}

Resizer::CannotOpen::CannotOpen(const boost::filesystem::path &path) :
    std::runtime_error("File '" + path.string( ) + "' cannot be opened")
{
}

// Local function implementations

namespace
{

// Refactor to smaller to avoid the need for comments. Or is the code just
// self-explanatory?
void CopyFromTemporary(const Copy &copier,
                       const boost::filesystem::path &fileName)
{
    // Open the original file for input. Note that at this point the original
    // file is renamed as temporaray file.
    std::ifstream input;
    const std::string TMP_FILE_NAME(file::SafeModify::Name(fileName));
    Open(input, TMP_FILE_NAME, "Failed to open input file for resizing");

    // Open the temporary output file with the original file name
    std::ofstream output;
    Open(output, fileName.string( ), "Failed to open output file for resizing");

    // Copy the original contents to the new file
    copier(input, output);
}

template <typename Stream>
void Open(Stream &stream, const boost::filesystem::path &path,
          const std::string &errorString)
{
    stream.open(path.string( ).c_str( ),
                std::ios::binary | GetOpenMode<Stream>( ));
    if (stream.fail( ))
    {
        throw Resizer::CannotOpen(errorString + ": " + path.string( ));
    }
}

template <typename Stream>
inline std::ios::openmode GetOpenMode( )
{
    return std::ios::out;
}

template <>
inline std::ios::openmode GetOpenMode<std::ifstream>( )
{
    return std::ios::in;
}

}
}
}
