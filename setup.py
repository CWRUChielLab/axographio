#!/usr/bin/env python

from setuptools import setup, Extension
import numpy

with open("README.rst", "r") as f:
    long_description = f.read()

setup(
    name = "axographio",
    version = "0.2.0",
    setup_requires   = ['numpy', 'cython>=0.19'],   # needed to build package
    install_requires = ['numpy'],                   # needed to run package
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
    test_suite = 'test_axographio.test_all',
    # metatdata
    author = "Kendrick Shaw, Jeffrey Gill",
    author_email = "kms15@case.edu, jeffrey.p.gill@gmail.com",
    license = "BSD License",
    keywords = ["physiology","electrophysiology","axograph"],
    url = "https://github.com/CWRUChielLab/axographio",
    description = "A Python package for reading and writing AxoGraph data files",
    classifiers = [
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: BSD License",
        "Topic :: Scientific/Engineering :: Bio-Informatics"
        ],
    long_description = long_description
    )
