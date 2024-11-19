/* -----------------------------------------------------------------------------
Description:
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
    v0.2 - Approved
----------------------------------------------------------------------------- */
#ifndef __HRD31_PLUG_N_PLAY_HPP__
#define __HRD31_PLUG_N_PLAY_HPP__

#include <string> //std::string
#include <thread> //std::thread
#include <memory> //std::unique_ptr

#include "msg_broker.hpp"
#include "logger.hpp"

namespace hrd31
{

class PNPError : public std::runtime_error 
{
public:
	explicit PNPError(const std::string what_): runtime_error(what_)
	{}
};

class PlugNPlay
{
public:
    explicit PlugNPlay(const std::string &path_ = "./pnp");
    ~PlugNPlay() = default;

    PlugNPlay(const PlugNPlay &other_) = delete;
    PlugNPlay &operator=(const PlugNPlay &other_) = delete;

private:
    class DirMonitor;
    class SOLoader;

    Dispatcher<std::string> m_dispatcher;

    std::unique_ptr<DirMonitor> m_monitor;
    std::unique_ptr<SOLoader> m_loader;
};


class PlugNPlay::DirMonitor
{
public:
    explicit DirMonitor(Dispatcher<std::string> &dispatcher_,
                        const std::string &path_ = "./pnp");
    ~DirMonitor();

    DirMonitor(const DirMonitor &other_) = delete;
    DirMonitor &operator=(const DirMonitor &other_) = delete;

private:
    int m_dir_fd;
    int m_wd;
    Dispatcher<std::string> &m_dispatcher;
    std::thread m_async_listener;
    Logger *m_logger;

    static int InotifyInit();
    static int InotifyAdd(int fd_, const std::string &path_);
    void AsyncListener(const std::string &path_);
    
};

class PlugNPlay::SOLoader
{
public:
    explicit SOLoader(Dispatcher<std::string> &dispatcher_);
    ~SOLoader();

    SOLoader(const SOLoader &other_) = delete;
    SOLoader &operator=(const SOLoader &other_) = delete;

private:
    CallBack<std::string, SOLoader> m_call_back;
    void Load(const std::string &file_name_);
    std::vector<void *> m_opened_so;
    Logger *m_logger;
};

} // namespace hrd31

#endif //__HRD31_PLUG_N_PLAY_HPP__