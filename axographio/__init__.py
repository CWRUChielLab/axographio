from .version import version as __version__
from .version import git_revision as __git_revision__

# Detect if this module is being loaded as part of the axographio setup routine
# (probably to fetch the git_revision variable in the absence of the .git
# directory), to avoid attempting to load the extension before it is built.
try:
    __AXOGRAPHIO_SETUP__
except NameError:
    __AXOGRAPHIO_SETUP__ = False

if not __AXOGRAPHIO_SETUP__:
    # must be running from an installation
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
