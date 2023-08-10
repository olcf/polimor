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
#include <cstdint>
#include <nats/status.h>
#include <string>
#include <string_view>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <memory>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <vector>
#include <coroutine>
#include <cerrno>

#include <unistd.h>
#include <sys/epoll.h>

#include "nats/nats.h"


class nats_message_queue_publisher {

    friend class nats_messaging_service;

    private:
        std::string      _name;
        std::shared_ptr<natsConnection> _conn_ptr= nullptr;
       
        nats_message_queue_publisher(std::string_view name, 
                                     std::shared_ptr<natsConnection> conn_ptr): 
            _name(name), _conn_ptr(conn_ptr) { }

    public:
        explicit nats_message_queue_publisher() = delete;

        nats_message_queue_publisher(const nats_message_queue_publisher &) = delete;
        nats_message_queue_publisher(nats_message_queue_publisher &&) = delete;
        nats_message_queue_publisher &operator=(const nats_message_queue_publisher &) = delete;
        nats_message_queue_publisher &operator=(nats_message_queue_publisher &) = delete;

        ~nats_message_queue_publisher() = default;


        void send(std::string_view message) {

            natsStatus status = natsConnection_Publish(_conn_ptr.get(), 
                                                       _name.c_str(), 
                                                       message.data(), 
                                                       message.length());

            if(status != NATS_OK) {
                throw std::runtime_error(natsStatus_GetText(status));
            }
        }
};


class nats_message_queue_subscriber {

    friend class nats_messaging_service;

    private:
        std::string      _name;
        natsSubscription *_sub  = nullptr;

        nats_message_queue_subscriber(std::string_view name, 
                                      natsSubscription *sub): 
            _name(name), _sub(sub) { }

    public:
        explicit nats_message_queue_subscriber() = delete;

        nats_message_queue_subscriber(const nats_message_queue_subscriber &) = delete;
        nats_message_queue_subscriber(nats_message_queue_subscriber &&) = delete;
        nats_message_queue_subscriber &operator=(const nats_message_queue_subscriber &) = delete;
        nats_message_queue_subscriber &operator=(nats_message_queue_subscriber &) = delete;

        ~nats_message_queue_subscriber() {
            if(_sub != nullptr) {
                natsSubscription_Destroy(std::exchange(_sub, nullptr));
            }
        }

        std::string_view receive(char *buf, size_t size) {

            natsMsg *msg = nullptr;

            natsStatus status = natsSubscription_NextMsg(&msg, _sub, INT64_MAX);

            if(status != NATS_OK) {
                throw std::runtime_error(natsStatus_GetText(status));
            }            

            std::unique_ptr<natsMsg, void (*)(natsMsg *)> msg_ptr(msg, natsMsg_Destroy);
    
            assert(sizeof(size_t) >= sizeof(int));

            size_t msg_size = natsMsg_GetDataLength(msg_ptr.get());

            if(msg_size > size) {
                throw std::runtime_error("Buffer too small");
            }

            std::copy_n(natsMsg_GetData(msg_ptr.get()), msg_size, buf);

            
            return { buf, static_cast<std::string_view::size_type>(msg_size) };
        }
};



struct scheduler {
    int      _epfd;
    natsSock _socket;
  
    scheduler(int epfd, natsSock socket=-1)  : _epfd(epfd), _socket(socket) { }

    ~scheduler() {
        if(_epfd > 0) {
            close(_epfd);
        }
    }
};

class nats_messaging_service {


    friend nats_messaging_service *create_messaging_service(
                                    const std::string_view);
    
    private:   

        /* TODO need to use smart pointers to track either conn or 
         * the messaging service */
        std::shared_ptr<natsConnection> _conn_ptr;
        std::shared_ptr<scheduler> _scheduler_ptr;

        explicit nats_messaging_service(std::unique_ptr<natsConnection, void(*)(natsConnection*)> conn_ptr, std::unique_ptr<scheduler> scheduler_ptr) : 
            _conn_ptr(std::move(conn_ptr)), _scheduler_ptr(std::move(scheduler_ptr)) { }


    public:

        nats_messaging_service(const nats_messaging_service &) = delete;
        nats_messaging_service(nats_messaging_service &&) = delete;

        nats_messaging_service &operator=(const nats_messaging_service &) = delete;
        nats_messaging_service &operator=(nats_messaging_service &&) = delete;
            

        virtual ~nats_messaging_service() {

            // if(_conn != nullptr) {
            //     natsConnection_Destroy(std::exchange(_conn,  nullptr));
            // }
        }

        
        nats_message_queue_publisher *create_queue_publisher(const std::string_view name) {

            return new nats_message_queue_publisher(name, _conn_ptr);
        }
        

        nats_message_queue_subscriber *create_queue_subscriber(const std::string_view name) {

            natsSubscription *subscription = nullptr;
                
            natsStatus status = natsConnection_QueueSubscribeSync(&subscription, 
                                                                  _conn_ptr.get(), 
                                                                  name.data(), 
                                                                  name.data());

            if(status != NATS_OK) {
                throw std::runtime_error(natsStatus_GetText(status));
            }

            return new nats_message_queue_subscriber(name, subscription);
        }   
};


