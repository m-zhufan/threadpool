/********************************************************************
 * Copyright (c) 2021 Edan Instruments,Inc.
 * All rights reserved.
 *
 * Project:    EUP
 * File:       cxx_thread.h
 * Date:       2021-09-30
 * Author:     zhufan
 * Brief:
 * Other:      None
 *
 * Revision:
 *    Date:    2021-09-30
 *    Author:  zhufan
 *    Content: Create
 ********************************************************************
 */
#ifndef CXX_THREAD_H
#define CXX_THREAD_H

#include <functional>
#include "cxx_thread_pool_global.h"

CXX_THREAD_POOL_BEGIN

class cxx_thread_pool;
class CXX_THREAD_POOLSHARED_EXPORT cxx_thread
{
public:
    using task_fn = std::function<void (void)>;

public:
    cxx_thread() = delete;
    cxx_thread(const char *name, cxx_thread_pool *owner);
    virtual ~cxx_thread();

public:
    void start();
    void quit();
    void wait();

    void process(const task_fn &fn);

public:
    class thread_impl;
    friend class thread_impl;
private:
    thread_impl *m_impl = 0;
};

CXX_THREAD_POOL_END

#endif // CXX_THREAD_H
