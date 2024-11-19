#include <iostream>
#include <sys/inotify.h>
#include <unistd.h> // read
#include <dlfcn.h> // dlopen, dlclose

#include "pnp.hpp"

namespace hrd31
{
const int EVENT_SIZE = sizeof (struct inotify_event);
const int EVENT_BUF_LEN = EVENT_SIZE + 256;

PlugNPlay::PlugNPlay(const std::string& path_): m_dispatcher(), 
												m_monitor(std::unique_ptr<DirMonitor>(new DirMonitor(m_dispatcher, path_))),
												m_loader(std::unique_ptr<SOLoader>(new SOLoader(m_dispatcher)))
{

}

PlugNPlay::DirMonitor::DirMonitor(Dispatcher<std::string> &dispatcher_,
                         		  const std::string &path_):
								  m_dir_fd(InotifyInit()),
								  m_wd(InotifyAdd(m_dir_fd,path_)),
								  m_dispatcher(dispatcher_),
								  m_async_listener(),
								  m_logger(Singleton<Logger>::GetInstance()) 
{
	m_async_listener = std::thread(&PlugNPlay::DirMonitor::AsyncListener, this, path_);
}

PlugNPlay::DirMonitor::~DirMonitor()
{
	m_logger->Log(Logger::INFO, "DirMonitor->In destructor body",
				   __FILE__, __LINE__, false); 
	inotify_rm_watch(m_dir_fd, m_wd);	
	close(m_dir_fd);

	if (m_async_listener.joinable())
	{
		m_async_listener.join();
	}
	else
	{
		std::cout << "wtf thread not joinbale\n";
	}
	m_logger->Log(Logger::INFO, "DirMonitor->Finishing destructor",
				   __FILE__, __LINE__, false); 
}

int PlugNPlay::DirMonitor::InotifyInit()
{
	int ret_val = inotify_init();
	if (-1 == ret_val)
	{
		throw PNPError("Failed to inotify_init (creating fd for inotify)");
	}
	return ret_val;
}

int PlugNPlay::DirMonitor::InotifyAdd(int fd_, const std::string &path_)
{
	int ret_val = inotify_add_watch(fd_, path_.c_str(), IN_CREATE);
	if (-1 == ret_val)
	{
		throw PNPError("Failed to inotify_add_watch (creating watch fd)\n");
	}
	return ret_val;
}

void PlugNPlay::DirMonitor::AsyncListener(const std::string &path_)
{

	ssize_t read_length = 0;
	while (true) 
	{
		char buffer[EVENT_BUF_LEN] = {0};

		read_length = read(m_dir_fd, buffer, EVENT_BUF_LEN); 
		if (read_length <= 0)
		{
			break;
		}

		struct inotify_event *event = reinterpret_cast<struct inotify_event*> (&buffer);
		std::cout << "event name = " << event->name << "\n";

		std::string full_path = path_ + "/" + event->name; 
		m_dispatcher.NotifyAll(full_path);
	}
}

PlugNPlay::SOLoader::SOLoader(Dispatcher<std::string> &dispatcher_):
							  m_call_back(&dispatcher_, *this, &PlugNPlay::SOLoader::Load),
							  m_opened_so(),
							  m_logger(Singleton<Logger>::GetInstance())
{

}

PlugNPlay::SOLoader::~SOLoader()
{
	m_logger->Log(Logger::INFO, "SOLoader->In destructor body",
				   __FILE__, __LINE__, false); 
	for (auto i : m_opened_so)
	{
		std::cout << "dlclose ?!?!\n";
		dlclose(i);
	}
	m_logger->Log(Logger::INFO, "SOLoader->Finishing destructor",
				   __FILE__, __LINE__, false); 
}

void PlugNPlay::SOLoader::Load(const std::string &file_name_)
{
	m_opened_so.push_back(dlopen(file_name_.c_str(), RTLD_LAZY | RTLD_GLOBAL));	
}

} //namespace hrd31