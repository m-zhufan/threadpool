/********************************************************************
 * Copyright (c) 2021 Edan Instruments,Inc.
 * All rights reserved.
 *
 * Project:    threadpool
 * File:       cxx_thread.cpp
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
#include "cxx_thread.h"

#include <string>
#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>
#include "cxx_thread_pool.h"


CXX_THREAD_POOL_BEGIN

class cxx_thread::thread_impl
{
public:
    bool m_quit = false;
    bool m_running = false;

    int timeout_sec = 0;

    std::string m_name = 0;
    cxx_thread_pool *owner_pool = 0;
    cxx_thread *m_self = 0;

    std::list<cxx_thread::task_fn> m_tasks;

    std::mutex m_mutex;
    std::condition_variable m_cond;

public:
    thread_impl(cxx_thread *self, const char *name, cxx_thread_pool *owner)
     : m_name(name), owner_pool(owner), m_self(self)
    { }

    void start()
    {
        if (m_quit || m_running)
        {
            return;
        }

        std::thread runner(std::bind(&thread_impl::running, this));
        pthread_setname_np(runner.native_handle(), m_name.data());

        m_running = true;
        runner.detach();
    }

    void quit()
    {
        m_mutex.lock();

        m_quit = true;
        m_cond.notify_one();

        m_mutex.unlock();
    }

    void wait()
    {
        while (1)  //等待线程任务执行完
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            m_mutex.lock();
            if (!m_running)
            {
                m_mutex.unlock();
                break;
            }
            m_mutex.unlock();
        }
    }

    void process(const cxx_thread::task_fn &fn)
    {
        m_mutex.lock();

        m_tasks.push_back(fn);
        m_cond.notify_one();

        m_mutex.unlock();
    }

private:
    void running()
    {
        while (1)
        {
            m_mutex.lock();
            if (m_quit)
            {
                m_mutex.unlock();
                break;
            }

            m_mutex.lock();
            if (m_tasks.empty())
            {
                std::unique_lock<std::mutex> locker(m_mutex, std::try_to_lock_t());
                m_cond.wait_for(locker, std::chrono::milliseconds(1000), [this]() ->bool {
                                    m_mutex.lock();

                                    if (!m_tasks.empty())
                                    {
                                        m_mutex.unlock();
                                        return true; //来新任务, 退出休眠
                                    }

                                    if (m_quit)
                                    {
                                        return true; //主动释放线程
                                    }

                                    ++timeout_sec;
                                    if (++timeout_sec >= 60)
                                    {
                                        if (owner_pool)
                                        {
                                            owner_pool->task_timeout_exit(m_self);
                                        }

                                        m_quit = true;
                                        m_mutex.unlock();
                                        return true; //空闲60s，释放线程
                                    }

                                    m_mutex.unlock();
                                    return false; //继续等待
                });

                m_mutex.unlock();

            }
            else
            {
                timeout_sec = 0;

                cxx_thread::task_fn fn = m_tasks.front();
                m_tasks.erase(m_tasks.begin());

                m_mutex.unlock();

                if (fn) fn();
            }
        }

        m_mutex.lock();
        m_running = false;
        m_mutex.unlock();
    }
};

cxx_thread::cxx_thread(const char *name, cxx_thread_pool *owner)
    : m_impl(new thread_impl(this, name, owner))
{

}

cxx_thread::~cxx_thread()
{
    quit();
    wait();

    if (m_impl) delete m_impl;
    m_impl = 0;
}

void cxx_thread::start()
{
    if (m_impl)
    {
        m_impl->start();
    }
}

void cxx_thread::quit()
{
    m_impl->quit();
}

void cxx_thread::wait()
{
    m_impl->wait();
}

void cxx_thread::process(const cxx_thread::task_fn &fn)
{
    m_impl->process(fn);
}

CXX_THREAD_POOL_END

using cxx_thread = _cxx_thread::cxx_thread;
