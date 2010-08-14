#! /usr/bin/env python
#
# @todo documentation
#
# $Id$

import getopt
import sys
import doctest
import os.path
import re

def openFile(path):
    """ @todo """
    if (not os.path.exists(path) or not os.path.isfile(path)):
        raise FileNotFound("Failed to find template project file " + path)

    return open(path, 'r')

def templateProjectFile( ):
    """ Returns a path to the template project file. Throws if file does not
    exist.
    >>> handle = templateProjectFile( )
    >>> handle.name
    'build/vs8/template/template.vcproj'
    >>> handle.close( )
    """
    return openFile('build/vs8/template/template.vcproj')

def templateSourceFile( ):
    """ Returns a path to the template source file. Throws if file does not
    exist.
    >>> handle = templateSourceFile( )
    >>> handle.name
    'build/vs8/template/template.cpp'
    >>> handle.close( )
    """
    return openFile('build/vs8/template/template.cpp')

def extractProjectRootFromPath(path):
    """ @todo documentation
    >>> extractProjectRootFromPath('')
    ''
    >>> extractProjectRootFromPath('myrrh')
    ''
    >>> extractProjectRootFromPath('myrrh/')
    ''
    >>> extractProjectRootFromPath('myrrh/folder')
    'folder/'
    >>> extractProjectRootFromPath('myrrh/folder/subfolder')
    'folder/subfolder/'
    >>> extractProjectRootFromPath('myrrh/folder/subfolder/')
    'folder/subfolder/'
    >>> extractProjectRootFromPath('folder/subfolder')
    'folder/subfolder/'
    >>> extractProjectRootFromPath('folder/subfolder/myrrh')
    'folder/subfolder/myrrh/'
    >>> extractProjectRootFromPath('folder/subfolder/myrrh/')
    'folder/subfolder/myrrh/'
    """
    parts = path.split('/')
    if parts[0] == 'myrrh':
        path = path[6:]
    if not len(path):
        return path
    if path[len(path) - 1] != '/':
        path = path + '/'
    return path

def extractProjectFromFileName(fileName):
    """ Extracts the project name from header file name 
    >>> extractProjectFromFileName('file.hpp')
    'file'
    >>> extractProjectFromFileName('fileName.hpp')
    'fileName'
    >>> extractProjectFromFileName('File.hpp')
    'file'
    >>> extractProjectFromFileName('FileName.hpp')
    'fileName'
    >>> extractProjectFromFileName('NameWithManyManyParts.hpp')
    'nameWithManyManyParts'
    """
    assert(len(fileName) > 0)
    noHpp = fileName.split('.')[0]
    return noHpp[0].lower( ) + noHpp[1:]

def buildProjectNameFromPath(fileName):
    """ Builds the name of the test driver project from header file path
    >>> buildProjectNameFromPath('file.hpp')
    'TestFile'
    >>> buildProjectNameFromPath('myrrh/file.hpp')
    'TestFile'
    >>> buildProjectNameFromPath('myrrh/folder/file.hpp')
    'TestFolderFile'
    >>> buildProjectNameFromPath('myrrh/folder/subfolder/file.hpp')
    'TestFolderSubfolderFile'
    """
    assert(len(fileName) > 0)
    parts = extractProjectRootFromPath(fileName).split('.')[0].split('/');
    result = 'Test'
    for part in parts:
        result = result + part[0].upper( ) + part[1:]
    return result

def buildFolderStructure(headerPath):
    """ Builds a folder structure as path string from given header path
    >>> buildFolderStructure('file.hpp')
    'test/'
    >>> buildFolderStructure('folder/FileName.hpp')
    'folder/test/'
    >>> buildFolderStructure('folder/subFolder/FileName.hpp')
    'folder/subFolder/test/'
    >>> buildFolderStructure('myrrh/folder/subFolder/Filename.hpp')
    'folder/subFolder/test/'
    """
    assert(len(headerPath) > 0)
    directory = os.path.dirname(headerPath)
    return extractProjectRootFromPath(directory) + "test/"

def buildProjectFilePath(headerPath):
    """ Builds a string that is usable as a path to the resulting project file
    >>> buildProjectFilePath('File.hpp')
    'build/vs8/test/file/file.vcproj'
    >>> buildProjectFilePath('folder/File.hpp')
    'build/vs8/folder/test/file/file.vcproj'
    >>> buildProjectFilePath('folder/subfolder/File.hpp')
    'build/vs8/folder/subfolder/test/file/file.vcproj'
    >>> buildProjectFilePath('folder/NameWithManyWords.hpp')
    'build/vs8/folder/test/nameWithManyWords/nameWithManyWords.vcproj'
    """
    assert(len(headerPath) > 0)
    project = extractProjectFromFileName(os.path.basename(headerPath))
    return "build/vs8/" +\
           buildFolderStructure(headerPath) +\
           project + "/" +\
           project +\
           ".vcproj"

