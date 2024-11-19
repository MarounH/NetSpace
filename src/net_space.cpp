#include <iostream>
#include <unistd.h> // close()
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <cstring> // strncmp

#include "i_driver_communicator.hpp"
#include "nbd_driver_communicator.hpp"
#include "i_storage.hpp"
#include "ram_storage.hpp"
#include "driver_data.hpp"
#include "re_fw.hpp"

using namespace hrd31;

enum key {READ_TASK=req_type_t::READ, WRITE_TASK=req_type_t::WRITE,
		  STOP_TASK=20, DO_NOTHING_TASK=30};

class IArgs
{
public:
	explicit IArgs() = default;
	virtual ~IArgs() = default;
	IArgs(const IArgs& o_) = delete;
	IArgs& operator=(const IArgs& o_) = delete;
};

struct NBDArgs: public IArgs
{
	explicit NBDArgs(NBDriverCommunicator& nbd_, RAMStorage& storage_,
					 std::shared_ptr<DriverData> driver_data_):
					 m_nbd(nbd_),
					 m_ram_storage(storage_),
					 m_driver_data(driver_data_)
	{}

	NBDriverCommunicator& m_nbd;
	RAMStorage& m_ram_storage;
	std::shared_ptr<DriverData> m_driver_data;
};

struct StopArgs: public IArgs
{
	explicit StopArgs(RequestEngine<key,std::shared_ptr<IArgs>>& re_):
					  m_re(re_)
	{}

	RequestEngine<key,std::shared_ptr<IArgs>>& m_re;
};

class NBDInput: public IInputSrc<key,std::shared_ptr<IArgs>>
{
public:
	explicit NBDInput(NBDriverCommunicator& nbd_, RAMStorage& storage_):
					  m_nbd(nbd_),
					  m_storage(storage_)
	{}

    std::pair<key,std::shared_ptr<IArgs>> Read() override
	{
		std::shared_ptr<DriverData> driver_data = m_nbd.ReceiveRequest();
		return std::make_pair(key(driver_data->m_type),
							  std::make_shared<NBDArgs> (m_nbd, m_storage, driver_data));
	}
    int GetFd() override
	{
		return m_nbd.GetRequestFD();
	}
	
private:
	NBDriverCommunicator& m_nbd;
	RAMStorage& m_storage;
};

class Input: public IInputSrc<key,std::shared_ptr<IArgs>>
{
public:
	explicit Input(RequestEngine<key,std::shared_ptr<IArgs>>& re_):
					   m_re(re_)
	{}

	std::pair<key,std::shared_ptr<IArgs>> Read() override
	{
		char buffer[256] = {0};
		read(STDIN_FILENO, buffer, 256);
		buffer[2] = '\0';

		if (!strncmp(buffer, "q\n", 2))
		{
			std::cout << "in q\n";
			return std::make_pair(STOP_TASK,
						std::make_shared<StopArgs>(m_re));
		}

		else
		{
			std::cout << "in case of other than q\n";
			return std::make_pair(DO_NOTHING_TASK, std::make_shared<IArgs>());
		}	

	}
	int GetFd() override 
	{
		return STDIN_FILENO;
	}

private:
	RequestEngine<key,std::shared_ptr<IArgs>>& m_re;
};

class NBDReadTask: public IRETask
{
public: 
	explicit NBDReadTask(NBDriverCommunicator& nbd_, RAMStorage& ram_storage_,
						 std::shared_ptr<DriverData> driver_data_):
						 m_nbd(nbd_),
						 m_ram_storage(ram_storage_),
						 m_driver_data(driver_data_)
	{}

	static std::shared_ptr<IRETask> CreateFunc(std::shared_ptr<IArgs> args_)
	{
		std::shared_ptr<NBDArgs> arguments = 
		std::dynamic_pointer_cast<NBDArgs>(args_);

		return std::make_shared<NBDReadTask>(arguments->m_nbd,
											 arguments->m_ram_storage,
											 arguments->m_driver_data);
	}

	void Execute() override
	{
		m_ram_storage.Read(m_driver_data);

		m_nbd.SendReply(m_driver_data);
	}

private:
	NBDriverCommunicator& m_nbd;
	RAMStorage& m_ram_storage;
	std::shared_ptr<DriverData> m_driver_data;
};

class NBDWriteTask: public IRETask
{
public:
	explicit NBDWriteTask(NBDriverCommunicator& nbd_, RAMStorage& ram_storage_,
						 std::shared_ptr<DriverData> driver_data_):
						 m_nbd(nbd_),
						 m_ram_storage(ram_storage_),
						 m_driver_data(driver_data_)
	{}

	static std::shared_ptr<IRETask> CreateFunc(std::shared_ptr<IArgs> args_)
	{
		std::shared_ptr<NBDArgs> arguments = 
		std::dynamic_pointer_cast<NBDArgs>(args_);

		return std::make_shared<NBDWriteTask>(arguments->m_nbd,
											 arguments->m_ram_storage,
											 arguments->m_driver_data);
	}

	void Execute() override
	{

		m_ram_storage.Write(m_driver_data);

		m_nbd.SendReply(m_driver_data);
	}

private: 
	NBDriverCommunicator& m_nbd;
	RAMStorage& m_ram_storage;
	std::shared_ptr<DriverData> m_driver_data;
};

