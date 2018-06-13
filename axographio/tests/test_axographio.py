""" Basic unit tests for axographio

This module contains the tests (and doctest wrappers) for the axographio
module.
"""

import doctest
import inspect
import unittest
import numpy as np
import os.path
import pkg_resources
import os
import tempfile
import copy

import axographio

example_files = {
    'old_digitized_format': pkg_resources.resource_filename(__name__,
        '../include/axograph_readwrite/AxoGraph Digitized File'),
    'old_graph_format':     pkg_resources.resource_filename(__name__,
        '../include/axograph_readwrite/AxoGraph Graph File'),
    'axograph_x_format':    pkg_resources.resource_filename(__name__,
        '../include/axograph_readwrite/AxoGraph X File.axgx'),
    }

# doctests need the array formatting used in numpy < 1.14
try:
    np.set_printoptions(legacy='1.13')
except TypeError:
    pass

class TestSampleFiles(unittest.TestCase):
    """Test the ability to read some sample data files.

    These files were provided by Axograph with their sample code in
    Axograph X, and generously released into the public domain.
    """


    def test_digitizedfile(self):
        file = axographio.read(example_files['old_digitized_format'])
        # do some sanity checks to make sure the file loaded as expected
        self.assertEqual(file.fileformat, axographio.old_digitized_format)
        self.assertEqual(len(file.names), len(file.data))
        self.assertEqual(len(file.names), 29)
        self.assertEqual(len(file.names), len(file.data))
        self.assertEqual(file.names[0], 'Time (s)')
        self.assertEqual(file.names[1], 'Current (A)')
        self.assertEqual(file.names[28], 'Column29')
        for d in file.data:
            self.assertEqual(len(d), 200)
        self.assertEqual(round(sum(file.data[1]),13), -1.973206e-07)
        self.assertEqual(round(sum(file.data[28]),7), -0.0101875)
        self.assertEqual(round(file.data[0][199],6), 0.02)


    def test_graphfile(self):
        file = axographio.read(example_files['old_graph_format'])
        # do some sanity checks to make sure the file loaded as expected
        self.assertEqual(file.fileformat, axographio.old_graph_format)
        self.assertEqual(len(file.names), len(file.data))
        self.assertEqual(len(file.names), 3)
        self.assertEqual(len(file.names), len(file.data))
        self.assertEqual(file.names[0], 'Time (sec)')
        self.assertEqual(file.names[1], 'Current (A)')
        self.assertEqual(file.names[2], 'Current (A)')
        for d in file.data:
            self.assertEqual(len(d), 2048)
        self.assertEqual(round(sum(file.data[1]),13), -2.1483816e-6)
        self.assertEqual(round(sum(file.data[2]),13), -1.4318602e-6)
        self.assertEqual(round(float(file.data[0][2047]),6), 1.024)


    def test_axograph_x_file(self):
        file = axographio.read(example_files['axograph_x_format'])
        # do some sanity checks to make sure the file loaded as expected
        self.assertEqual(file.fileformat, axographio.axograph_x_format)
        self.assertEqual(len(file.names), len(file.data))
        self.assertEqual(len(file.names), 7)
        self.assertEqual(len(file.names), len(file.data))
        self.assertEqual(file.names[0], 'Time (s)')
        self.assertEqual(file.names[1], 'Current (A)')
        self.assertEqual(file.names[2], '')
        self.assertEqual(file.names[6], '')
        for d in file.data:
            self.assertEqual(len(d), 1000)
        self.assertEqual(round(sum(file.data[1]),15), -2.3021573e-8)
        self.assertEqual(round(sum(file.data[6]),15), -2.5514375e-8)
        self.assertEqual(file.data[0][999], 0.05)



