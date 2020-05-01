#  Copyright (c) 2019-2020 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
from pytiramisu import init_physl, var, expr, function, primitive_t, argument_t, buffer, computation, codegen_physl, p_uint8, a_output

if __name__ == "__main__":

    '''
    '''

    init_physl("function0")
    i = var("i", expr(0), expr(10))
    S0 = computation("S0", list([i,]), expr(3) + expr(4))
    S0.parallelize(i)
    buf0 = buffer("buf0", list([expr(10),]), p_uint8, a_output)
    S0.store_in(buf0)

    physl_str = codegen_physl([buf0, ])
    print(physl_str)
