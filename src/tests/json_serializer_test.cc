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
#include <string>
#include <iostream>

#include "../messaging/messages.h"
#include "../messaging/serializers.h"


std::string scan_record = 
    "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr\", \"atime\": 1642662012, \"mtime\": 1642661471, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x200000403:0x295:0x0\" } }";


int main(int argc, const char* argv[]) {

    
   
    auto msg =  json_deserializer_impl<scan_message>()(scan_record);


    std::clog << "done" << std::endl;


    return EXIT_SUCCESS;
}