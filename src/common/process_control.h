/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __PROCESS_CONTROL_H__
#define __PROCESS_CONTROL_H__

#include <algorithm>
#include <bits/ranges_base.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unistd.h>
#include <fcntl.h>
#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>
#include <ext/stdio_filebuf.h>
#include <utility>
#include <vector>
#include <array>
#include <ranges>

class process {

    private:
        pid_t _pid;
        int _pipefds[2];
        std::vector<std::string> _args;

        void _cleanup() {

            if(_pipefds[0] != -1) {
                close(_pipefds[0]);
                _pipefds[0] = -1; 
            }

            if(_pipefds[1] != -1) {
                close(_pipefds[1]);
                _pipefds[1] = -1;
            }

            if(_pid != -1) {
                kill(_pid, SIGKILL);
                waitpid(_pid, nullptr, 0);
                _pid = -1;
            }
        }

    public:

        /* TODO add concept restaint to restrict args to strings */
        template<typename... Args>
        process(const std::string &executable, Args&... args) : 
            _args{ executable, std::forward<Args>(args)...}, 
            _pid(-1), _pipefds{-1, -1} { };
        
        template<typename list>
        requires std::ranges::range<list>
        process(const list &args)  {
            std::ranges::copy(args, std::begin(_args));
        }


        ~process() {
            this->_cleanup();
        }


        std::basic_filebuf<char> launch() {

            this->_cleanup();

            try {
                const char *argv[_args.size()+1];

                int rc = pipe2(_pipefds, O_DIRECT|O_CLOEXEC);

                if(rc != 0) {
                    throw std::system_error(errno, std::generic_category(), "Error creating pipe");
                }

                
                pid_t tmp_pid = -1;

                posix_spawn_file_actions_t file_actions;
            
                posix_spawn_file_actions_init(&file_actions);
            
                posix_spawn_file_actions_adddup2(&file_actions, _pipefds[1], STDOUT_FILENO);
                posix_spawn_file_actions_adddup2(&file_actions, _pipefds[1], STDERR_FILENO);

                /* Build the array of char * */
                for(int i=0; i<_args.size(); ++i) {
                    argv[i] = _args[i].c_str();
                }

                argv[_args.size()] = nullptr;
               
                /* Spawn the subprocess */
                rc = posix_spawn(&tmp_pid, 
                                argv[0], 
                                &file_actions, 
                                nullptr, 
                                const_cast<char *const *>(argv), 
                                nullptr);

                switch(rc) {

                    case 0:
                        this->_pid = tmp_pid;
                        break;

                    default:
                        throw std::system_error(errno, 
                                                std::generic_category(), 
                                                "Error launching process");
                }

                posix_spawn_file_actions_destroy(&file_actions);
             
                close(_pipefds[1]); 
                _pipefds[1] = -1;

                /* Create a filebuf wrapper for the input end of the pipe and
                 * return it */
                __gnu_cxx::stdio_filebuf<char> filebuf(_pipefds[0], 
                                                       std::ios::in);

                /* Reset the input pipefd since the fd will be closed when
                 * filebuf is reclaimed */
                _pipefds[0] = -1;

                return std::move(filebuf);

            } catch(const std::system_error &e) {
        
                this->_cleanup();
                throw;
            } 
        }

        int wait() {

            if(this->_pid == -1) {
                throw std::runtime_error("No process available");
            }

            int status;

            int rc = waitpid(this->_pid, &status, 0);

            if(rc != this->_pid) {
                throw std::system_error(errno, 
                                        std::generic_category(), 
                                        "Error waiting on process");
            }

            return WEXITSTATUS(status);
        }

        void stop() {

            if(this->_pid == -1) {
                throw std::runtime_error("No process available");
            }

            kill(this->_pid, SIGKILL);

            this->_cleanup();
        }
};


#endif // __PROCESS_CONTROL_H__