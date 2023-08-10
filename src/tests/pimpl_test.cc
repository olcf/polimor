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
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <utility>
#include <cassert>

#include "../common/pimpl.h"
#include "../common/impl/pimpl_impl.h"




class impl {

        private:
            std::string _str;

        public:
            impl(std::string str) : _str(str) { };

            std::string  get() { return _str; }
    };


template<template<typename impl> typename PIMPL, typename T>
class impl_wrapper {

    private:
        PIMPL<impl> _impl;

    public:
        impl_wrapper(): _impl("") { };
        impl_wrapper(std::string v) : _impl(v) { };

        std::string get() { return _impl->get(); }

};



template<template<typename> typename PIMPL, typename T>
void test_copy_constructor(impl_wrapper<PIMPL, T> value) {
    
    std::clog << "Testing " << (typeid(value).name()) << std::endl;

    impl_wrapper<PIMPL, T> a(value);

    std::clog << "A = " << a.get() << " value = " << value.get() << std::endl;

    assert(a.get() == value.get());
}

template<template<typename> typename PIMPL, typename T>
void test_copy_assignment(impl_wrapper<PIMPL, T> value) {

    
    impl_wrapper<PIMPL, T> a;

    a = value;

    std::clog << "A = " << a.get() << " orig_value = " << value.get() << std::endl;

    assert(a.get() == value.get());
}



template<template<typename> typename PIMPL, typename T>
void test_move_constructor(impl_wrapper<PIMPL, T> value) {

    T orig_value = value.get();

    impl_wrapper<PIMPL, T>  a(std::move(value));

    std::clog << "A = " << a.get() << std::endl;

    assert(a.get() == orig_value);
    
    
    bool threw=false;
    try {

        value.get();

    } catch(...) {

        threw = true;
    }

    if(not threw) {
        throw std::runtime_error("Failed to throw exception");
    }
}



template<template<typename> typename PIMPL, typename T>
void test_move_assignment(impl_wrapper<PIMPL, T> value) {

    T orig_value = value.get();


    impl_wrapper<PIMPL, T> a; 

    a = std::move(value);

    std::clog << "A = " << a.get() << std::endl;

    assert(a.get() == orig_value);

    bool threw=false;

    try {

        value.get();

    } catch(...) {

        threw = true;
    }

    if(not threw) {
        throw std::runtime_error("Failed to throw exception");
    }
}




void test_pimpl() {

    /** Test constructor **/
    impl_wrapper<qs::common::pimpl, std::string> orig_value("Hello There");

    std::clog << "orig_value = " << orig_value.get() << std::endl;

    /** Test copy constructor **/
    test_copy_constructor(orig_value);

    /** Test copy assignment **/
    test_copy_assignment(orig_value);
    
    /** Test move constructor **/
    test_move_constructor(orig_value);

    /** Test move assignment **/
    test_move_assignment(orig_value);    
}


void test_shared_pimpl() {

    /** Test constructor **/
    impl_wrapper<qs::common::shared_pimpl, std::string> orig_value("Hello There");

    std::clog << "orig_value = " << orig_value.get() << std::endl;

    /** Test copy constructor **/
    test_copy_constructor(orig_value);

    /** Test copy assignment **/
    test_copy_assignment(orig_value);
    
    /** Test move constructor **/
    test_move_constructor(orig_value);

    /** Test move assignment **/
    test_move_assignment(orig_value);
}

void test_internal_placement_pimpl() {

    /** Test constructor **/
    impl_wrapper<qs::common::inplace_pimpl, std::string> orig_value("Hello There");

    std::clog << "orig_value = " << orig_value.get() << std::endl;

    /** Test copy constructor **/
    test_copy_constructor(orig_value);  

    /** Test copy assignment **/
    test_copy_assignment(orig_value);
    
    /** Test move constructor **/
    test_move_constructor(orig_value);

    /** Test move assignment **/
    test_move_assignment(orig_value);

    std::clog << "orig_value = " << orig_value.get() << std::endl;
}


int main(int argc, char *argv[]) {


    test_pimpl();

    test_shared_pimpl();

    test_internal_placement_pimpl();

    return EXIT_SUCCESS;
}