from __future__ import absolute_import

__license__ = """ 
Copyright (c) 2020 R. Tohid

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast

from typing import Callable

from phyfleaux.iter_space import Polytope


def polyhedral(fn: Callable) -> Callable:
    """Attempts to detect SCoPs and apply polyhedral transformations.
    
    :arg fn: python function.

    Directs Phyfleaux to apply polyhedral transformations on affine iteration 
    spaces in :func:`fn`.

    reads:
    -----
    https://polyhedral.info/

    https://en.wikipedia.org/wiki/Polytope_model
    https://en.wikipedia.org/wiki/Affine_space
    """

    polytope = Polytope(fn)
    return polytope