def buildSourceFilePath(headerPath):
    """ Builds a string that is usable as a path to the resulting source file
    >>> buildSourceFilePath('File.hpp')
    'src/test/TestFile.cpp'
    >>> buildSourceFilePath('folder/File.hpp')
    'src/folder/test/TestFile.cpp'
    >>> buildSourceFilePath('folder/subfolder/File.hpp')
    'src/folder/subfolder/test/TestFile.cpp'
    >>> buildSourceFilePath('folder/NameWithManyWords.hpp')
    'src/folder/test/TestNameWithManyWords.cpp'
    """
    assert(len(headerPath) > 0)
    fileName = re.sub('\.\w+', '.cpp', os.path.basename(headerPath))
    return 'src/' +\
           buildFolderStructure(headerPath) +\
           'Test' +\
           fileName

def prepareFolder(path):
    """ First checks if the directory already exists. If it does, then it is
    checked if the file exists. If it does an exception is thrown. If it does
    not, nothing is done as the folder already exists. If the folder does not
    exist, it is created.
    >>> directory = 'PathThatShouldNotExist/'
    >>> path = directory + 'File.postfix'
    >>> os.path.exists(directory)
    False
    >>> prepareFolder(path)
    >>> os.path.exists(directory)
    True
    >>> os.path.exists(path)
    False
    >>> testExistingTargetFile(path)
    >>> prepareFolder(path)
    >>> os.path.exists(directory)
    True
    >>> os.path.exists(path)
    False
    >>> os.removedirs(directory)
    """
    if os.path.exists(path):
        raise TargetExists("Target file " + path + " already exists")
    directory = os.path.dirname(path)
    if not os.path.exists(directory):
        os.makedirs(directory)

def testExistingTargetFile(path):
    """ Used for unit tests of prepareFolder """
    temporaryFile = open(path, 'w')
    temporaryFile.write('Just some content without purpose')
    try:
        prepareFolder(path)
        print 'An exception should have been raised'
    except TargetExists:
        pass
    temporaryFile.close( )
    os.remove(path)

def handleHeader(path):
    """ Creates a project file and source file for given header file.
    @todo add more tests
    >>> handleHeader('myrrh/AHeaderThatDefinitelyShouldNotExist.hpp')
    Failed to find myrrh/AHeaderThatDefinitelyShouldNotExist.hpp
    """
    if (not os.path.exists(path)):
        print "Failed to find", path
        return

    sourceFile = SourceFile(path)
    projectFile = ProjectFile(path)
    sourceFile.write( )
    projectFile.write( )

class SourceFile:
    """ Generates a proper source file as a basis for a new test driver. """
    
    def __init__(self, headerPath):
        """ Constructor.
        >>> directory = 'JustForTest/'
        >>> path = directory + 'header.hpp'
        >>> sourceFile = SourceFile(path)
        >>> os.path.exists('src/JustForTest/test/')
        False
        >>> os.path.exists('src/JustForTest/test/Testheader.cpp')
        False
        >>> print sourceFile.targetPath
        src/JustForTest/test/Testheader.cpp
        """
        self.headerPath = headerPath
        templateFile = templateSourceFile( )
        self.content = templateFile.read( )
        templateFile.close( )
        self.targetPath = buildSourceFilePath(headerPath)

    def write(self):
        """ Writes the source file
        >>> sourceFile = SourceFile('JustForTest/header.hpp')
        >>> sourceFile.write( )
        >>> sourcePath = 'src/JustForTest/test/TestHeader.cpp'
        >>> os.path.exists(sourcePath)
        True
        >>> testFile = open(sourcePath)
        >>> print re.sub('\$Id.*\$', '$Id$', testFile.read( ))
        /**
         * This file contains the unit test(s) for header
         *
         * $Id$
         */
        <BLANKLINE>
        #include \"JustForTest/header.hpp\"
        <BLANKLINE>
        #define DISABLE_CONDITIONAL_EXPRESSION_IS_CONSTANT
        #include \"myrrh/util/Preprocessor.hpp\"
        <BLANKLINE>
        #include \"boost/test/unit_test.hpp\"
        <BLANKLINE>
        #ifdef WIN32
        #pragma warning(pop)
        #endif
        <BLANKLINE>
        typedef boost::unit_test::test_suite TestSuite;
        <BLANKLINE>
        void Testheader( )
        {
            BOOST_FAIL(\"Unimplemented\");
        }
        <BLANKLINE>
        TestSuite *init_unit_test_suite(int, char *[])
        {
            TestSuite* test = BOOST_TEST_SUITE(\"Test suite for header\");
            test->add(BOOST_TEST_CASE(Testheader));
            return test;
        }
        <BLANKLINE>
        >>> testFile.close( )
        >>> os.remove(sourcePath)
        >>> os.removedirs('src/JustForTest/test/')
        """
        assert(len(self.content) > 0)
        prepareFolder(self.targetPath)
        target = open(self.targetPath, 'w')
        moduleName = re.sub('\.\w+', '', os.path.basename(self.headerPath))
        newContent = re.sub('__TO_BE_REPLACED__',
                            moduleName, self.content)
        newContent = re.sub('__INSERT_PATH_HERE__', self.headerPath,
                            newContent)
        target.write(newContent)
        target.close( )

