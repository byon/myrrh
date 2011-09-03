#! /usr/bin/env python
# encoding: utf-8

import sys

VERSION='0.0.1'
APPNAME='myrrh'

top = '.'
out = 'build'

def options(opt):
	opt.load('compiler_cxx')

def configure(conf):
	conf.load('compiler_cxx')
        conf.check(header_name='stdio.h', features='cxx cxxprogram',
                   mandatory=False)

        if sys.platform != 'win32':
                conf.env.STLIB_BOOST = ['boost_thread',
                                        'boost_regex',
                                        'boost_system',
                                        'boost_filesystem',
                                        'boost_unit_test_framework']
                conf.env.LIB_BOOST = ['pthread']
                conf.env.STLIBPATH_BOOST = ['/usr/lib']
                conf.env.INCLUDES_BOOST = ['/usr/include']

def build(bld):
	bld.recurse('src')
