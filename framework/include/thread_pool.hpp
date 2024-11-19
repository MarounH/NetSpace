/* -----------------------------------------------------------------------------
Description: 
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
    v0.2 - Mentor approved
----------------------------------------------------------------------------- */
#ifndef __HRD31_THREAD_POOL_HPP__
#define __HRD31_THREAD_POOL_HPP__

#include <memory>
#include <vector>
#include <thread>
#include <functional>
#include <chrono>

#include "waitable_pq.hpp"
#include "i_task.hpp"
#include "logger.hpp"

namespace hrd31
{

class TPError : public std::runtime_error 
{
public:
	explicit TPError(const std::string what_): runtime_error(what_)
	{}
};

class ThreadPool final
{
public:
    //DefaultThreadsNum - returns the number of concurrent threads the 
    //implementation supports;
    //if the value is not well defined it returns 1.
    explicit ThreadPool(size_t size_ = DefaultThreadsNum());   
    ~ThreadPool();

    ThreadPool(const ThreadPool& other_) = delete;
    ThreadPool& operator=(const ThreadPool& other_) = delete;

    void AddTask(std::shared_ptr<ITask> task_);
    void SetSize(size_t new_size_);
    void Pause();
    void Resume();
    void Stop();
    void Stop(std::chrono::seconds timeout_);
    static size_t DefaultThreadsNum(); // wrapper for std::thread::hardware_concurrency()

private:

    using task_ptr = std::shared_ptr<ITask>;
    using Container = std::vector<task_ptr>;
    using Compare = std::function<bool(task_ptr p1_, task_ptr p2_)>;

    WPQueue<task_ptr, Container, Compare> m_tasks;
    std::vector<std::thread> m_pool;
    size_t m_pool_size;
    std::condition_variable m_cv;
    Logger *m_logger;
	
    class PauseTask;
    class StopTask;
    
    void ThreadFunc();
    void ConstructThreads(size_t size_);
    static bool CompareFunc(task_ptr p1_, task_ptr p2_);
};

}//namespace hrd31

#endif //__HRD31_THREAD_POOL_HPP__