class ProjectFile:
    """ Generates a project file as a basis for a new test driver. """
    def __init__(self, headerPath):
        """ Constructor.
        >>> projectFile = ProjectFile('JustForTest/header.hpp')
        >>> os.path.exists('build/vs8/JustForTest/test/')
        False
        >>> os.path.exists('src/JustForTest/test/header/header.vcproj')
        False
        """
        self.headerPath = headerPath
        templateFile = templateProjectFile( )
        self.content = templateFile.read( )
        templateFile.close( )
        self.targetPath = buildProjectFilePath(headerPath)
        self.projectName = buildProjectNameFromPath(headerPath)

    def write(self):
        """ Writes the project file
        >>> projectFile = ProjectFile('JustForTest/header.hpp')
        >>> projectFile.write( )
        >>> path = 'build/vs8/JustForTest/test/header/header.vcproj'
        >>> os.path.exists(path)
        True
        >>> testFile = open(path)
        >>> content =  testFile.read( )
        >>> re.findall('AdditionalIncludeDirectories=\"(.+)\"', content)
        ['..\\\\..\\\\..\\\\..\\\\..', '..\\\\..\\\\..\\\\..\\\\..']
        >>> re.findall('RelativePath=\"(.+)\"', content)
        ['..\\\\..\\\\..\\\\..\\\\..\\\\src\\\\justfortest\\\\test\\\\testheader.cpp']
        >>> testFile.close( )
        >>> os.remove(path)
        >>> os.removedirs('build/vs8/JustForTest/test/header/')
        """
        assert(len(self.content) > 0)
        prepareFolder(self.targetPath)
        target = open(self.targetPath, 'w')
        newContent = re.sub('__TEMPLATE__NAME_TO_BE_REPLACED__',
                            self.projectName,
                            self.content)
        includePath = os.path.normcase(self.GetIncludePath( ))
        newContent = re.sub('AdditionalIncludeDirectories=\".*\"',
                            'AdditionalIncludeDirectories=\"' +\
                            repr(includePath)[1:-1] + '\"',
                            newContent)
        sourcePath = os.path.normcase(self.GetIncludePath( ) + '/' +\
                                      buildSourceFilePath(self.headerPath));

        newContent = re.sub('RelativePath=\".*\"',
                            'RelativePath=\"' +\
                            repr(sourcePath)[1:-1] + '\"',
                            newContent)
        target.write(newContent)
        target.close( )

    def GetIncludePath(self):
        """ Builds the include directory from project file to myrrh root
        >>> projectFile = ProjectFile('File.hpp')
        >>> projectFile.GetIncludePath( )
        '../../../..'
        >>> projectFile = ProjectFile('myrrh/File.hpp')
        >>> projectFile.GetIncludePath( )
        '../../../..'
        >>> projectFile = ProjectFile('Folder/File.hpp')
        >>> projectFile.GetIncludePath( )
        '../../../../..'
        >>> projectFile = ProjectFile('myrrh/Folder/File.hpp')
        >>> projectFile.GetIncludePath( )
        '../../../../..'
        >>> projectFile = ProjectFile('myrrh/Folder/subfolder/File.hpp')
        >>> projectFile.GetIncludePath( )
        '../../../../../..'
        >>> projectFile = ProjectFile('myrrh/util/file/Copy.hpp')
        >>> projectFile.GetIncludePath( )
        '../../../../../..'
        """
        rootless = extractProjectRootFromPath(self.headerPath)
        result = '../../..'
        for char in rootless:
            if char == '/':
                result = result + '/..'
        return result

def handleHeaders(pathList):
    """ Goes through given list of paths and does project files for each of
    them. @todo Add more tests
    >>> testNoHeaders( )
    """
    if len(pathList) == 0:
        raise NoHeaders

    for path in pathList:
        handleHeader(path)

def testNoHeaders( ):
    """ Used for unit tests for handleHeaders"""
    try:
        handleHeaders([])
        print 'Giving empty list should throw'
    except NoHeaders:
        pass

def usage(path):
    print "Usage: " + path + " [-ht] HEADER [HEADER_2 ...]"
    print "-h and --help options bring up this help"
    print "-t and --test options run the unit tests"

class OurException:
    """ This is just an interface for all local exception classes"""
    pass

class NoHeaders(OurException):
    what = "There are no paths to header files to process "

class FileNotFound(OurException):
    def __init__(self, error):
        self.what = error

class TargetExists(OurException):
    def __init__(self, error):
        self.what = error

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

    try:
        handleHeaders(args)
    except OurException, exception:
        print str(type(exception)) + ": " + exception.what

if __name__ == "__main__":
    main( )
