__license__ = """
Copyright (c) 2020 R. Tohid

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast
from collections import OrderedDict, defaultdict
from copy import deepcopy
from typing import Callable

from phyfleaux.task import Task


class CALL:
    pass


class DEFINE:
    pass


class Load:
    pass


class Store:
    pass


class Function:
    def __init__(self, name, dtype=None):
        self.name = name
        self.args = list()
        self.body = list()
        self.context = None


class Statement:
    statements = OrderedDict()

    def __init__(self):
        self._lhs = None
        self._rhs = None
        self.name = 'S' + str(len(Statement.statements))
        Statement.statements[self.name] = self

    @property
    def lhs(self):
        return self._lhs

    @lhs.setter
    def lhs(self, targets):
        self._lhs = targets

    @property
    def rhs(self):
        return self._rhs

    @rhs.setter
    def rhs(self, expr):
        self._rhs = expr


class Computation:
    pass


class Buffer:
    def __init__(self, name):
        self.name = name
        self.indices = list()
        self.context = None

    def dimension(self):
        return len(self.indices)


class Loop:
    def __init__(self, iterator=None):
        self.iterator = iterator
        self.bounds = {'lower': None, 'upper': None, 'stride': None}
        self.body = list()

    def set_bounds(self, lower, upper, stride=1):
        self.bounds['lower'] = lower
        self.bounds['upper'] = upper
        self.bounds['stride'] = stride


class Polytope(Task):
    def __init__(self, fn: Callable) -> None:
        """Representation of the function in an affine space.

        :arg fn: python function.

        related resources:
        ------------------
        https://polyhedral.info/

        https://en.wikipedia.org/wiki/Polytope_model
        https://en.wikipedia.org/wiki/Affine_space
        """

        super().__init__(fn)

        self.ir = defaultdict(lambda: defaultdict())
        # maps hashes of nodes in :function:`fn`'s Python AST to their copy.
        for node in ast.walk(self.tree):
            self.ir[hash(node)] = deepcopy(node)
            setattr(self.ir[hash(node)], 'tiramisu', None)

        fn = self.visit_FunctionDef(self.tree.body[0])

        self.loops = OrderedDict()

    def visit_Assign(self, node) -> Statement:
        target = node.targets
        value = node.value

        s = Statement()

        s.rhs = self.visit(value)

        if 1 < (len(target)):
            raise NotImplementedError(
                "Multi-target assignments are not yet supported.")
        s.lhs = self.visit(target[0])

        return s

    def visit_BinOp(self, node) -> Function:
        fn_name = self.visit(node.op)
        lhs = self.visit(node.left)
        rhs = self.visit(node.right)

        fn = Function(fn_name)
        fn.args = [lhs, rhs]

        return fn

    def visit_Call(self, node: ast.Call) -> Function:
        fn_name = node.func.id
        fn = Function(fn_name)

        for arg in node.args:
            fn.args.append(self.visit(arg))

        for attr in node.keywords:
            val = self.visit(attr.value)
            setattr(fn, attr.arg, val)

        return fn

    def visit_Constant(self, node: ast.Constant) -> [int, str]:
        return node.value

    def visit_For(self, node: ast.For) -> Loop:
        ir_index = hash(node)
        ir_node = self.ir[ir_index]

        loop = Loop(node.target.id)

        if isinstance(ir_node.iter,
                      ast.Call) and 'range' == ir_node.iter.func.id:
            bounds = []
            for arg in node.iter.args:
                try:
                    if arg.id in self.args:
                        bounds.append(arg.id)
                except AttributeError:
                    if isinstance(arg, ast.Constant):
                        bounds.append(arg.n)
                except AttributeError:
                    raise AttributeError(
                        f"Expected '<class ast.Name>' received {type(arg)}")

            if 1 == len(bounds):
                bounds = [0] + bounds

            loop.set_bounds(bounds[0], bounds[1])

        if isinstance(node.iter, ast.List):
            raise TypeError("'list' is not an affine space.")

        for statement in node.body:
            loop.body.append(self.visit(statement))

        return loop

    def visit_FunctionDef(self, node: ast.FunctionDef) -> Function:
        fn = Function(node.name)
        fn.context = DEFINE()

        for statement in node.body:
            fn.body.append(self.visit(statement))

        return fn

    def visit_Name(self, node: ast.Name) -> str:
        return node.id

    def visit_Return(self, node):
        return self.visit(node.value)

    def visit_Subscript(self, node: ast.Subscript) -> tuple:

        ir_index = hash(node)
        ir_node = self.ir[ir_index]

        indices = []

        while isinstance(ir_node, ast.Subscript):
            slice_ = ir_node.slice
            if isinstance(slice_, ast.Index):
                val = self.visit(slice_)
                indices.insert(0, val)
            ir_node = self.visit(ir_node.value)

        buffer_name = ir_node

        buffer = Buffer(buffer_name)
        buffer.indices = indices

        return buffer
