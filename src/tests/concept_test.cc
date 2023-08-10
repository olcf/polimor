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
#include <string_view>
#include <iostream>

template<typename agent_impl>
concept agent_like = requires(agent_impl impl) {
        { impl.run() } -> std::same_as<void>;
        { impl.stop() } -> std::same_as<void>;
};

template<agent_like agent_impl>
class agent : public agent_impl { 

     int a;
     public:

        agent() {
            std::clog << "Agent constructor" << std::endl;

            a = 1;
        }
        agent(const agent &) {
            std::clog << "agent Copy constructor" << std::endl;
        }

        agent(agent &&) {
            std::clog << "agent Move constructor" << std::endl;
        }        

        agent &operator=(const agent &) {
            std::clog << "agent Copy operator" << std::endl;

            return *this;
        }

        agent &operator=(const agent &&) {
            std::clog << "agent Move operator" << std::endl;

            return *this;
        }
};


class my_agent_impl {

    int b;
    public:

        my_agent_impl() {
            std::clog << "my_agent_impl constructor" << std::endl;
            b = 0;
        }

        my_agent_impl(const my_agent_impl &) {
            std::clog << "my_agent_impl Copy constructor" << std::endl;
        }

        my_agent_impl(my_agent_impl &&) {
            std::clog << "my_agent_impl Move constructor" << std::endl;
        }        

        my_agent_impl &operator=(const my_agent_impl &) {
            std::clog << "my_agent_impl Copy operator" << std::endl;

            return *this;
        }

        my_agent_impl &operator=(const my_agent_impl &&) {
            std::clog << "my_agent_impl Move operator" << std::endl;

            return *this;
        }

        void run() { std::clog << "running" <<std::endl;}
        void stop() {};
};

using my_agent = agent<my_agent_impl>;


template<agent_like agent_impl>
agent<agent_impl> create_agent() {

    agent<agent_impl> a = agent<agent_impl>();

    if(true) {
        return a;
    }

    return agent<agent_impl>();
}


class message {

};

class scan_message : public message {

};

class purge_message : public message {

};

template<typename MsgImpl>
concept Msg = std::derived_from<MsgImpl, message>;





template<typename MsgPublisherImpl>
concept MsgPublisher = requires(MsgPublisherImpl impl,
                                const scan_message &scan_msg,
                                const purge_message &purge_msg) {

    requires Msg<scan_message>;
    requires Msg<purge_message>;

    { impl.template send<scan_message>(scan_msg)  } -> std::same_as<void>;
    { impl.template send<purge_message>(purge_msg) } -> std::same_as<void>;
};



template<typename MsgSubscriberImpl>
concept MsgSubscriber = requires(MsgSubscriberImpl impl) {
       
    requires Msg<scan_message>;
    requires Msg<purge_message>;

    {  impl.template receive<scan_message>() } -> std::same_as<scan_message>;
    {  impl.template receive<purge_message>() } -> std::same_as<purge_message>;
};

template<typename MsgServiceImpl> 
concept MsgService = requires(MsgServiceImpl impl,
                              const std::string_view sv) {

    { impl.create_queue_publisher(sv ) } -> MsgPublisher;
    { impl.create_queue_subscriber(sv) } -> MsgSubscriber;
};


class msg_subscriber_impl {
    public:
        
        template<Msg T> 
        T receive() { return {}; }
};


class msg_publisher_impl {
    public:
        template<Msg T> 
        void send(const T &msg) { }
};



class msg_service_impl {

    public:
        MsgPublisher auto create_queue_publisher(const std::string_view sv) {
            return msg_publisher_impl();
        }

        MsgSubscriber auto create_queue_subscriber(const std::string_view name) { 
            return msg_subscriber_impl();
        }
};

MsgService auto create_msg_service_impl() {

    return msg_service_impl();
}

int main(int argc, const char* argv[]) {
    

    MsgService auto m = create_msg_service_impl();

    auto p = m.create_queue_publisher("test");

    //agent a;

    my_agent a = create_agent<my_agent_impl>();

    a.run();

    return EXIT_SUCCESS;
}