#!/usr/bin/env python
"""
axographio is a library that makes it easy to read and write binary data files
in the AxoGraph file format.  

AxoGraph X is a commercial software package used for data acquisition and
analysis that is widely used in electrophysiological research (see
http://axographx.com for more details).  While it can read and write files in
text format, its binary format is much smaller and faster to load and save;
thus many users preferentially use this format.  The company distributes the
details of the file format along with sample C++ code for reading and writing
to these files with AxoGraph X.

Python is a powerful and easy to use general purpose programming language (see
http://python.org for more details).  There are many useful python libraries
available for scientific data analysis and data visualization such as scipy,
matplotlib and MayaVI.  

This library provides a simple interface for loading AxoGraph data files into
a python program or interactive session.  If you want to analyze data you
recorded in AxoGraph using python based tools, this library provides the glue
code you'll need.  

Installation
============

Preinstallation Requirements
----------------------------

* A working Python installation
* The setuptools package
* The NumPy package
* The Cython package

Note that NumPy takes a bit of work to build, so it may be easiest to install
it from your linux distribution's repository, or use as pre-built package
such as the Scipy Superpack (http://macinscience.org/?page_id=6) for the mac. 
Depending on your OS, you may be able to get away with simply typing:

::

 sudo easy_install numpy
 sudo easy_install Cython


Installation
------------

Once all the preinstallation requirements have been met, you can download and
install axographio using easy_install by typing the following command in a
terminal window:

::

 easy_install axographio


Upgrading
---------

If you have an older version of the package installed, you can update it to 
the newest version using easy_install with the "-U" flag:

::
 
 easy_install -U axographio

Usage
=====

Loading a data file is as easy as calling `read`:

>>> import axographio
>>>
>>> f = axographio.read("AxoGraph X File.axgx") 

At this point the variable f will contain a file_contents object with the
column names and data from the file.  For example, you could now plot the first
two columns using matplotlib:

>>> import matplotlib.pyplot as plt
>>> 
>>> plt.plot(f.data[0], f.data[1]) 
>>> plt.xlabel(f.names[0]) 
>>> plt.ylabel(f.names[1])
>>> plt.show() 

(The plt.show() command may be optional depending on your OS.)

Of course, you probably have grander plans than just plotting the data.  The
column data supports the standard sequence interfaces (i.e. indexing,
iteration, etc.) and can be converted to a scipy or numpy array using the
asarray functions in these packages, e.g.:

>>> import scipy as sp
>>>
>>> times = sp.asarray(f.data[0])

Writing files is also relatively easy.  You simply create a new file_contents
object (or use one you loaded earlier), and then call write.  For example, the
following code creates a file in the current directory called "my60Hz.axgx" 
with two channels with 60 Hz sine waves

>>> import axographio 
>>> import numpy as np
>>>
>>> times = np.arange(0,10,0.0001) 
>>> column1 = np.sin(2*np.pi * 60 * times) 
>>> column2 = np.cos(2*np.pi * 60 * times) 
>>> f = axographio.file_contents(
...    ['time (s)', 'my recording (V)', 'your recording (V)'], 
...    [times, column1, column2])
>>> f.write("my60Hz.axgx")


Questions and Support
=====================

Please post any questions, problems, comments, or suggestions on the axographio
group on google groups (http://groups.google.com/group/axographio)


News
====

0.1.1
-----
    Fixed a rounding error that could create one extra data point in the time 
    column. 

0.1.0
-----
    First release


Acknowledgments
===============

This initial version of this project was written in the
Chiel Laboratory at Case Western Reserve University, with support from NIH
grant NS047073, an Ohio Innovation Incentive Award Fellowship, and the
Case Western Reserve MSTP (NIH T32 GM007250).  This project builds on a 
number of other open source projects, including Python, C++ AxoGraph file
input/output code from AxoGraph Scientific (placed in the public domain; a
modified version is included with the project source code), Cython, and many
others.  Thanks also to Dr. Hillel Chiel for providing testing and helpful 
suggestions.  
"""

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
    version = "0.1.1b1",
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
    test_suite = 'test_axographio.test_all',
    # metatdata
    author = "Kendrick Shaw",
    author_email = "kms15@case.edu",
    license = "BSD License",
    keywords = ["physiology","electrophysiology","axograph"],
    url = "http://code.google.com/p/axographio/",
    description = "A python library for reading and writing AxoGraph data files",
    classifiers = ["Development Status :: 4 - Beta", "Intended Audience :: Developers", 
        "Intended Audience :: Science/Research", "License :: OSI Approved :: BSD License",
        "Topic :: Scientific/Engineering :: Bio-Informatics"],
    long_description = __doc__
    )

