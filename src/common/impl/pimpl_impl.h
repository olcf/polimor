/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __PIMPL_IMPL_H__
#define __PIMPL_IMPL_H__


namespace qs::common {


    /*** PIMPL implementation ***/

    template<typename T>
    template<typename ...Args> 
    pimpl<T>::pimpl(Args&& ...args) :
        _ptr(std::make_unique<T>(std::forward<Args>(args)...)) { }
   
            
    template<typename T>
    pimpl<T>::pimpl(const pimpl &other) : _ptr(std::make_unique<T>(*other)) { }
            
    template<typename T>
    pimpl<T>& pimpl<T>::operator=(const pimpl &rhs) {
        _ptr = std::make_unique<T>(*rhs);
        return *this;
    }

    template<typename T>
    pimpl<T>::pimpl(pimpl&& other) : 
        _ptr(std::exchange(other._ptr, nullptr)) { }

    template<typename T>
    pimpl<T>& pimpl<T>::operator=(pimpl&& rhs) {
        _ptr = std::exchange(rhs._ptr, nullptr);
        return *this;
    }
        
    template<typename T>
    T* pimpl<T>::operator->() const {

        if(not _ptr) {
            throw std::logic_error("pimpl<T> is not initialized");
        }

        return _ptr.get();
    }

    template<typename T>
    T& pimpl<T>::operator*() const {

        if(not _ptr) {
            throw std::logic_error("pimpl<T> is not initialized");
        }

        return *_ptr.get();
    }


    /*** Shared PIMPL implementation ***/


    template<typename T>
    template<typename ...Args> 
    shared_pimpl<T>::shared_pimpl(Args&& ...args) :
        _ptr(std::make_shared<T>(std::forward<Args>(args)...)) { }

    template<typename T>
    shared_pimpl<T>::shared_pimpl(const shared_pimpl &other) : _ptr(other._ptr) { }
    
    template<typename T>
    shared_pimpl<T>& shared_pimpl<T>::operator=(const shared_pimpl &rhs) {
        _ptr = rhs._ptr;
        return *this;
    }

    template<typename T>
    shared_pimpl<T>::shared_pimpl(shared_pimpl&& other) : 
        _ptr(std::exchange(other._ptr, nullptr)) { }

    template<typename T>
    shared_pimpl<T>& shared_pimpl<T>::operator=(shared_pimpl&& rhs) {
        _ptr = std::exchange(rhs._ptr, nullptr);
        return *this;
    }
        
    template<typename T>  
    T* shared_pimpl<T>::operator->() const {

        if(not _ptr) {
            throw std::logic_error("shared_pimpl<T> is not initialized");
        }

        return _ptr.get();
    }

    template<typename T>
    T& shared_pimpl<T>::operator*() const {
                
        if(not _ptr) {
            throw std::logic_error("shared_pimpl<T> is not initialized");
        }

        return *_ptr.get();
    }


    /*** Inplace PIMPL implementation ***/


    template<typename T> 
    T *inplace_pimpl<T>::_get_ptr() const { 
        return std::launder(const_cast<T *>(
            reinterpret_cast<const T *>(_data))); 
    }

        
    template<typename T> 
    template<typename ...Args> 
    inplace_pimpl<T>::inplace_pimpl(Args&& ...args) {
        ::new(_data) T(std::forward<Args>(args)...); 
        _initialized = true;
    }
   
    template<typename T> 
    inplace_pimpl<T>::inplace_pimpl(const inplace_pimpl &other) {
                
        if(_initialized) {
            std::destroy_at(_get_ptr());
        }

        ::new(_data) T(*other._get_ptr());
        _initialized = true;
    }

    template<typename T> 
    inplace_pimpl<T>& inplace_pimpl<T>::operator=(const inplace_pimpl &rhs) {

        if(_initialized) {
            std::destroy_at(_get_ptr());
        }
        
        ::new(_data) T(*rhs._get_ptr());
        _initialized = true;

        return *this;
    }

    template<typename T> 
    inplace_pimpl<T>::inplace_pimpl(inplace_pimpl&& other) {
        *_get_ptr() = std::move(*other._get_ptr());
        _initialized = true;
        other._initialized = false;
    }
            
    template<typename T> 
    inplace_pimpl<T>& inplace_pimpl<T>::operator=(inplace_pimpl&& rhs) {
        *_get_ptr() = std::move(*rhs._get_ptr());
        _initialized = true;
        rhs._initialized = false;

        return *this;
    }
    
    template<typename T> 
    inplace_pimpl<T>::~inplace_pimpl() {
        if(_initialized) {
            std::destroy_at(_get_ptr());
        }
    }

    template<typename T> 
    T* inplace_pimpl<T>::operator->() const {

        if(not _initialized) {
            throw std::logic_error("inplace_pimpl<T> is not initialized");
        }

        return _get_ptr();
    }

    template<typename T> 
    T& inplace_pimpl<T>::operator*() const {
        
        if(not _initialized) {
            throw std::logic_error("inplace_pimpl<T> is not initialized");
        }

        return *_get_ptr();
    }
}



#endif // __PIMPL_IMPL_H__