/**
 * @brief Callback initiated when NATS is connected or reconnected to a server.
 * 
 * This callback is invoked after NATS has connected or reconnected to a NATS server. 
 * 
 * @param userData On initial connects this is filled in by this function and 
 *                 used by other callbacks.
 * @param loop 
 * @param nc natsConnection object.
 * @param socket Socket ready for use.
 * @return natsStatus NATS_OK for success and something else for failure.
 */
static natsStatus connection_attach(void **userData, 
                                    void *loop, 
                                    natsConnection *nc, 
                                    natsSock socket) {

    std::clog << __FUNCTION__ << std::endl;

    assert(sizeof(natsSock) == sizeof(int));

    auto scheduler = static_cast<struct scheduler *>(loop);

    scheduler->_socket = socket;

    *userData = static_cast<void *>(scheduler);

    /* Set up socket for listening for read and write events */
    struct epoll_event event = { .events = EPOLLIN|EPOLLOUT,
                                 .data = { .ptr = nullptr  }};

    int rc = epoll_ctl(scheduler->_epfd, EPOLL_CTL_ADD, scheduler->_socket, &event);


    if(rc != 0) {
        //throw std::system_error(errno, std::generic_category(), std::string("Error setting up epoll at ")+__FUNCTION__ + ":" + std::to_string(__LINE__));
        return NATS_IO_ERROR;
    }
    
    /* Success */
    return NATS_OK;
}

/**
 * @brief Callback initiated when the nats socket should either be added or 
 *        removed from read polling.
 * 
 * @param userData callback data previous created in connection_attach. 
 * @param add True when to start polling the socket and False when to stop.
 * @return natsStatus NATS_OK for success and something else for failure.
 */
static natsStatus connection_read_add_remove_poll(void *userData, bool add) {

    std::clog << __FUNCTION__ << " " << "Add = " << add << std::endl;



    return NATS_OK;
}


/**
 * @brief Callback initiated when the nats socket should either be added or removed from write polling.
 * 
 * @param userData callback data previous created in connection_attach. 
 * @param add True when to start polling the socket and False when to stop.
 * @return natsStatus NATS_OK for success and something else for failure.
 */
static natsStatus connection_write_add_remove_poll(void *userData, bool add) {


    std::clog << __FUNCTION__ << " " << "Add = " << add << std::endl;


    return NATS_OK;
}


/**
 * @brief Callback initiated when the connection is closed and resources can
 *        be reclaimed.
 * 
 * @param userData callback data previous created in connection_attach. 
 * @return natsStatus NATS_OK for success and something else for failure.
 */
static natsStatus connection_detach(void *userData) {

    std::clog << __FUNCTION__ << std::endl;

    auto scheduler = reinterpret_cast<struct scheduler *>(userData);

    /* Remove the epfd from the scheduler */

    int rc = epoll_ctl(scheduler->_epfd, EPOLL_CTL_DEL, scheduler->_socket, nullptr);


    if(rc != 0) {
        return NATS_IO_ERROR;
    }

    return NATS_OK;
}


nats_messaging_service *create_messaging_service(
    std::string_view urls) {


    natsStatus status;
    natsOptions *nats_opts = nullptr;
    natsConnection *conn = nullptr;

    std::vector<char *> server_urls;

    char url_buf[urls.length()+1];
    url_buf[urls.length()]='\0';

    
    /* Create the nats options */
    status = natsOptions_Create(&nats_opts);

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("here1") +natsStatus_GetText(status));
    }

    /* Manage the lifetime of the natOptions with this unique_ptr */
    std::unique_ptr<natsOptions, void(*)(natsOptions *)> nats_opts_ptr(nats_opts, [](natsOptions *nats_opts) {natsOptions_Destroy(nats_opts);});


    /* Build a list of servers */
    std::replace_copy(const_cast<char *>(urls.begin()), const_cast<char *>(urls.end()), url_buf, ',', '\0');

    for(auto next_it = url_buf, it = url_buf; 
        it < url_buf+sizeof(url_buf); 
        ++it) {
        
        server_urls.push_back(it);

        it = std::find(it, url_buf+sizeof(url_buf), '\0');
    }

    for(auto s: server_urls) {
        std::clog << s << std::endl;
    }


    /* Assign the servers */
    status = natsOptions_SetServers(
                        nats_opts_ptr.get(),
                        const_cast<const char**>(server_urls.data()), 
                        server_urls.size());

    if(status != NATS_OK) {
        throw std::runtime_error(natsStatus_GetText(status));
    }


    

    /* Create the epoll file descriptor */
    int epfd = epoll_create1(0);
    
    if(epfd <= 0) {
        throw std::system_error(errno, std::generic_category(), 
                          std::string("Error creating epoll file descriptor at " __FILE__ ":")+std::to_string(__LINE__));
    }
    


    auto scheduler_ptr = std::make_unique<struct scheduler>(epfd);
    

    /* Setup the callbacks for monitoring the NATS socket */
    status = natsOptions_SetEventLoop(nats_opts_ptr.get(), 
                                      scheduler_ptr.get(),
                                      connection_attach,
                                      connection_read_add_remove_poll,
                                      connection_write_add_remove_poll,
                                      connection_detach);

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("here2") +natsStatus_GetText(status));
    }

    /* Connect to a NATS server */
    status = natsConnection_Connect(&conn, nats_opts_ptr.get());

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("here3") +natsStatus_GetText(status));
    }

    std::unique_ptr<natsConnection, void(*)(natsConnection *)> nats_conn_ptr(conn, [](natsConnection *conn) {natsConnection_Close(conn); natsConnection_Destroy(conn);});


    return new nats_messaging_service(std::move(nats_conn_ptr), std::move(scheduler_ptr));
}


