#  Copyright (c) 2019-2020 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
from pytiramisu import init_physl, var, expr, function, primitive_t, argument_t, buffer, computation, codegen_physl

if __name__ == "__main__":

    '''
    the goal of pytiramisu is to make the following c++ sample code function in python

    // C++ code with a Tiramisu expression.
    #include "tiramisu/tiramisu.h"
    using namespace tiramisu;

    void generate_code()
    {
        // Specify the name of the function that you want to create.
        tiramisu::init("foo");

        // Declare two iterator variables (i and j) such that 0<=i<100 and 0<=j<100.
        var i("i", 0, 100), j("j", 0, 100);

        // Declare a Tiramisu expression (algorithm) that is equivalent to the following C code
        // for (i=0; i<100; i++)
        //   for (j=0; j<100; j++)
        //     C(i,j) = 0;
        computation C({i,j}, 0);
    
        // Specify optimizations
        C.parallelize(i);
        C.vectorize(j, 4);
    
        buffer b_C("b_C", {100, 100}, p_int32, a_output);
        C.store_in(&b_C);

        // Generate code
        C.codegen({&b_C}, "generated_code.o"); // NOTE: this is no longer correct
    }
    '''

    init_physl("foo")
    srange_expr = expr(0) 
    erange_expr = expr(100) 
    i = var("i", srange_expr, erange_expr)
    j = var("j", srange_expr, erange_expr)
    iter_range = [i, j]
    crange_expr = expr(0)
    C = computation(iter_range, crange_expr)
    C.parallelize(i)
    C.vectorize(j, 4)

    buffers = [ C.get_buffer(), ]
    physl_str = codegen_physl(buffers)
    print(physl_str)
