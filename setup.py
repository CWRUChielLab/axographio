#!/usr/bin/env python

from setuptools import setup
from distutils.extension import Extension

# setuptools DWIM monkey-patch madness
# http://mail.python.org/pipermail/distutils-sig/2007-September/thread.html#8204
import sys
import numpy

if 'setuptools.extension' in sys.modules:
    m = sys.modules['setuptools.extension']
    m.Extension.__dict__ = m._Extension.__dict__

setup(
    name = "axographio",
    version = 0.1,
    description = "Importer for Axograph data files",
    setup_requires = ['setuptools_cython', 'numpy'],
    ext_modules = [
        Extension('axographio', [
            'axographio.pyx', 
            'axograph_readwrite/fileUtils.cpp', 
            'axograph_readwrite/byteswap.cpp', 
            'axograph_readwrite/stringUtils.cpp', 
            'axograph_readwrite/AxoGraph_ReadWrite.cpp'], 
            language='c++', include_dirs=[numpy.get_include()],
            define_macros=[('NO_CARBON',1)]
            )
        ],
    test_suite = 'test_axographio.test_all'
)