class StopTask: public IRETask
{
public:
	explicit StopTask(RequestEngine<key,std::shared_ptr<IArgs>>& re_):
						 m_re(re_)
	{}

	static std::shared_ptr<IRETask> CreateFunc(std::shared_ptr<IArgs> args_)
	{
		std::shared_ptr<StopArgs> arguments = 
		std::dynamic_pointer_cast<StopArgs>(args_);

		return std::make_shared<StopTask>(arguments->m_re);
	}

	void Execute() override
	{
		std::cout << "in Execute sending stop to request engine\n";
		m_re.Stop();
	}

private:
	RequestEngine<key,std::shared_ptr<IArgs>>& m_re;
};

class DoNoThingTask: public IRETask
{
public:
	static std::shared_ptr<IRETask> CreateFunc(std::shared_ptr<IArgs> args_)
	{
		static_cast<void>(args_);
		return std::make_shared<DoNoThingTask>();
	}

	void Execute() override
	{
		std::cout << "If you meant to stop request engine type 'q'\n";
	}
};

int main()
{
	try
	{
		size_t storage_size = 1000*4096;
		RAMStorage storage(storage_size);
		NBDriverCommunicator nbd_driver("/dev/nbd2", 1000,4096);

		RequestEngine<key,std::shared_ptr<IArgs>> re;

		std::shared_ptr<IInputSrc<key,std::shared_ptr<IArgs>>> 
		nbd_input_src = std::make_shared<NBDInput>(nbd_driver, storage);
		re.ConfigInputSrc(nbd_input_src);
		re.ConfigTask(READ_TASK, NBDReadTask::CreateFunc);
		re.ConfigTask(WRITE_TASK, NBDWriteTask::CreateFunc);

		std::shared_ptr<IInputSrc<key,std::shared_ptr<IArgs>>>
		input_src = std::make_shared<Input>(re);
		re.ConfigInputSrc(input_src);
		re.ConfigTask(STOP_TASK, StopTask::CreateFunc);
		re.ConfigTask(DO_NOTHING_TASK, DoNoThingTask::CreateFunc);

		re.Run();

		// sleep(2);
		// re.Stop();

	}
	catch (ReqEngineError& exception_)
	{
		std::cerr << exception_.what();
		return 1;
	}
	catch (DriverError &exception_)
	{
		std::cerr << exception_.what();
		int ret = write(STDIN_FILENO, "q\n", sizeof("q\n"));
		if (-1 == ret)
		{
			std::cout << "Failed to write\n";
		}
		//return 1;
	}
	catch (PNPError &exception_)
	{
		std::cerr << exception_.what();
	}
	catch (...)
	{
		std::cerr << "idk what happened\n";
		return 1;
	}
	return 0;
}

//OLD main.. before implementing the request engine frame-work api
// int main(void)
// {
// 	try
// 	{
// 		size_t storage_size = 10000*4096;
// 		NBDriverCommunicator nbd_driver("/dev/nbd1", 10000,4096);
// 		RAMStorage storage(storage_size);
		
// 		const int MAX_EVENTS = 2;
// 		int epoll_fd = epoll_create1(0);

// 		if (epoll_fd == -1) 
// 		{
// 			std::cout << "Failed to create epoll file descriptor\n";
// 			exit(1);
// 		}
		
// 		struct epoll_event event, events[MAX_EVENTS];
// 		event.events = EPOLLIN;
		
// 		int input_fd = fileno(stdin);
// 		event.data.fd = input_fd;
// 		if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fd, &event))
// 		{
// 			std::cout << "Failed to add file descriptor to epoll\n";
// 			close(epoll_fd);
// 			return 1;
// 		}
		
// 		int request_fd = nbd_driver.GetRequestFD();
// 		event.data.fd = request_fd;
// 		if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, request_fd, &event))
// 		{
// 			std::cout << "Failed to add file descriptor to epoll\n";
// 			close(epoll_fd);
// 			return 1;
// 		}
		
// 		while(NBDriverCommunicator::g_nbd_fd)
// 		{
// 			int i = 0;
// 			int n_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
// 			for (i = 0; i < n_fds; i++) 
// 			{
// 				if (request_fd == events[i].data.fd)
// 				{
// 				std::shared_ptr<DriverData> received_request(nbd_driver.ReceiveRequest());	
// 					switch(received_request->m_type)
// 					{
// 						case READ:
// 						{
// 							std::cout << "\nRequest for read of size " <<
// 										 received_request->m_len << std::endl;
				
// 							storage.Read(received_request);
// 							break;	
// 						}
						
// 						case WRITE:
// 						{
// 							std::cout << "Request for write of size " << 
// 										 received_request->m_len << std::endl;
							
// 							storage.Write(received_request);
							
// 							break;
// 						}
						
// 					}
					
// 					nbd_driver.SendReply(received_request);
// 				}
				
// 				std::string input;
// 				if (input_fd == events[i].data.fd)
// 				{
// 					std::getline(std::cin, input);
// 				}
				
// 				if(!strncmp(input.c_str(), "q", 1))
// 				{
// 					NBDriverCommunicator::g_nbd_fd = 0;
// 					std::cout << "received q... exiting ""NetSpace""\n";
// 					break;
// 				}
// 			}

// 		}
	
// 	}
	
// 	catch (DriverError &exception_)
// 	{
// 		std::cerr << exception_.what();
// 		return 1;
// 	}
// 	return 0;
// }


