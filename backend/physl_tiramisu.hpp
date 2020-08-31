//  Copyright (c) 2019-2020 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef __PHYSL_TIRAMISU__
#define __PHYSL_TIRAMISU__

#include <string>
#include <vector>
#include <memory>

#include <tiramisu/tiramisu.h>

#include "physl_isl.hpp"

using namespace tiramisu;

namespace physl { namespace tiramisu {

class PhyslFunction : public function, generator {

    public:
    PhyslFunction() : function("physl_codegen"), generator() {
    }

    PhyslFunction(std::string & name) : function(name), generator() {
    }

    void lift_dist_computations() {
        lift_dist_comps();
    }

    void generate_code(std::vector< buffer* > & bufs, std::string &physlstr) {
        set_arguments(bufs);
        lift_dist_computations();
        gen_time_space_domain();
        gen_isl_ast();
        physl::codegen::generate_physl(
            this->get_isl_ctx(),
            this->get_isl_ast(),
            physlstr
        );
    }
};

int codegen(std::vector< buffer > &arguments, std::string & physlstr);

} /* end namespace tiramisu */ } // end namespace physl

static inline std::string codegen_physl(std::vector< buffer > &arguments) {
    std::string output{};
    physl::tiramisu::codegen(arguments, output);
    return output;
}

#endif
