.. image:: https://mybinder.org/badge.svg
    :target: https://mybinder.org/v2/gh/CWRUChielLab/axographio/master?filepath=examples%2Fbasic-demo.ipynb

axographio is a Python package that makes it easy to read and write binary data
files in the AxoGraph file format.

AxoGraph X is a commercial software package used for data acquisition and
analysis that is widely used in electrophysiological research (see
https://axograph.com for more details).  While it can read and write files in
text format, its binary format is much smaller and faster to load and save;
thus many users preferentially use this format.  The company distributes the
details of the file format along with sample C++ code for reading and writing
to these files with AxoGraph X.

Python is a powerful and easy to use general purpose programming language (see
https://python.org for more details).  There are many useful Python libraries
available for scientific data analysis and data visualization such as SciPy,
Matplotlib and Mayavi.

This package provides a simple interface for loading AxoGraph data files into
a Python program or interactive session.  If you want to analyze data you
recorded in AxoGraph using Python based tools, this package provides the glue
code you'll need.

Installation
============

Preinstallation Requirements
----------------------------

* Python 2 or Python 3
* The ``pip`` package manager (``easy_install`` may also work)
* The NumPy package (``pip install numpy``)
* The Cython package, version 0.19 or later (``pip install cython>=0.19``)
* Git (unless you install from a local copy of the source)

axographio is compatible with both Python 2 and Python 3. It requires both NumPy
and Cython to build, and it requires NumPy to run.

The package includes an extension that is built from C++ code using Cython. You
may need to install a C++ compiler if you do not already have one (e.g., Visual
C++ Build Tools from Microsoft on Windows systems).

Git may be needed for ``pip`` to understand the ``git+https`` protocol. If you
don't want to install Git, you can circumvent this by manually downloading the
source and installing from your local directory.

Installing & Upgrading
----------------------

    NOTE: As of 2018-06-01, this fork of axographio
    (`github.com/CWRUChielLab/axographio
    <https://github.com/CWRUChielLab/axographio>`_) is not on PyPI, and the
    `version available there <https://pypi.org/project/axographio/>`_ is
    outdated. Running ``pip install axographio`` will utilize PyPI as the
    default package index and will therefore not install this updated version.
    Follow these instructions to build from source using this fork.

You can simultaneously download, build, and install axographio by typing the
following command in a terminal window::

    pip install git+https://github.com/CWRUChielLab/axographio

If you have an older version of the package installed, you can update it to
the newest version using the ``-U`` flag::

    pip install -U git+https://github.com/CWRUChielLab/axographio

Usage
=====

Loading a data file is as easy as calling ``read``:

>>> import axographio
>>>
>>> f = axographio.read('AxoGraph X File.axgx')

At this point the variable ``f`` will contain a ``file_contents`` object with
the column names and data from the file.  For example, you could now plot the
first two columns using Matplotlib:

>>> import matplotlib.pyplot as plt
>>>
>>> plt.plot(f.data[0], f.data[1])
>>> plt.xlabel(f.names[0])
>>> plt.ylabel(f.names[1])
>>> plt.show()  # may be optional depending on your OS

Of course, you probably have grander plans than just plotting the data.  The
column data supports the standard sequence interfaces (i.e. indexing,
iteration, etc.) and can be converted to a NumPy or SciPy array using the
``asarray`` functions in these packages, e.g.:

>>> import numpy as np
>>>
>>> times = np.asarray(f.data[0])

Writing files is also relatively easy.  You simply create a new
``file_contents`` object (or use one you loaded earlier), and then call
``write``.  For example, the following code creates a file in the current
directory called 'my60Hz.axgx' with two channels with 60 Hz sine waves:

>>> import axographio
>>> import numpy as np
>>>
>>> times = np.arange(0, 10, 0.0001)
>>> column1 = np.sin(2*np.pi * 60 * times)
>>> column2 = np.cos(2*np.pi * 60 * times)
>>> f = axographio.file_contents(
...    ['time (s)', 'my recording (V)', 'your recording (V)'],
...    [times, column1, column2])
>>> f.write('my60Hz.axgx') # created in the current directory

Questions and Support
=====================

Please post any questions, problems, comments, or suggestions in the `GitHub
issue tracker <https://github.com/CWRUChielLab/axographio/issues>`_.

News
====

0.2.0
-----
    Added compatibility with Python 3.

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
