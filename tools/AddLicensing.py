#! /usr/bin/env python
#
# @todo documentation
#
# $Id$

import getopt
import sys
import doctest
import os.path
import dircache
import re

def listFiles(path):
    """ Lists all the files in the given directory and all the subdirectories.
        The files are returned in a list.
    >>> testDir = 'testDir/'
    >>> os.mkdir(testDir)
    >>> print listFiles(testDir)
    []
    >>> name1 = testDir + 'file1'
    >>> file1 = open(name1, 'w')
    >>> print listFiles(testDir)
    ['testDir\\\\file1']
    >>> name2 = testDir + 'file2'
    >>> file2 = open(name2, 'w')
    >>> print listFiles(testDir)
    ['testDir\\\\file1', 'testDir\\\\file2']
    >>> sub1 = testDir + 'sub1/'
    >>> os.mkdir(sub1)
    >>> print listFiles(testDir)
    ['testDir\\\\file1', 'testDir\\\\file2']
    >>> name3 = sub1 + 'file1'
    >>> file3 = open(name3, 'w')
    >>> print listFiles(testDir)
    ['testDir\\\\file1', 'testDir\\\\file2', 'testDir\\\\sub1\\\\file1']
    >>> sub2 = sub1 + 'sub2/'
    >>> os.mkdir(sub2)
    >>> sub3 = sub2 + 'sub3/'
    >>> os.mkdir(sub3)
    >>> print listFiles(testDir)
    ['testDir\\\\file1', 'testDir\\\\file2', 'testDir\\\\sub1\\\\file1']
    >>> name4 = sub3 + 'file4'
    >>> file4 = open(name4, 'w')
    >>> print listFiles(testDir)
    ['testDir\\\\file1', 'testDir\\\\file2', 'testDir\\\\sub1\\\\file1', 'testDir\\\\sub1\\\\sub2\\\\sub3\\\\file4']
    >>> file1.close( )
    >>> os.remove(name1)
    >>> file2.close( )
    >>> os.remove(name2)
    >>> file3.close( )
    >>> os.remove(name3)
    >>> file4.close( )
    >>> os.remove(name4)
    >>> os.rmdir(sub3)
    >>> os.rmdir(sub2)
    >>> os.rmdir(sub1)
    >>> os.rmdir(testDir)
    """
    checkDirectory(path)
    result = []
    for root, dirs, files in os.walk(path):
        for file in files:
            result += [os.path.normpath(os.path.join(root, file))]
    return result

def selectFiles(paths, expression):
    """ Selects the files from given list that mathes the given expression.
        The result is returned in a new list.
    >>> print selectFiles([], 'some')
    []
    >>> print selectFiles(['a', 'b', 'c'], 'some')
    []
    >>> print selectFiles(['a', 'b', 'c'], 'b')
    ['b']
    >>> print selectFiles(['a', 'b', 'c'], '.')
    ['a', 'b', 'c']
    >>> print selectFiles(['a/b', 'b', 'c'], 'a')
    []
    >>> print selectFiles(['a/b', 'b', 'c'], 'b')
    ['a/b', 'b']
    >>> print selectFiles(['a/b/c/d/e/foo'], 'e')
    []
    >>> print selectFiles(['a/b/c/d/e/foo'], 'o')
    []
    """
    assert(len(expression) > 0)
    result = []
    for path in paths:
        if re.match(expression, os.path.basename(path)):
            result += [path]
    return result

def licenseHeader(year, author):
    """ Returns the required license header string
    >>> print licenseHeader(str(2007), 'Marko Raatikainen')
    // Copyright 2007 Marko Raatikainen.
    // Distributed under the Boost Software License, Version 1.0. (See
    // accompanying file LICENSE_1_0.txt or copy at
    // http://www.boost.org/LICENSE_1_0.txt)
    """
    assert(year > 0)
    assert(len(author) > 0)
    return "// Copyright " + year + " " + author + ".\n\
// Distributed under the Boost Software License, Version 1.0. (See\n\
// accompanying file LICENSE_1_0.txt or copy at\n\
// http://www.boost.org/LICENSE_1_0.txt)"

def startsWithLicenseHeader(content):
    """ Checks if the given string content contains the license header
    >>> startsWithLicenseHeader('')
    False
    >>> startsWithLicenseHeader('just some content')
    False
    >>> startsWithLicenseHeader('http://www.boost.org/LICENSE_1_0.txt)')
    False
    >>> startsWithLicenseHeader('Copyright 2007 Marko Raatikainen.')
    False
    >>> startsWithLicenseHeader(licenseHeader('100', 'blaa'))
    True
    >>> startsWithLicenseHeader('something\\n' + licenseHeader('100', 'blaa'))
    False
    >>> startsWithLicenseHeader(licenseHeader('100', 'blaa') + 'something\\n')
    True
    """
    expression = licenseHeader('\\d+', '.+')
    expression = re.sub(':', '\\:', expression)
    expression = re.sub('\\(', '\\(', expression)
    expression = re.sub('\\)', '\\)', expression)

    if re.match(expression, content):
        return True
    return False

