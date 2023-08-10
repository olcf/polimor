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
#include <regex>
#include <string>


std::string examples[] = {
    "{ record: \"metadata\", type: \"d\", path: \"/lustre/ldev/rmohr\", atime: \"1642662012\", mtime: \"1642661471\", size: \"4096\", uid: \"6598\", gid: \"9294\", format: { filesys: \"lustre\", ost_pool: \"\", stripe_count: \"0\", fid: \"0x200000403:0x295:0x0\" } }",
    "{ record: \"metadata\", type: \"d\", path: \"/lustre/ldev/rmohr/dir1\", atime: \"1642661534\", mtime: \"1642661594\", size: \"4096\", uid: \"6598\", gid: \"9294\", format: { filesys: \"lustre\", ost_pool: \"\", stripe_count: \"0\", fid: \"0x240000403:0xa:0x0\" } }",
    "{ record: \"metadata\", type: \"d\", path: \"/lustre/ldev/rmohr/dir1/subdir1\", atime: \"1642662138\", mtime: \"1642661593\", size: \"4096\", uid: \"6598\", gid: \"9294\", format: { filesys: \"lustre\", ost_pool: \"\", stripe_count: \"0\", fid: \"0x280000403:0x21:0x0\" } }",
    "{ record: \"metadata\", type: \"d\", path: \"/lustre/ldev/rmohr/dir1/subdir2\", atime: \"1642661656\", mtime: \"1642661652\", size: \"4096\", uid: \"6598\", gid: \"9294\", format: { filesys: \"lustre\", ost_pool: \"\", stripe_count: \"0\", fid: \"0x200000403:0x296:0x0\" } }",
    "{ record: \"metadata\", type: \"f\", path: \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.1\", atime: \"1609553580\", mtime: \"1609553580\", size: \"10485760\", uid: \"6598\", gid: \"9294\", format: { filesys: \"lustre\", ost_pool: \"\", stripe_count: \"3\", fid: \"0x200000403:0x297:0x0\" } }",
    "{ record: \"metadata\", type: \"f\", path: \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.2\", atime: \"1642661652\", mtime: \"1642661687\", size: \"3145728\", uid: \"6598\", gid: \"9294\", format: { filesys: \"lustre\", ost_pool: \"\", stripe_count: \"2\", fid: \"0x200000403:0x298:0x0\" } }",
    "{ record: \"metadata\", type: \"f\", path: \"/lustre/ldev/rmohr/dir1/output.txt\", atime: \"1633093200\", mtime: \"1633093200\", size: \"54272\", uid: \"6598\", gid: \"9294\", format: { filesys: \"lustre\", ost_pool: \"\", stripe_count: \"3\", fid: \"0x240000403:0xc:0x0\" } }",
    "{ record: \"metadata\", type: \"f\", path: \"/lustre/ldev/rmohr/dir1/input-data\", atime: \"1642661510\", mtime: \"1592936581\", size: \"10485760\", uid: \"6598\", gid: \"9294\", format: { filesys: \"lustre\", ost_pool: \"\", stripe_count: \"1\", fid: \"0x240000403:0xb:0x0\" } }",
    };

int main(int argc, const char* argv[]) {

    
    std::regex json("^\\s*\\{" 
      
        "\\s*record:\\s*\"metadata\"," 
        "\\s*type:\\s*\"(d|f)\"," 
        "\\s*path:\\s*\"([^\"]+)\","
        "\\s*atime:\\s*\"(\\d+)\"," 
        "\\s*mtime:\\s*\"(\\d+)\","
        "\\s*size:\\s*\"(\\d+)\","
        "\\s*uid:\\s*\"(\\d+)\"," 
        "\\s*gid:\\s*\"(\\d+)\","
        "\\s*format:\\s*\\{" 
            "\\s*filesys: \"([^\"]+)\","
            "\\s*ost_pool: \"([^\"]*)\","
            "\\s*stripe_count: \"(\\d+)\"," 
            "\\s*fid: \"([^\"]+)\",?"
        "\\s*\\},?"
        "\\s*\\}\\s*$", std::regex_constants::ECMAScript); //"\\s+\\{.*}\\s+");//, std::regex_constants::ECMAScript);

    std::smatch match;

    std::regex_match(examples[0], match, json);

constexpr int match_type_pos = 1;
constexpr int match_path_pos = 2;
constexpr int match_atime_pos = 3;
constexpr int match_mtime_pos = 4;
constexpr int match_size_pos = 5;
constexpr int match_uid_pos = 6;
constexpr int match_gid_pos = 7;
constexpr int match_fsname_pos = 8;
constexpr int match_ost_pool_pos = 9;
constexpr int match_stripe_cout_pos = 10;
constexpr int match_fid_pos = 11;

    std::clog << match.size() << " " << match[1] << std::endl;

    for(auto &field : match) {
        std::clog << field << std::endl;
    }

    return EXIT_SUCCESS;
}