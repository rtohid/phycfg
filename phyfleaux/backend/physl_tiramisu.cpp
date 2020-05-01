//  Copyright (c) 2019-2020 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <memory>

#include <isl/ast_type.h>

#include "physl_tiramisu.hpp"

#include <tiramisu/debug.h>
#include <tiramisu/expr.h>
#include <tiramisu/type.h>
#include <tiramisu/computation_graph.h>
#include <tiramisu/core.h>

using namespace tiramisu;

namespace physl { namespace tiramisu {

// taken from `tiramisu_codegen_halide.cpp:2549`
//
//static inline isl_ast_node *for_code_generator_after_for(isl_ast_node *node, isl_ast_build *build, void *user) {
//    return node;
//}

//int codegen(const std::vector< std::shared_ptr<buffer> > &arguments, const bool gen_cuda_stmt, std::string & physlstr)
//
int codegen(std::vector< buffer > &arguments, std::string & physlstr)
{

    std::vector< buffer * > bufs{};

    if(arguments.size()) {
        bufs.reserve(arguments.size());

        for(auto & arg : arguments) {
            //bufs.push_back(&arg);
            bufs.emplace_back( std::addressof(arg) );
        }
    }

    { // begin scope

        PhyslFunction * fct = static_cast<PhyslFunction *>(global::get_implicit_function());
        /*
        if(gen_cuda_stmt)
        {
            if(!fct.get_mapping().empty())
            {
                tiramisu::computation* c1 = fct->get_first_cpt();
                tiramisu::computation* c2 = fct->get_last_cpt();
                Automatic_communication(c1,c2);
            }
            else {
                DEBUG(3, tiramisu::str_dump("You must specify the corresponding CPU buffer to each GPU buffer else you should do the communication manually"));
            }
        }
        */

        fct->generate_code(bufs, physlstr);

        /*
        if (gen_cuda_stmt) {
            fct.gen_cuda_stmt();
        }

        fct.gen_halide_stmt();
        fct->gen_halide_obj(obj_filename);
        */

    } // end scope

    return 1;

} // end codegen

} /* end namespace tiramisu */ } // end namespace physl
