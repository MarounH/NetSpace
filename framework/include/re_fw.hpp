/* -----------------------------------------------------------------------------
Description: Request Engine Frame-Work
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
    v0.2 - Mentor approved
    v0.2.1 - class Stop and Pause are changed to PauseTask and StopTask
----------------------------------------------------------------------------- */
#ifndef __HRD31_RE_FW_HPP__
#define __HRD31_RE_FW_HPP__

#include <string>
#include <memory> //std::shared_ptr
#include <unordered_map>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <unistd.h> // pipe

#include "i_task.hpp"
#include "factory.hpp"
#include "thread_pool.hpp"
#include "pnp.hpp"
#include "i_input_src.hpp"
#include "i_re_task.hpp"

namespace hrd31
{

class ReqEngineError : public std::runtime_error 
{
public:
	explicit ReqEngineError(const std::string what_): runtime_error(what_)
	{}
};

template <typename KEY, typename ARGS> 
class RequestEngine final
{
public:
    const int MAX_EVENTS = 10;
    enum pipe_fd {READ=0, WRITE=1, PIPE_SIZE=2};
    //DefaultThreadsNum - returns the number of concurrent threads the 
    //implementation supports;
    //if the value is not well defined it returns 1.
    explicit RequestEngine(size_t thread_num_ = ThreadPool::DefaultThreadsNum(),
        const std::string& pnp_path_ = "./pnp"); 
    ~RequestEngine();
    RequestEngine(const RequestEngine&) = delete;
    RequestEngine& operator=(const RequestEngine&) = delete;

    void ConfigInputSrc(std::shared_ptr<IInputSrc<KEY, ARGS>> input_src_);
    using CreateFunc = std::function<std::shared_ptr<IRETask>(ARGS)>;
    void ConfigTask(const KEY& key_, CreateFunc func_);
    void Stop();
    void Run();

private:
    Factory<IRETask, KEY, ARGS>* m_factory;
    ThreadPool m_thread_pool;
    PlugNPlay m_pnp;
    std::thread m_reactor;
    std::unordered_map<int ,std::shared_ptr<IInputSrc<KEY, ARGS>>> m_request_srcs;
    int m_epoll_fd;
    std::vector<struct epoll_event> m_events;
    int m_run;
    void AsyncEpoll();
    std::vector<int> m_pipe;
    Logger *m_logger;

    static int EpollCreate();
    static void AddFdToEpoll(int epoll_fd_, int to_add_fd_);
};

/////////////////////////////////// Implementation ////////////////////////////
template <typename KEY, typename ARGS>
RequestEngine<KEY,ARGS>::RequestEngine(size_t thread_num_,
                                       const std::string& pnp_path_):
                                       m_factory(Singleton<Factory<IRETask, KEY, ARGS>>::GetInstance()),
                                       m_thread_pool(thread_num_),
                                       m_pnp(pnp_path_),
                                       m_reactor(),
                                       m_request_srcs(),
                                       m_epoll_fd(EpollCreate()), 
                                       m_events(MAX_EVENTS),
                                       m_run(false),
                                       m_pipe(PIPE_SIZE),
                                       m_logger(Singleton<Logger>::GetInstance())
{
    if (-1 == pipe(m_pipe.data()))
    {
        std::cout << "couldn't create pipe necessary for exiting epoll\n";
        // handle exception
        exit(0);
    }

    AddFdToEpoll(m_epoll_fd, m_pipe[READ]);
}

template <typename KEY, typename ARGS>
RequestEngine<KEY,ARGS>::~RequestEngine()
{
    m_logger->Log(Logger::INFO, "RequestEngine->In destructor body",
				  __FILE__, __LINE__, false); 

    m_reactor.join();
    if (0 == m_run) // not good enough because someone might not use Run()
    {
        close(m_epoll_fd);
        close(m_pipe[READ]); close(m_pipe[WRITE]);
    }
    
    m_logger->Log(Logger::INFO, "RequestEngine->Finishing destructor",
				  __FILE__, __LINE__, false);
}

template <typename KEY, typename ARGS>
void RequestEngine<KEY,ARGS>::ConfigInputSrc(std::shared_ptr<IInputSrc<KEY, ARGS>> input_src_)
{
    AddFdToEpoll(m_epoll_fd, input_src_->GetFd()); 

    m_request_srcs.insert(std::make_pair(input_src_->GetFd(), input_src_));
}

template <typename KEY, typename ARGS>
void RequestEngine<KEY,ARGS>::ConfigTask(const KEY& key_, CreateFunc func_)
{
    m_factory->Add(key_, func_);
}

template <typename KEY, typename ARGS>
void RequestEngine<KEY,ARGS>::Run()
{
    m_run = true;
    m_reactor = std::thread(&RequestEngine<KEY,ARGS>::AsyncEpoll, this);
}

template <typename KEY, typename ARGS>
void RequestEngine<KEY,ARGS>::Stop()
{
    int write_ret = write(m_pipe[WRITE], "exit", sizeof("exit"));
    if (-1 == write_ret)
    {
        std::cout << "write failed\n";

    }
}

template <typename KEY, typename ARGS>
void RequestEngine<KEY,ARGS>::AsyncEpoll()
{
    while (m_run)
    {
        int n_fds = epoll_wait(m_epoll_fd, m_events.data(), MAX_EVENTS, -1);
        for (int i = 0; i < n_fds; ++i)
        {
            if(m_pipe[READ] == m_events[i].data.fd)
            {
                m_run = 0;
                break;
            }

            if(m_events[i].events & EPOLLIN)
            {
                int current_fd = m_events[i].data.fd;
                std::pair<KEY, ARGS> input_pair = m_request_srcs[current_fd]->Read();

                m_thread_pool.AddTask(m_factory->Create(input_pair.first,
                                                        input_pair.second));
            }
        }
    }
    std::cout << "exited reactor thread\n";
}


template <typename KEY, typename ARGS>
int RequestEngine<KEY,ARGS>::EpollCreate()
{
    int ret_val = epoll_create1(0);
    if (-1 == ret_val)
    {
        throw ReqEngineError("Failed to epoll_create (create epoll fd)");
    }
    return ret_val;
}

template <typename KEY, typename ARGS>
void RequestEngine<KEY,ARGS>::AddFdToEpoll(int epoll_fd_, int to_add_fd_)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = to_add_fd_;

    int epoll_ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, to_add_fd_, &event); 
    if (-1 == epoll_ret)
    {
        std::cout << "couldn't add fd = " << to_add_fd_ << " to epoll\n";
        // handle exception
        exit(1);
    }
}

}//namespace hrd31
#endif //__HRD31_RE_FW_HPP__