#include <memory> // shared_ptr
#include <future> // std::future

#include "thread_pool.hpp"
#include "i_task.hpp"

namespace hrd31
{

class ThreadPool::PauseTask: public ITask
{
public:
	explicit PauseTask(std::condition_variable &cv_): ITask(ADMIN), m_cv(cv_)
	{}
	
	~PauseTask() = default;
	
	PauseTask(const PauseTask& other_) = delete;
	PauseTask& operator=(const PauseTask& other_) = delete;
private:
	std::condition_variable &m_cv;
	void Execute() override
	{
		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		m_cv.wait(lock);
	}
};

class ThreadPool::StopTask: public ITask 
{
public: 
	explicit StopTask(): ITask(ADMIN)
	{}
	
	~StopTask() = default;
	
	StopTask(const StopTask& other_) = delete;
	StopTask& operator=(const StopTask& other_) = delete;
	
private:
	void Execute() override
	{
		throw 's'; //using exception to exit while of thread pool func
	}
};

ThreadPool::ThreadPool(size_t size_): m_tasks(CompareFunc , Container()),
									  m_pool(size_),
									  m_pool_size(size_),
									  m_cv(),
									  m_logger(Singleton<Logger>::GetInstance())
{
	ConstructThreads(size_);
}
   
ThreadPool::~ThreadPool()
{
	m_logger->Log(Logger::INFO, "ThreadPool->In destructor body",
				   __FILE__, __LINE__, false); 
	Stop();
	m_logger->Log(Logger::INFO, "ThreadPool->Finishing destructor",
				   __FILE__, __LINE__, false); 
}

void ThreadPool::ConstructThreads(size_t size_)
{
	for (size_t i = 0; i < size_; ++i)
	{
		m_pool.emplace_back(&ThreadPool::ThreadFunc, this);
	}
}

void ThreadPool::ThreadFunc() 
{
	try
	{
		while (true)
		{
			m_tasks.Pop()->Execute();
		}
	}
		
	catch (char c_)
	{}
		
}

bool ThreadPool::CompareFunc(task_ptr p1_, task_ptr p2_)
{
	return p1_->m_priority < p2_->m_priority;
}

void ThreadPool::AddTask(std::shared_ptr<ITask> task_)
{
	m_tasks.Push(task_);
}

void ThreadPool::SetSize(size_t new_size_)
{
	if (new_size_ > m_pool_size)
	{
		if (new_size_ > m_pool.size()) // we need more threads
		{
			// notfify m_pool.size() - m_pool_size times
			for (size_t i = 0; i < m_pool.size() - m_pool_size; ++i)
			{
				m_cv.notify_one();
			}
			//add threads number new_size_-m_pool.size()
			ConstructThreads(new_size_ - m_pool.size());
		}
		else
		{
			// notify new_size_ - m_pool.size()
			for (size_t i = 0; i < m_pool.size() - new_size_; ++i)
			{
				m_cv.notify_one();
			}
		}
	}

	else
	{
		// pause new_size_-m_pool_size
		for (size_t i = 0; i < new_size_ - m_pool.size(); ++i)
		{
			AddTask(std::make_shared<PauseTask>(m_cv));
		}
	}
	m_pool_size = new_size_;
}

void ThreadPool::Pause()
{
	for (size_t i = 0; i < m_pool.size(); ++i)
	{
		AddTask(std::make_shared<PauseTask>(m_cv));
	}
}

void ThreadPool::Resume()
{
	m_cv.notify_all();
}

void ThreadPool::Stop()
{
	Resume();

	for (size_t i = 0; i < m_pool.size(); ++i) // send exception
	{
		AddTask(std::make_shared<StopTask>());
	}
	
	for (size_t i = 0; i < m_pool.size(); ++i) // join threads
	{
		if (m_pool[i].joinable())
		{
			m_pool[i].join();
		}
	}
		
}

void ThreadPool::Stop(std::chrono::seconds timeout_)
{
	std::future<void> stop_ret = std::async(std::launch::async,
								 static_cast<void (ThreadPool::*)()>(&ThreadPool::Stop),
								 this);

	std::future_status status = stop_ret.wait_for(timeout_);
	if (status == std::future_status::ready)
	{
		return;
	}
	m_pool.clear();
}

size_t ThreadPool::DefaultThreadsNum() // wrapper hardware_concurrency()
{
    unsigned int n = std::thread::hardware_concurrency();
    if (n != 0)
	{
		return n;
	}	
	return 1;
}


/*

    using task_ptr = std::shared_ptr<ITask>;
    using Container = std::vector<task_ptr>;
    using Compare = std::function<bool(task_ptr p1_, task_ptr p2_)>;

    WPQueue<task_ptr, Container, Compare> m_tasks;
    std::vector<std::thread> m_pool;
    size_t m_pool_size;
    std::condition_variable m_cv;

    class Pause;
    class Stop;
};*/



//Pause_task
    //wait on condition var

//Pause
    //AddTask(pause_task) as nums of threads

//Resume
    //Notify m_pool_size times

//SetSize
    //if size > m_pool_size
        //add threads / resume threads
    //else
        //pause threads

//Stop
    //


}//namespace hrd31

