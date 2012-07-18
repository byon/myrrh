#! /usr/bin/env python
# encoding: utf-8

import os, sys

VERSION='0.0.1'
APPNAME='myrrh'

top = '.'
if sys.platform == 'win32':
    out = 'build'
else:
    # Win32 build has variants, which adds one layer to the directory
    # hierarchy until unit test projects. We need to make the hierarchies of
    # the same depth, so the tests work on both platforms.
    out = 'build/linux'

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    if sys.platform == 'win32':
        configureDebugBuild(conf)
        configureReleaseBuild(conf)
    else:
        conf.load('compiler_cxx')
        setBoostConfigurationLinux(conf)

def build(bld):
    checkVariantIsDefined(bld)
    bld.recurse('src')

def configureDebugBuild(conf):
    conf.setenv('debug')
    conf.load('compiler_cxx')
    setCxxFlags(conf, ['/ZI', '/RTC1', '/Gm', '/Od', '/D "_DEBUG"', '/MDd'])
    setLinkerFlags(conf, ['/INCREMENTAL'])
    conf.env.INCLUDES += ['src']
    configureLibraries(conf)

def configureReleaseBuild(conf):
    conf.setenv('release')
    conf.load('compiler_cxx')
    setCxxFlags(conf, ['/Zi', '/Gy', '/Gm-', '/O2', '/Oi', '/GL',
                       '/D "NDEBUG"', '/MD'])
    setLinkerFlags(conf, ['/INCREMENTAL:NO', '/LTCG', '/OPT:ICF', '/OPT:REF'])
    conf.env.INCLUDES += ['src']
    conf.env.ARFLAGS += ['/LTCG']
    configureLibraries(conf)

def configureLibraries(conf):
    setBoostConfiguration(conf)

def checkVariantIsDefined(bld):
    if sys.platform != 'win32':
        return
    if bld.__class__.__name__ == "msvs_generator":
        return
    if not bld.variant:
        bld.fatal('call "waf build_debug" or "waf build_release", ' +
                  'and try "waf --help"')

def defineSeparateConfigurations( ):
    from waflib.Build import (BuildContext, CleanContext, InstallContext,
                              UninstallContext)

    for configuration in 'debug release'.split():
        for context in (BuildContext, CleanContext, InstallContext,
                        UninstallContext):
            name = context.__name__.replace('Context','').lower()
            class ConfigurationContext(context):
                cmd = name + '_' + configuration
                variant = configuration

# @todo windows and linux are actually so close to each other that try to
# combine
def setBoostConfiguration(conf):
    boost_path = 'C:\\Utilities\\boost\\boost_1_50_0'
    conf.env.STLIBPATH_boost = [boost_path + '\\stage\\lib']
    conf.env.INCLUDES_boost = [boost_path]

def setBoostConfigurationLinux(conf):
    boost_path = '/home/byon/src/vendor/boost/boost_1_50_0/'
    conf.env.STLIB_boost = ['boost_thread',
                            'boost_regex',
                            'boost_system',
                            'boost_filesystem',
                            'boost_date_time',
                            'boost_unit_test_framework']
    conf.env.LIB_boost = ['pthread']
    conf.env.STLIBPATH_boost = [boost_path + 'stage/lib']
    conf.env.INCLUDES_boost = [boost_path]

def setCxxFlags(conf, flags):
    conf.env.CXXFLAGS += flags + commonCxxFlags( )

def commonCxxFlags( ):
    return ['/W3', '/WX-', '/analyze-', '/errorReport:queue',
            '/Oy-', '/DWIN32', '/D_CONSOLE', '/D_UNICODE',
            '/Zc:forScope', '/DUNICODE', '/EHsc', '/Zc:wchar_t',
            '/Fdvc100.pdb']

def setLinkerFlags(conf, flags):
    conf.env.LINKFLAGS += flags + commonLinkerFlags( )

def commonLinkerFlags( ):
    # @todo Work to get rid of the linker warning instead of just hiding the
    #       problem
    return ['/DEBUG', '/ERRORREPORT:QUEUE', '/SUBSYSTEM:CONSOLE',
            'ignore:4099']

from waflib.Tools.waf_unit_test import utest

class UnitTest(utest):

    def run(self):
        self.addDllPaths( )
        self.setRunningDirectory(self.inputs[0])
        utest.run(self)
        return self.reportFailures(self.generator.bld)

    def setRunningDirectory(self, node):
        directory = self.runningDirectory(node)
        if not os.path.exists(directory):
            os.makedirs(directory)
        setattr(self.generator, 'ut_cwd', directory)

    def runningDirectory(self, node):
        base = os.path.splitext(node.name)[0]
        return os.path.join(node.parent.abspath( ), base)

    def reportFailures(self, bld):
        returnValue = 0
        for (file, code, stdout, stderr) in getattr(bld, 'utest_results', []):
            import sys
            if code != 0:
                print(stdout.decode('utf-8'))
                print(stderr.decode('utf-8'))
                returnValue = returnValue +1
        setattr(bld, 'utest_results', [])
        return returnValue

    def addDllPaths(self):
        for path in self.env.DLL_PATHS:
            self.addToPath(path)

    def addToPath(self, toAdd):
        import os
        original = os.environ.get('PATH')
        for current in original.split(';'):
            if os.path.exists(current):
                if os.path.samefile(current, toAdd):
                    return
        os.environ['PATH'] = original + ';' + toAdd


from waflib.TaskGen import feature, after_method

@feature('UnitTest')
@after_method('apply_link')
def make_test(self):
    if getattr(self, 'link_task', None):
        self.create_task('UnitTest', self.link_task.outputs)

if sys.platform == 'win32':
    defineSeparateConfigurations( )
