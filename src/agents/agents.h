/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __AGENTS_H__
#define __AGENTS_H__


class agent {

    public:

        agent() = default;
        virtual ~agent() = default;

        virtual void run() = 0;
        virtual void stop() = 0;
};



#endif // __AGENTS_H__