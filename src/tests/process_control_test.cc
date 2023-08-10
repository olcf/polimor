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
#include <fstream>
#include <string>

#include "../common/process_control.h"


int main(int argc, const char* argv[]) {


    process p("/bin/echo", "hello", "world");   

    std::basic_filebuf<char> filebuf = p.launch();

    /* Read line by line until there is no more data */
    for(std::string buffer; std::getline(std::istream(&filebuf), buffer);) {
        std::clog << "output: " << buffer << std::endl;
    }

    p.stop(); 

    return EXIT_SUCCESS;
}