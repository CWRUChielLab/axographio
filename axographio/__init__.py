"""A Python package for reading and writing AxoGraph data files

Use `f = axographio.read(filename)` to load an AxoGraph file. Data and column
names are accessed as lists with `f.data` and `f.names`. Data objects (with
types `axographio.extension.linearsequence` and
`axographio.extension.scaledarray`) can be converted to NumPy or SciPy arrays,
e.g., `numpy.asarray(f.data[0])`.

Tests can be run using `axographio.tests.run()`.

To get started, try executing this code, which uses an example AxoGraph data
file included with the axographio package:

>>> import axographio
>>> import matplotlib.pyplot as plt
>>>
>>> f = axographio.read(axographio.example_files['axograph_x_format'])
>>>
>>> plt.plot(f.data[0], f.data[1])
>>> plt.xlabel(f.names[0])
>>> plt.ylabel(f.names[1])
>>> plt.show()  # may be optional depending on your OS
"""

from .version import version as __version__
from .version import git_revision as __git_revision__
from .extension import *
from . import tests
from .tests.test_axographio import example_files

# explicit listing needed to ensure pydoc/help() finds everything
__all__ = [
    'file_contents',
    'linearsequence',
    'scaledarray',
    'aslinearsequence',
    'asscaledarray',
    'read',
    'axograph_x_format',
    'newest_format',
    'old_digitized_format',
    'old_graph_format',
    'supported_formats',
    'example_files',
    ]
