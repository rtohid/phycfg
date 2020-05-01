<!-- 
Copyright (c) 2020 R. Tohid

Distributed under the Boost Software License, Version 1.0.(See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) 
-->

# Tiramisu

## Buffer
A class that represents buffers.

Buffers have two use cases:
- store the results of computations, and
- represent input arguments to functions.


## Computation
A class that represents computations.

A computation is an expression associated with an iteration domain. A computation indicates what needs to be computed (the expression
that should be computed).

A computation has three representations:

### Level I
this level specifies "what" should be computed but does not specify "when" 
(order) and "where" (on which processor) each expression is computed.
This level also does not specify where computations should be stored in memory
nor the data layout.

### Level II
this level specifies "when" and "where" a compuation should be operated.

### Level III
this level is similar to Level 2 but it specifies where computations should be
stored in memory and also the corresponding data layout.


## Constant
A class that represents loop invariants.

An object of the invariant class can be an expression, a symbolic constant
or a variable that is invariant to all the loops of the function.

## Function
A function is composed of a set of computations.