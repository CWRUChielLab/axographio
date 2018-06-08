__version__ = '0.2.0'

from .extension import *

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
    'supported_formats'
    ]
