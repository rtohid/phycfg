from __future__ import absolute_import

__license__ = """
Copyright (c) 2020 R. Tohid

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

from phyfleaux.backend.pytiramisu import init, expr, var


class ISL:
    pass


class Array:
    def __init__(self, name: str, size: int = None, name_space=None):
        super().__init__()

    def getbuffer(self):
        pass

    def codegen(self):
        pass

    def parallelize(self):
        pass

    def vectorize(self):
        pass