class TestReadWrite(unittest.TestCase):
    """Test reading and writing of different data types"""

    def roughly(self, x, accuracy):
        # round x to about 'accuracy' digits of precision
        array = np.asarray(x, dtype=np.float64)

        scale = np.exp(round(np.log(array.max())))
        return ((array/scale).round(accuracy))*scale

    def testreadwrite(self):
        # create a file with all of the supported data types
        timecol = [2.01, 2.02, 2.03, 2.04]
        listcol = [1.0, 2.1, 3.4, 5.3]
        seqcol = axographio.linearsequence(4, 0.12345, 0.54321)
        scaledcol = axographio.scaledarray([12, -18, 16, 42], 0.001, 0.1)
        floatcol = np.array([1.8218, 4.3672, 5.1484, 7.3235], dtype=np.float32)
        doublecol = np.array(
                [1.82187845, 4.367244252, 5.14842452445, 7.322452435],
                dtype=np.float64)
        shortcol = np.array([11043, -21274, -1834, 32341], dtype=np.int16)
        intcol = np.array([1213104371, -2027483727, -18, 3234], dtype=np.int32)
        originalfile = axographio.file_contents(
                ['time', '', 'seq', 'float', 'double', 'short', 'int'],
                [timecol, listcol, seqcol, floatcol, doublecol, shortcol,
                    intcol]
                )

        # try rewriting and rereading it multiple times
        for fileformat in axographio.supported_formats:
            currentfile = copy.deepcopy(originalfile) # to protect the original
            currentfile.fileformat = fileformat

            # the older formats may lose precision; that's OK for this
            # test.
            if fileformat == axographio.old_graph_format:
                accuracy = 5 # digits
            elif fileformat == axographio.old_digitized_format:
                accuracy = 3 # digits
            else:
                accuracy = 16 # digits

            # Do more than one pass of reading and writing to make sure the
            # data remains consistent.
            for pass_number in range(2):
                handle, tempfilename = tempfile.mkstemp()
                try:
                    currentfile.write(tempfilename)
                    currentfile = axographio.read(tempfilename)
                finally:
                    os.close(handle)
                    os.remove(tempfilename)

                # make sure the newly read data matches the original version
                self.assertEqual(currentfile.fileformat, fileformat)
                self.assertEqual(len(originalfile.names),
                        len(currentfile.names))
                self.assertEqual(len(originalfile.data), len(currentfile.data))
                self.assertTrue(np.all([a == b
                    for a,b in zip(originalfile.names, currentfile.names)]))

                for a,b in zip(originalfile.data, currentfile.data):
                    self.assertTrue(np.all(
                        self.roughly(a, accuracy) == self.roughly(b, accuracy)))



class TestRegressions(unittest.TestCase):
    """Tests for bugs that were found in previous releases"""

    # bugs fixed in 0.1.1
    def test_linearsequencelength(self):
        """ Off-by-one error in len(np.asarray(a_linearsequence))
        Found by: Dr. Hillel Chiel (hjc@case.edu)
        """
        seq = axographio.linearsequence(1000, 0., 0.036)
        seqAsArray = np.asarray(seq)
        self.assertEqual(len(seq), 1000)
        self.assertEqual(len(seqAsArray), 1000)



def test_suite():
    """Returns a test suite with all of the tests for axographio"""
    suite = unittest.TestSuite()
    fix_module_doctest(axographio.extension)
    suite.addTest(doctest.DocTestSuite(axographio.extension))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestSampleFiles))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestReadWrite))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestRegressions))
    return suite

def run():
    """Run the test suite containing all of the tests for axographio"""
    unittest.TextTestRunner(verbosity=2).run(test_suite())

#
# The following two functions work around an incompatability with doctest and
# Cython.  These two functions were cut and pasted from the Cython FAQ at
# https://github.com/cython/cython/wiki/FAQ#how-can-i-run-doctests-in-cython-code-pyx-files
# - see this FAQ for more details.
#


def _from_module(module, object):
    """ Return true if the given object is defined in the given module."""
    if module is None:
        return True
    elif inspect.getmodule(object) is not None:
        return module is inspect.getmodule(object)
    elif inspect.isfunction(object):
        return module.__dict__ is object.func_globals
    elif inspect.isclass(object):
        return module.__name__ == object.__module__
    elif hasattr(object, '__module__'):
        return module.__name__ == object.__module__
    elif isinstance(object, property):
        return True # [XX] no way not be sure.
    else:
        raise ValueError("object must be a class or function")

def fix_module_doctest(module):
    """
    Extract docstrings from cython functions, that would be skipped by doctest
    otherwise.
    """
    module.__test__ = {}
    for name in dir(module):
       value = getattr(module, name)
       if (inspect.isbuiltin(value) and isinstance(value.__doc__, str)
               and _from_module(module, value)):
           module.__test__[name] = value.__doc__


if __name__ == '__main__':
    run()
