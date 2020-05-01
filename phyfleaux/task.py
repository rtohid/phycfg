__license__ = """
Copyright (c) 2020 R. Tohid

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast

from inspect import getsource
from typing import Callable


class Task(ast.NodeVisitor):
    def __init__(self,
                 fn: Callable,
                 backend: object = lambda: None,
                 cost_function: Callable = lambda: None) -> None:
        """
        Phyfleaux callable.

        :arg fn: a Python callable
        :arg backend: a particular implementation of the :func:`fn`.
        :arg cost_function: cost associated to computation of the :func:`fn`.
        """

        # function, it's tree representation, and id (hash of tree).
        self.fn = fn
        self.tree = ast.parse((getsource(fn)))
        self.id = hash(self.tree)

        # self.args = defaultdict()
        self.args = list()
        for arg in self.tree.body[0].args.args:
            self.args.append(arg.arg)
            # self.args[arg.arg] = arg

        # user provides the cost function to be minimized.
        self.cost = cost_function

        # might be useful for derivative calculations.
        self.derivative = lambda: None
        # self.forward_propagation = None
        # self.backward_propagation = None

    def __call__(self, *args, **kwargs):
        return self.fn(*args, **kwargs)
