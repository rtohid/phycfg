<!-- 
Copyright (c) 2020 R. Tohid

Distributed under the Boost Software License, Version 1.0.(See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) 
-->

# Phyfleaux: Context-Aware Scheduler for Distributed Programs

## About

Phyfleaux is [Phylanx](github.com/stellar-group/phylanx) scheduler. Given a
Python program, Phyfleaux analyze the code and generate optimized distributed
PhySL. Phyfleaux will also support submission to various science gateways
through [JetLag](https://github.com/STEllAR-GROUP/JetLag.git).

## Problem

### Input
Code is the input. Here, code is a valid Python program encapsulated in a
function. Therefore:

* Valid implementation of a sequential algorithm exists.
* Code is already checked for syntax and grammar errors.
* Indexing of all possible iteration spaces are known, either:

   1.parametrically, e.g., affine iteration spaces which are commonly occur in

      data science and linear algebra in general

   2.stored as data, e.g., indirect memory accesses.
   * In the first case, the more computation power the higher throughput.
   * for the second, more memory, higher throughput.

* In sequential implementation, memory access patterns of both data consumed by
  the application, and that used for indexing of iteration spaces are known.
   <!-- * It might be beneficial to assume the initial memory layout in 1-d.-->

#### Description

Given a valid Python code and specification of the system, generate PhySL
distributed program, where:

* Each PhySL primitive can run on one or more nodes.
* There is a cost function associated to each PhySL function, a.k.a., Task.

associate a (or a range of) time-step(s) and target-node(s) to each task so the
cost function is minimized.

## Notes

* Cost could be minimizing execution time or maximizing throughput.
* Ideally, the cost function returns outputs of a NN.

## Solution

### Implementation

* Within the framework, code is accessible through its python AST.
* Several common iteration spaces, e.g., affine, structured and unstructured
  mesh, tree, ... will be supported as data domains.  
  **Note** (FleCSI
  [topologies](https://github.com/laristra/flecsi/tree/1/flecsi/topology) might
  be relevant here.).

### TODO
#### polyhedral
1. initialize IR.
   - ir is initialized as a map from hash of an AST to a copy of the same AST.
   - in implementation, ir is a dictionary with hashes as keys and copies as
     values.
2. find iteration spaces in the code (for now just `for` loops.)
   - find the iteration boundaries (upper- and lower- bounds)
   - find data access patterns of all arrays.
