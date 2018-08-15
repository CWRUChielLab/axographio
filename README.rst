axographio
==========

.. image:: https://img.shields.io/pypi/v/axographio.svg
    :target: PyPI_
    :alt: PyPI project

.. image:: https://img.shields.io/badge/github-source_code-blue.svg
    :target: GitHub_
    :alt: GitHub source code

.. image:: https://img.shields.io/badge/binder-launch_demo-e66581.svg
    :target: Binder_
    :alt: Launch a demo in Binder

**axographio** is a Python package that makes it easy to read and write binary
data files in the AxoGraph file format.

AxoGraph_ is a commercial software package for data acquisition and analysis
that is widely used in electrophysiological research. Although it can read and
write files in text format, its binary format is much smaller and faster to load
and save; thus many users preferentially use this format. The company
distributes the details of the file format along with sample C++ code for
reading and writing to these files using third-party software, such as this
Python package.

Python_ is a powerful and easy to use general purpose programming language.
There are many useful Python libraries available for scientific data analysis
and data visualization such as SciPy, Matplotlib, and Mayavi.

This package provides a simple interface for loading AxoGraph data files into
a Python program or interactive session. If you want to analyze data you
recorded in AxoGraph using Python-based tools, this package provides the glue
code you'll need. You can also write data to the AxoGraph binary format so that
it can be viewed and analyzed within AxoGraph.

.. _PyPI:       https://pypi.org/project/axographio/
.. _GitHub:     https://github.com/CWRUChielLab/axographio
.. _Binder:     https://mybinder.org/v2/gh/CWRUChielLab/axographio/master?filepath=examples%2Fbasic-demo.ipynb
.. _AxoGraph:   https://axograph.com
.. _Python:     https://python.org

Getting axographio
------------------

**axographio** is compatible with both Python 2 and Python 3.

The easiest way to get **axographio** is to install the latest stable version
using ``pip``, but you can alternatively build it from the source code.

Installing the latest stable version
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Requirements for installing and running **axographio**:

* The NumPy package (``pip install numpy``)

The **axographio** package contains C++ code that must be compiled. PyPI_ stores
pre-compiled copies of the package for common platforms (e.g., Python 3 on
64-bit Windows), and these can be installed using ``pip``.

To install the latest stable version, try the following::

    pip install axographio

If a pre-compiled package is available for your platform on PyPI_, ``pip``
should quickly download and install it. If not, ``pip`` will automatically
attempt to build the package from source code. Building the package has
additional requirements. If ``pip`` fails during building, keep reading.

Building from source code
~~~~~~~~~~~~~~~~~~~~~~~~~

If you need to build the package because a pre-compiled version is not already
available for your platform on PyPI_, or if you just want to try building from
the source code, you will need to meet additional requirements.

Requirements for building **axographio** from source code:

* The NumPy package (``pip install numpy``)
* The Cython package, version 0.19 or later (``pip install cython>=0.19``)
* A C++ compiler  (e.g., Visual C++ Build Tools from Microsoft on Windows
  systems, or Xcode on Mac systems)

If ``pip`` failed while trying to build from source code, make sure you meet
these requirements and try again.

If you would like to build and install using the latest development source code
from GitHub_, try the following::

    pip install git+https://github.com/CWRUChielLab/axographio

This command requires ``git``. If you don't have ``git``, you can instead
manually download the source from GitHub_ and install from your local
directory::

    pip install C:\wherever-you-put-the-source-code

Usage
-----

Try out the Binder_ demo for an interactive Python session that requires no
installation or fuss. You can start hacking *right now!*

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
column data supports the standard sequence interfaces (i.e., indexing,
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
---------------------

Please post any questions, problems, comments, or suggestions in the `GitHub
issue tracker <https://github.com/CWRUChielLab/axographio/issues>`_.

Changes
-------

0.3.1
~~~~~

* Modify NumPy's global print settings only when running tests

0.3.0
~~~~~

* Package test suite can be run using ``axographio.tests.run()``
* Package version can be accessed using ``axographio.__version__``
* Added example Jupyter notebook to source repository (not included with
  installation)
* Updated installation instructions
* Improved documentation
* Reorganized source code file structure
* Fixed doctests for NumPy < 1.14

0.2.0
~~~~~

* Added compatibility with Python 3

0.1.1
~~~~~

* Fixed a rounding error that could create one extra data point in the time
  column

0.1.0
~~~~~

* First release

Acknowledgments
---------------

This initial version of this project was written in the
Chiel Laboratory at Case Western Reserve University, with support from NIH
grant NS047073, an Ohio Innovation Incentive Award Fellowship, and the
Case Western Reserve MSTP (NIH T32 GM007250).  This project builds on a
number of other open source projects, including Python, C++ AxoGraph file
input/output code from AxoGraph Scientific (placed in the public domain; a
modified version is included with the project source code), Cython, and many
others.  Thanks also to Dr. Hillel Chiel for providing testing and helpful
suggestions.
