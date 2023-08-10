/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <chrono>
#include <string_view>
#include <string>
#include <iostream>


/* Top level message class that exists currently just for tagging the message
 * types for concepts. */
struct message_tag { };

/* Used to check if a template is a message type */
template<typename MsgImpl>
concept IsMsg = std::derived_from<MsgImpl, message_tag>;


struct scan_message : public message_tag {

    scan_message() : size(0), uid(0), gid(0), stripe_count(0), type(0) { }

    scan_message(const scan_message& other) = default;
    scan_message(scan_message&& other) = default;

    scan_message& operator=(const scan_message& other) = default;
    scan_message& operator=(scan_message&& other) = default;
    
    ~scan_message() = default;

    /* Properties */
    std::chrono::time_point<std::chrono::system_clock> atime; 
    std::chrono::time_point<std::chrono::system_clock> mtime;
   
    std::uint64_t size;
    std::uint64_t uid;
    std::uint64_t gid;
    std::uint64_t stripe_count;

    std::string filesys;
    std::string path;
    std::string ost_pool;
    std::string fid;

    char type;
};


struct purge_message : public message_tag {

    
    purge_message() = default;
    purge_message(std::string_view sv) : path(sv) { }

    purge_message(const purge_message&) = default;
    purge_message(purge_message &&) = default;
    
    purge_message &operator=(const purge_message &o) = default;
    purge_message &operator=(purge_message &&o) = default;

    ~purge_message() = default;

    /* Properties */ 
    std::string path;
};


struct migration_message : public message_tag {

    
    migration_message() = default;
    migration_message(std::string_view sv) : path(sv) { }

    migration_message(const migration_message&) = default;
    migration_message(migration_message &&) = default;
    
    migration_message &operator=(const migration_message &o) = default;
    migration_message &operator=(migration_message &&o) = default;

    ~migration_message() = default;

    /* Properties */ 
    std::string path;
};

struct recorder_message : public message_tag {


    recorder_message(): size(0), uid(0), gid(0), 
                        stripe_count(0), type(0) { }

    recorder_message(const recorder_message&) = default;
    recorder_message(recorder_message &&) = default;
    
    recorder_message &operator=(const recorder_message &o) = default;
    recorder_message &operator=(recorder_message &&o) = default;

    ~recorder_message() = default;

    /* Properties */ 
    std::chrono::time_point<std::chrono::system_clock>  atime;
    std::chrono::time_point<std::chrono::system_clock>  mtime;
    std::uint64_t size;
    std::uint64_t uid;
    std::uint64_t gid;
    std::uint64_t stripe_count;

    std::string filesys;
    std::string path;
    std::string ost_pool;
    std::string fid;

    char type;
};





#endif // __MESSAGES_H__