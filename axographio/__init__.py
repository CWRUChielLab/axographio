from .version import version as __version__
from .version import git_revision as __git_revision__
from .extension import *
from . import tests

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