nats_messaging_service *create_messaging_service() {

    return create_messaging_service(std::string_view(NATS_DEFAULT_URL));
}



void get_message() {

    

}

template<typename PromiseType = void>
struct Awaitable {


    PromiseType *_promise = nullptr;



    /* Returns true if the coroutine can continue without suspension or false
     * if the coroutine needs suspension (returning true allows for optimization
     * to prevent creation of coroutine for await_suspend) */
    bool await_ready() const noexcept {

        return false;
    }

    /* Suspends the current coroutine
     * Three return types:  void - suspends the coroutine, 
     *                      bool - true: suspend the coroutine, 
     *                             false: do not suspend the coroutine,
     *                      std::coroutine_handle<> - Next coroutine to resume */
    void await_suspend(std::coroutine_handle<PromiseType> h) noexcept {

        this->_promise = &h.promise();
    }
    
    /* This returns void for the return type of the function */
    PromiseType *await_resume() const noexcept {

        return this->_promise;
    }

};

struct ReturnObject {

    struct promise_type {

        int _val = 20;

        promise_type() = default;

        /* Return the parent */
        ReturnObject get_return_object() { 
            
            std::clog << __FUNCTION__ << std::endl;
            return { std::coroutine_handle<promise_type>::from_promise(*this) }; 
        };

        /* Do not suspend the coroutine initially */
        std::suspend_never initial_suspend() { 
            
            std::clog << __FUNCTION__ << std::endl;
            return {}; 
        };

        /* Always suspend the coroutine at the end before cleanup */
        std::suspend_always final_suspend() noexcept { 

            std::clog << __FUNCTION__ << std::endl;    
            return {}; 
        };

        /* Handles exceptions that occurs while processing the coroutine */
        void unhandled_exception() {  

            std::clog << __FUNCTION__ << std::endl;    
        };

        /* Gets the corountine handle */
        auto get_handle() {
            return std::coroutine_handle<promise_type>::from_promise(*this);
        }

    };

    /* The handle for the coroutine */
    std::coroutine_handle<promise_type> _handle;

    /* Constructor for ReturnObject that is called from get_return_object with 
     * the coroutine handle */
    ReturnObject(std::coroutine_handle<promise_type> handle): _handle(handle) { }

    /* Converts the return object to it's coroutine handle */
    operator std::coroutine_handle<promise_type>() const { return _handle; }
};

ReturnObject run() {
    
// struct CoroutineFrame {
//
//   ReturnObject::promise_type promise;
//   bool initial_await_resume_called = false;
//   int state = 0;
//     
//   void operator()() {
//
//     try {
//       co_await promise.initial_suspend() ;


std::clog << __FUNCTION__ << ":" << __LINE__ << std::endl; 
    
auto promise = co_await Awaitable<ReturnObject::promise_type>{}; 


std::clog << promise->_val << std::endl;

// std::clog << __FUNCTION__ << ":" << __LINE__ << std::endl; 

// co_await Awaitable<ReturnObject::promise_type>{}; 


// std::clog << __FUNCTION__ << ":" << __LINE__ << std::endl; 


//        promise.return_value(...);
//        goto final_suspend;
//
//     } catch ( ... ) {
//        if (!initial-await-resume-called)
//            throw ;
//        promise.unhandled_exception() ;
//     }
//
//     final_suspend :
//        co_await promise.final_suspend() ;
//
//   }
// };
//
//
//
// auto frame = new CoroutineFrame;
// auto returnObject { frame->promise.get_return_object() };
//    
// (*coroFrame)();
//        
// return returnObject;

}


int main(int argc, const char** argv) {



    auto ms = create_messaging_service("nats://localhost:4222,nats://localhost:4223,nats://localhost:4224");



    std::coroutine_handle<ReturnObject::promise_type> handle = run();

    std::cout << handle.promise()._val << std::endl;
    sleep(5);

    handle.resume();

    sleep(5);


    
    delete ms;

    std::clog << "Done."  << std::endl;

    

    return EXIT_SUCCESS;
}