def isFileLicensed(path):
    """ Checks is the file with given path licensed (i.e. has licensing info)
    >>> fileWithout = open('fileWithout.tmp', 'w')
    >>> fileWithout.write('content');
    >>> print isFileLicensed('fileWithout.tmp')
    False
    >>> fileWith = open('fileWith.tmp', 'w')
    >>> fileWith.write(licenseHeader('1', 'Anynomous Author'));
    >>> fileWith.close( )
    >>> print isFileLicensed('fileWith.tmp')
    True
    >>> fileWithout.close( )
    >>> os.remove('fileWithout.tmp')
    >>> os.remove('fileWith.tmp')
    """
    file = open(path)
    result = startsWithLicenseHeader(file.read( ))
    file.close( )
    return result

def readFile(path):
    """ Reads the contents of a file"""
    file = open(path)
    result = file.read( )
    file.close( )
    return result

def writeFile(path, content):
    """ Writes the given contents to a file"""
    file = open(path, 'w')
    file.write(content)
    file.close( )

def addLicensing(path, year, author):
    """ Adds the licensing header to a file with given path. It is assumed that
        the file exists and is already checked not to hold licensing header.
    >>> writeFile('emptyFile.tmp', '')
    >>> addLicensing('emptyFile.tmp', 2007, 'Marko Raatikainen')
    >>> print readFile('emptyFile.tmp')
    // Copyright 2007 Marko Raatikainen.
    // Distributed under the Boost Software License, Version 1.0. (See
    // accompanying file LICENSE_1_0.txt or copy at
    // http://www.boost.org/LICENSE_1_0.txt)
    <BLANKLINE>
    <BLANKLINE>
    >>> writeFile('content.tmp', 'line1\\nline2\\n')
    >>> addLicensing('content.tmp', 2007, 'Marko Raatikainen')
    >>> print readFile('content.tmp')
    // Copyright 2007 Marko Raatikainen.
    // Distributed under the Boost Software License, Version 1.0. (See
    // accompanying file LICENSE_1_0.txt or copy at
    // http://www.boost.org/LICENSE_1_0.txt)
    <BLANKLINE>
    line1
    line2
    <BLANKLINE>
    >>> os.remove('emptyFile.tmp')
    >>> os.remove('content.tmp')
    """
    writeFile(path, licenseHeader(str(year), author) + '\n\n' + readFile(path))

def addLicensingToFilesFromDirectory(path, expression):
    """Adds the licensing information if needed to all of the files in the
       given directory that matches the given expression
    """
    files = listFiles(path)
    targetFiles = selectFiles(files, expression)
    for file in targetFiles:
        if not isFileLicensed(file):
            addLicensing(file, 2007, 'Marko Raatikainen')

def checkDirectory(path):
    """ Checks that the source directory is good for use
    >>> testNoSuchDirectory( )
    >>> testDirectoryPathPointsToFile( )
    """
    if (not os.path.exists(path)):
        raise SourceDirectoryNotFound(path)
    if (not os.path.isdir(path)):
        raise SourceDirectoryInvalid(path)
    pass

def testNoSuchDirectory( ):
    """ Used for unit test of checkDirectory when directory is not existing"""
    try:
        checkDirectory("noSuchDirectoryOrVeryUnlikelyAtLeast")
        print 'An exception should have been raised when path is not found'
    except SourceDirectoryNotFound:
        pass

def testDirectoryPathPointsToFile( ):
    """ Used for unit test of checkDirectory when directory path points to
        file"""
    temporaryFile = open("justSomeFile", 'w');
    temporaryFile.write('content')
    try:
        checkDirectory("justSomeFile")
        print 'An exception should have been raised when path points to file'
    except SourceDirectoryInvalid:
        pass
    temporaryFile.close( )
    os.remove("justSomeFile")

def usage(path):
    print "Usage: " + path + " [-h] DIRECTORY"
    print "-h and --help options bring up this help"
    print "-t and --test options run the unit tests"

class OurException:
    """ This is just an interface for all local exception classes"""
    pass

class SourceDirectoryNotFound(OurException):
    def __init__(self, path):
        self.what = "Directory " + path + " not found"

class SourceDirectoryInvalid(OurException):
    def __init__(self, path):
        self.what = "Directory path " + path + " points to a file"

def main( ):
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ht", ["help", "test"])
    except getopt.GetoptError:
        print "Error parsing arguments"
        usage(sys.argv[0])
        sys.exit(2)
        
    for o, a in opts:
        if o in ("-h", "--help"):
            usage(sys.argv[0])
            sys.exit( )
        if o in ("-t", "--test"):
            print "Starting unit tests"
            doctest.testmod( )
            print "Unit tests finished"
            sys.exit( )

    try:
        addLicensingToFilesFromDirectory(args[0], args[1])
    except OurException, exception:
        print str(type(exception)) + ": " + exception.what

if __name__ == "__main__":
    main( )
