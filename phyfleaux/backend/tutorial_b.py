#  Copyright (c) 2019-2020 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
from pytiramisu import init_physl, var, expr, constant, function, primitive_t, argument_t, buffer, computation, codegen_physl, p_uint8, a_output, a_input, input, cast, p_int32, int32_expr, uint8_expr

if __name__ == "__main__":

    NN = int32_expr(10)
    MM = int32_expr(20)

    i = var("i", int32_expr(0), NN)
    j = var("j", int32_expr(0), MM)

    init_physl("tut_02")

    A = input("A", list(["i", "j"]), list([NN, MM]), p_uint8)

    output = computation("output", list([i, j]), A(i, j) + cast(p_uint8, i) + uint8_expr(4))

    i0 = var("i0")
    i1 = var("i1")
    j0 = var("j0")
    j1 = var("j1")

    output.tile(i, j, 2, 2, i0, j0, i1, j1)
    output.parallelize(i0)

    b_A = buffer("b_A", list([NN, MM]), p_uint8, a_input)
    b_output = buffer("b_output", list([NN, MM]), p_uint8, a_output)

    A.store_in(b_A)
    output.store_in(b_output)

    physl_str = codegen_physl(list([b_A, b_output]))
    print(physl_str)
