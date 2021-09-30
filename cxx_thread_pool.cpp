#include "cxx_thread_pool.h"

#include <mutex>
#include <map>
#include <list>

CXX_THREAD_POOL_BEGIN

class cxx_thread_pool::pool_impl
{
    cxx_thread_pool *m_self = 0;

    int max_task = 0;
    std::string m_name = "";

    cxx_thread **m_tasks = 0;
    std::list<cxx_thread *> m_idle_tasks;
    std::list<cxx_thread *> m_busy_tasks;

    unsigned int process_id = 0;
    std::list<unsigned int> m_processing_task;
    std::list<std::pair<unsigned int, cxx_thread::task_fn> > m_pending_task;

    std::mutex m_task_mutex;

public:
    pool_impl(cxx_thread_pool *pool, const char *name, int max)
        : m_self(pool), max_task(max), m_name(name)
    {
        initialize_tasks();
    }

    unsigned int process(const cxx_thread::task_fn &fn, cxx_thread_pool::Task_Priority prio = cxx_thread_pool::Default)
    {
        m_task_mutex.lock();

        unsigned int id = ++process_id;
        if (!m_idle_tasks.empty())
        {
            m_processing_task.push_back(id);
            cxx_thread *task = m_idle_tasks.front();
            task->process(std::bind(&pool_impl::handler, this, task, fn, id));
        }
        else
        {
            int task_num = (int)m_idle_tasks.size() + (int)m_busy_tasks.size();
            if (task_num < max_task)
            {
                std::string name = m_name + "_" + std::to_string(task_num);
                cxx_thread *task = new cxx_thread(name.data(), m_self);
                task->start();

                m_tasks[task_num] = task;
                m_busy_tasks.push_back(task);

                m_processing_task.push_back(id);

                task->process(std::bind(&pool_impl::handler, this, task, fn, id));
            }
            else
            {
                switch (prio)
                {
                case cxx_thread_pool::Default:
                case cxx_thread_pool::Low_Priority:
                    m_pending_task.push_back(std::make_pair(id, fn));
                    break;
                case cxx_thread_pool::High_Priority:
                    m_pending_task.push_front(std::make_pair(id, fn));
                    break;
                default:
                    break;
                }
            }
        }

        m_task_mutex.unlock();

        return id;
    }

    int cancel(unsigned int process_id)
    {
        m_task_mutex.lock();

        auto process_iter = std::find(m_processing_task.begin(), m_processing_task.end(), process_id);
        if (process_iter != m_processing_task.end())
        {
            m_task_mutex.unlock();
            return 1;  //该任务在执行中
        }

        auto pending_iter = std::find_if(m_pending_task.begin(), m_pending_task.end(),
            [process_id](std::pair<unsigned int, cxx_thread::task_fn> &pend) {
            return  process_id == pend.first;
        });
        if (pending_iter != m_pending_task.end())
        {
            m_pending_task.erase(pending_iter);  //从任务等待队列中删除任务
        }

        m_task_mutex.unlock();
        return 0;
    }

    void task_timeout_exit(cxx_thread *task)
    {
        m_task_mutex.lock();

        auto task_iter = std::find(m_idle_tasks.begin(), m_idle_tasks.end(), task);
        if (task_iter != m_idle_tasks.end())
        {
            m_idle_tasks.erase(task_iter);
        }

        for (int i = 0; i < max_task; i++)
        {
            if (m_tasks[i] == task)
            {
                m_tasks[i] = 0;
            }
        }

        if (task) delete task;
        task = 0;

        m_task_mutex.unlock();
    }

private:
    void handler(cxx_thread *task, const cxx_thread::task_fn &fn, unsigned int process_id)
    {
        if (fn) fn();

        m_task_mutex.lock();

        auto process_iter = std::find(m_processing_task.begin(), m_processing_task.end(), process_id);
        if (process_iter != m_processing_task.end())
        {
            m_processing_task.erase(process_iter);
        }

        if (!m_pending_task.empty())
        {
            unsigned int id = m_pending_task.front().first;
            cxx_thread::task_fn fn = m_pending_task.front().second;
            m_pending_task.erase(m_pending_task.begin());
            m_processing_task.push_back(id);

            task->process(std::bind(&pool_impl::handler, this, task, fn, id));
        }
        else
        {
            auto busy_iter = std::find(m_busy_tasks.begin(), m_busy_tasks.end(), task);
            if(busy_iter != m_busy_tasks.end())
            {
                m_busy_tasks.erase(busy_iter);
            }

            m_idle_tasks.push_back(task);
        }

        m_task_mutex.unlock();
    }

    void initialize_tasks()
    {
        if(m_self)
        {
            m_tasks = new cxx_thread *[max_task] {0};
        }
    }
};

cxx_thread_pool::cxx_thread_pool(const char *name, int max_task) : m_impl(new pool_impl(this, name, max_task))
{

}

unsigned int cxx_thread_pool::process(const cxx_thread::task_fn &fn, Task_Priority prio)
{
    return m_impl->process(fn, prio);
}

int cxx_thread_pool::cancel(unsigned int process_id)
{
    return m_impl->cancel(process_id);
}

void cxx_thread_pool::task_timeout_exit(cxx_thread *task)
{
    m_impl->task_timeout_exit(task);
}

CXX_THREAD_POOL_END

using cxx_thread_pool = _cxx_thread::cxx_thread_pool;
