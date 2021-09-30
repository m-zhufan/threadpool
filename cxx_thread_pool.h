#ifndef CXX_THREAD_POOL_H
#define CXX_THREAD_POOL_H

#include "cxx_thread_pool_global.h"

#include "cxx_thread.h"

CXX_THREAD_POOL_BEGIN

class CXX_THREAD_POOLSHARED_EXPORT cxx_thread_pool
{
    enum Task_Priority {
        Default = 0,
        High_Priority,
        Low_Priority,
    };

public:
    cxx_thread_pool() = delete;
    cxx_thread_pool(const cxx_thread_pool &) = delete;
    cxx_thread_pool(cxx_thread_pool &&) = delete;

    cxx_thread_pool(const char *name, int max_task = 8);


    unsigned int process(const cxx_thread::task_fn &fn, Task_Priority prio = Default);

    //! \return 0 : 任务取消， 1 :任务进行中
    int cancel(unsigned int process_id);

    void task_timeout_exit(cxx_thread *task);

public:
    class pool_impl;
    friend class pool_impl;
private:
    pool_impl *m_impl= 0;
};

CXX_THREAD_POOL_END

#endif // CXX_THREAD_POOL_H
