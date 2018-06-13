"""Test suite for axographio

Run all tests using axographio.tests.run()
"""

import unittest
from . import test_axographio
from .test_axographio import run

# explicit listing needed to ensure pydoc/help() finds everything
__all__ = [
    'run',
    ]
