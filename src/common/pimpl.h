/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __PIMPL_H__
#define __PIMPL_H__

#include <memory>
#include <new>
#include <utility>
#include <type_traits>

namespace qs::common {

    /**
    * @brief Basic private implementation pointer using a unique_ptr.  T is 
    * a template parameter to the implementation.
    */
    template<typename T>
    class pimpl {

        private:
            std::unique_ptr<T> _ptr;

        public:

            template<typename ...Args> 
            pimpl(Args&& ...args);

            pimpl(const pimpl &other);
            
            pimpl& operator=(const pimpl &rhs);

            pimpl(pimpl&& other);

            pimpl& operator=(pimpl&& rhs);
        
            ~pimpl() = default;

            /* Access impl */
            T* operator->() const;

            /* Access impl */
            T& operator*() const;
    };


    /**
     * @brief Another pimpl implementation using a shared pointer.  The copy 
     * constructor and assignment operator increases the reference count on 
     * the impl object.
     */
    template<typename T>
    class shared_pimpl {

        private:
            std::shared_ptr<T> _ptr;

        public:

            template<typename ...Args> 
            shared_pimpl(Args&& ...args);
   
            shared_pimpl(const shared_pimpl &other);
            shared_pimpl& operator=(const shared_pimpl &rhs);

            /* Moves are enabled */
            shared_pimpl(shared_pimpl&& other);
            shared_pimpl& operator=(shared_pimpl&& rhs);
        
            ~shared_pimpl() = default;

            /* Access impl */
            T* operator->() const;

            /* Access impl */
            T& operator*() const;
    };

    
    /**
     * @brief Similar to the pimpl implementation but has an inplace buffer for
     * storage of the impl object. This should be more cache friendly than the
     * default pimpl. 
     */
    template<typename T>
    class inplace_pimpl {

        private:

            /* Aligned storage area */  
            std::aligned_storage_t<sizeof(T), alignof(T)> _data[sizeof(T)];

            /* Whether the _data buffer holds an object or not */
            bool _initialized = false;

            T *_get_ptr() const;

        public:

            template<typename ...Args> 
            inplace_pimpl(Args&& ...args);
   
            inplace_pimpl(const inplace_pimpl &other);

            inplace_pimpl& operator=(const inplace_pimpl &rhs);

            inplace_pimpl(inplace_pimpl&& other);
            
            inplace_pimpl& operator=(inplace_pimpl&& rhs);
        
            ~inplace_pimpl();

            /* Access impl */
            T* operator->() const;

            /* Access impl */
            T& operator*() const;
    };
}

#include "impl/pimpl_impl.h"


#endif // __PIMPL_H__