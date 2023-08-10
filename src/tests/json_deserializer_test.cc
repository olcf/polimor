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
#include <array>

#include "../messaging/details/messages.h"
#include "../messaging/details/serializers.h"
#include "../messaging/details/message_json_deserializer_boost_impl.h"
#include "../messaging/details/message_json_serializer_boost_impl.h"


const std::array<std::string, 7> scan_messages = {
    "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr\", \"atime\": 1642662012, \"mtime\": 1642661471, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x200000403:0x295:0x0\" }}",
    "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir1\", \"atime\": 1642662138, \"mtime\": 1642661593, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x280000403:0x21:0x0\" }}",
    "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2\", \"atime\": 1642661656, \"mtime\": 1642661652, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x200000403:0x296:0x0\" }}",
    "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.1\", \"atime\": 1609553580, \"mtime\": 1609553580, \"size\": 10485760, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 3, \"fid\": \"0x200000403:0x297:0x0\" }}",
    "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.2\", \"atime\": 1642661652, \"mtime\": 1642661687, \"size\": 3145728, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 2, \"fid\": \"0x200000403:0x298:0x0\" }}",
    "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/output.txt\", \"atime\": 1633093200, \"mtime\": 1633093200, \"size\": 54272, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 3, \"fid\": \"0x240000403:0xc:0x0\" }}",
    "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/input-data\", \"atime\": 1642661510, \"mtime\": 1592936581, \"size\": 10485760, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 1, \"fid\": \"0x240000403:0xb:0x0\" }}",
};


const std::array<std::string, 7> recorder_messages = {
    "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr\", \"atime\": 1642662012, \"mtime\": 1642661471, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x200000403:0x295:0x0\" }}",
    "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir1\", \"atime\": 1642662138, \"mtime\": 1642661593, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x280000403:0x21:0x0\" }}",
    "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2\", \"atime\": 1642661656, \"mtime\": 1642661652, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x200000403:0x296:0x0\" }}",
    "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.1\", \"atime\": 1609553580, \"mtime\": 1609553580, \"size\": 10485760, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 3, \"fid\": \"0x200000403:0x297:0x0\" }}",
    "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.2\", \"atime\": 1642661652, \"mtime\": 1642661687, \"size\": 3145728, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 2, \"fid\": \"0x200000403:0x298:0x0\" }}",
    "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/output.txt\", \"atime\": 1633093200, \"mtime\": 1633093200, \"size\": 54272, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 3, \"fid\": \"0x240000403:0xc:0x0\" }}",
    "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/input-data\", \"atime\": 1642661510, \"mtime\": 1592936581, \"size\": 10485760, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 1, \"fid\": \"0x240000403:0xb:0x0\" }}",
};


const std::array<std::string, 7> purge_messages = {
    "{ \"path\": \"/lustre/ldev/rmohr\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/subdir1\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/subdir2\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.1\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.2\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/output.txt\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/input-data\" }",
};

const std::array<std::string, 7> migration_messages = {
    "{ \"path\": \"/lustre/ldev/rmohr\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/subdir1\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/subdir2\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.1\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.2\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/output.txt\" }",
    "{ \"path\": \"/lustre/ldev/rmohr/dir1/input-data\" }",
};


void test_scan_message() {

    char buffer[4096];

    for(auto const &scan_record : scan_messages) {
        
        auto const &msg = json_deserializer_impl<scan_message>()(scan_record);

        auto res1 = json_serializer_impl<scan_message>()(msg);
        auto res2 = json_serializer_impl<scan_message>()(msg, {buffer, sizeof(buffer) });

        assert(scan_record.compare(res1) == 0);
        assert(scan_record.compare(0, scan_record.length(), res2, 0, res2.length()-1) == 0);
    }
}

void test_recorder_message() {

    char buffer[4096];

    for(auto const &recorder_record : recorder_messages) {
           
        auto const &msg = json_deserializer_impl<recorder_message>()(recorder_record);

        auto res1 = json_serializer_impl<recorder_message>()(msg);
        auto res2 = json_serializer_impl<recorder_message>()(msg, {buffer, sizeof(buffer)});
        
        assert(recorder_record.compare(res1) == 0);
        assert(recorder_record.compare(0, recorder_record.length(), res2, 0, res2.length()-1) == 0);
    }
}

void test_purge_message() {

    char buffer[4096];

    for(auto const &purge_record : purge_messages) {
        
        auto const &msg = json_deserializer_impl<purge_message>()(purge_record);

        auto res1 = json_serializer_impl<purge_message>()(msg);
        auto res2 = json_serializer_impl<purge_message>()(msg, {buffer, sizeof(buffer)});

        assert(purge_record.compare(res1) == 0);
        assert(purge_record.compare(0, purge_record.length(), res2, 0, res2.length()-1) == 0);
    }
}

void test_migration_message() {

    char buffer[4096];

    for(auto const &migration_record : migration_messages) {
        
        auto const &msg = json_deserializer_impl<migration_message>()(migration_record);

        auto res1 = json_serializer_impl<migration_message>()(msg);
        auto res2 = json_serializer_impl<migration_message>()(msg, {buffer, sizeof(buffer)});

        assert(migration_record.compare(res1) == 0);
        assert(migration_record.compare(0, migration_record.length(), res2, 0, res2.length()-1) == 0);
    }
}

int main(int argc, const char* argv[]) {


    test_scan_message();
    test_recorder_message();
    test_purge_message();
    test_migration_message();

    return EXIT_SUCCESS;
}