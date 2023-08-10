/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/



#include <cstdlib>
#include <iostream>
#include <exception>

#include "../common/qs_exception.h"

void func3() {
    throw qs_exception("Error in func3");
}

void func2() {
    
    try {
        
        func3();

    } catch(...) {
        std::throw_with_nested(qs_exception("Error in func2"));
    }
}

void func1() {
    
    try {
     
        func2();

    } catch(...) {
         std::throw_with_nested(qs_exception("Error in func1"));
    }
}

void print_exception(std::exception_ptr eptr = std::current_exception()) {

    /* Get the current top level exception if there is one */
    
    /* Exception is present */
    
}

int main(int argc, char *argv[]) {

    std::set_terminate([](){
        std::clog << "Except caught" << std::endl;

        print_exception();

        std::clog << "done" << std::endl;
    });

    func1();

    return EXIT_SUCCESS;
}