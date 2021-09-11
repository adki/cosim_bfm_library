# The __init__.py file makes Python treat directories containing it as modules.
# 1. Hide confmc Python package hierarchy
# 2. Somthing to be initialized
#
# "from cosim.cosim_bfm import *", where 'cosim' is package.
# The funy single dot before module name is read as "current package"

from .cosim_bfm import *
