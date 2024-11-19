#include <iostream>
#include <sys/types.h> // open(), socketpair()
#include <sys/stat.h> // open() 
#include <fcntl.h> // open()
#include <sys/socket.h> // socketpair()
#include <unistd.h> // close()
#include <sys/ioctl.h> // ioctl()
#include <thread> 
#include <signal.h>
#include <arpa/inet.h> //ntohl()
#include <string.h> // std::to_string() 

#include "nbd_driver_communicator.hpp"

namespace hrd31
{
	int NBDriverCommunicator::g_nbd_fd;
	int NBDriverCommunicator::g_app_socket;
	
	NBDriverCommunicator::NBDriverCommunicator
	(const std::string& dev_path_, size_t storage_size_):
	m_logger(Singleton<Logger>::GetInstance()) 
	
    {
    	m_logger->SetLogLevel(Logger::INFO);
    	m_logger->Log(Logger::INFO, "In constructor body", __FILE__, __LINE__, true); 
    	m_logger->Log(Logger::DEBUG, "Entering Setup with dev_path: " +
    						  std::string(dev_path_) +
    						  " with 1 block_size and storage_size: " +
    						  std::to_string(storage_size_), __FILE__, __LINE__);
    						  
   		Setup(dev_path_, 0, storage_size_);
   		
   		m_logger->Log(Logger::INFO, "End of constructor body", __FILE__, __LINE__);
    }
    
	NBDriverCommunicator::NBDriverCommunicator
	(const std::string& dev_path_,size_t blocks_num_, size_t block_size_):
	m_logger(Singleton<Logger>::GetInstance())
	
   	{
   		m_logger->SetLogLevel(Logger::INFO);
    	m_logger->Log(Logger::INFO, "In constructor body", __FILE__, __LINE__);
    	m_logger->Log(Logger::DEBUG, "Entering Setup with dev_path: " +
    						  std::string(dev_path_) +
    						  " with " + std::to_string(blocks_num_) + "blocks and "
    						  + std::to_string(block_size_) + "block size"
    						  , __FILE__, __LINE__);   
    						   	 
   		Setup(dev_path_, blocks_num_, block_size_);
   		
   		m_logger->Log(Logger::INFO, "End of constructor body", __FILE__, __LINE__);
   	}
   	
	NBDriverCommunicator::~NBDriverCommunicator()  
	{
		std::cout << "beggining of  dtor NBDriverCommunicator\n";
    	m_logger->Log(Logger::INFO, "In destructor body", __FILE__, __LINE__); 
		Disconnect();
   		m_logger->Log(Logger::INFO, "End of destructor body", __FILE__, __LINE__);
		std::cout << "after dtor NBDriverCommunicator\n";
	}										
    
    std::shared_ptr<DriverData> NBDriverCommunicator::ReceiveRequest()  //may throw bad_read
    {
    	struct nbd_request request;
    	
	  	read(m_app_socket, &request, sizeof(request));
		
    	std::shared_ptr<DriverData> received_request(new DriverData(request));
		if (WRITE == received_request->m_type)
		{
			read(m_app_socket, received_request->m_data.data(), received_request->m_len);
		}
		
    	return received_request;
    }
    
    void NBDriverCommunicator::SendReply(std::shared_ptr<DriverData> data_)  //may throw bad_write
    {
		struct nbd_reply reply;
		
		reply.magic = htonl(NBD_REPLY_MAGIC);
		reply.error = htonl(data_->m_status);
		*(size_t*)reply.handle = data_->m_handle;

		static std::mutex mutex;
		std::lock_guard<std::mutex> lock(mutex);
		
		write(m_app_socket, &reply, sizeof(reply));
		
		if (READ == data_->m_type)
		{
			write(m_app_socket, data_->m_data.data(), data_->m_len);
		}
    }
    
    void NBDriverCommunicator::Disconnect()  //may throw bad_close, bad_ioctl, bad_join 
    {
		
		close(m_app_socket);
		
    	std::cout << "before join\n";
		m_set_clean.join();
		std::cout << "after join\n";
		
		
    }
    
	int NBDriverCommunicator::GetRequestFD()
	{
		return m_app_socket;
	}
	
    void NBDriverCommunicator::Setup(const std::string& dev_path_,
    								 size_t num_of_blocks_, size_t block_size_)
	{
		int socket_pair[2]; 
		
		m_nbd_fd = open(dev_path_.data(), O_RDWR);
		if (-1 == m_nbd_fd) 
		{
			throw DriverError("NetSpace... exception received:\n"
							  "Failed to open nbd file descriptor\n");
		}
		
		int ret;
		ret = socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair); //TODO RAII
		if (-1 == ret)
		{
			close(m_nbd_fd); // can be a class then dtor will be called automatically
			throw DriverError("NetSpace... exception received:\n"
				  "Failed to create socketpair\n");
		}
		
		m_nbd_socket = socket_pair[1];
		m_app_socket = socket_pair[0];
		g_nbd_fd = m_nbd_fd;
		
		SetSize(m_nbd_fd, num_of_blocks_, block_size_);
		
		BlockAllSignals();
		
		m_set_clean = std::thread(&DoIt, m_nbd_fd, m_nbd_socket);
		
		int sig_arr[] = {SIGINT, SIGTERM};
		AllowSignals(sig_arr, sizeof(sig_arr)/sizeof(int), DisconnectHandler);
		
    }
    
	void NBDriverCommunicator::SetSize(int nbd_fd_, size_t num_of_blocks_
											  ,size_t block_size_)
	{
		int ret;
		if (num_of_blocks_)
		{
			ret = ioctl(nbd_fd_, NBD_SET_SIZE_BLOCKS, num_of_blocks_);
			if (-1 == ret)
			{
				throw DriverError("NetSpace... exception received:\n"
 				  				  "could not ioctl NBD_SET_SIZE_BLOCKS\n");
			}
			
			ret = ioctl(nbd_fd_, NBD_SET_BLKSIZE, block_size_);
			if (-1 == ret)
			{
				throw DriverError("NetSpace... exception received:\n"
				 				  "could not ioctl NBD_SET_BLKSIZE\n");
			}
			
			ret = ioctl(nbd_fd_, NBD_CLEAR_SOCK);
			if (-1 == ret)
			{
				throw DriverError("NetSpace... exception received:\n"
 				  				  "could not ioctl NBD_CLEAR_SOCK\n");
			}
		}
		else
		{
			ret = ioctl(nbd_fd_, NBD_SET_SIZE, block_size_);
			if (-1 == ret)
			{
				throw DriverError("NetSpace... exception received:\n"
 				  				  "could not ioctl NBD_SET_SIZE\n");
			}
			
			ret = ioctl(nbd_fd_, NBD_CLEAR_SOCK);
			if (-1 == ret)
			{
				throw DriverError("NetSpace... exception received:\n"
 				  				  "could not ioctl NBD_CLEAR_SOCK\n");
			}
		}
	}
	
	void NBDriverCommunicator::DoIt(int nbd_fd_, int m_nbd_socket_)
	{
		int ret;
	    
		ret = ioctl(nbd_fd_, NBD_SET_SOCK, m_nbd_socket_);
		if (-1 == ret)
		{
			throw DriverError("NetSpace... exception received:\n"
  				  			  "could not ioctl NBD_SET_SOCK\n");
		}
		
		ret = ioctl(nbd_fd_, NBD_DO_IT);
		if (-1 == ret) 
		{
			throw DriverError("NetSpace... exception received:\n"
  				  			  "could not ioctl NBD_DO_IT\n");
		}
	}
	
	void NBDriverCommunicator::DisconnectHandler(int signal_)
	{
		if (SIGINT != signal_ && SIGTERM != signal_)
		{
			std::cout << "received wrong signal idk what to do about it\n";
		}
		
		// int ret;
		// ret = ioctl(NBDriverCommunicator::g_nbd_fd, NBD_DISCONNECT);
		// if (-1 == ret)
		// {
		// 	throw DriverError("NetSpace... exception received:\n"
	  	// 		  			  "could not ioctl NBD_DISCONNECT\n");
		// }
		
		//else 
		{
			std::cout << "\nDisconnecting gracefully\n";

			//g_nbd_fd = 0; // to exit loop in net_space.cpp(main)
			// int ret2 = write(STDERR_FILENO, "aa\n", sizeof("q\n"));
			// if (-1 == ret2)
			// {
			// 	std::cout << "couldn't write to STDIN_FILENO\n";
			// }

			// int ret1 = write(STDERR_FILENO, "q", sizeof("q"));
			// if (-1 == ret1)
			// {
			// 	std::cout << "couldn't write to STDIN_FILENO\n";
			// }
			// std::cout << "In signal handler.. g_nbd_fd = " << g_nbd_fd << std::endl;
		}
	}
	
	void NBDriverCommunicator::BlockAllSignals()
	{
		sigset_t sigset;

		if (sigfillset(&sigset) || sigprocmask(SIG_SETMASK, &sigset, NULL))
		{
			throw DriverError("NetSpace... exception received:\n"
	  			  			  "Failed to block signals\n");		
		}
	}
	
	void NBDriverCommunicator::AllowSignals(int sig_arr_[], int sig_arr_size_, 
											void (*sig_handler_)(int))
	{
		sigset_t sig_set;
		int ret;
		ret = sigemptyset(&sig_set);
		if (-1 == ret)
		{
			throw DriverError("NetSpace... exception received:\n"
	  			  			  "Failed to allow signals\n");	
		}
		
		int i = 0;
		for (i = 0; i < sig_arr_size_; ++i)
		{
			ret = sigaddset(&sig_set, sig_arr_[i]);
			if (-1 == ret)
			{
				throw DriverError("NetSpace... exception received:\n"
	  			  			  	  "Failed to allow signals\n");	
			}
		}
		
		ret = pthread_sigmask(SIG_UNBLOCK, &sig_set, NULL);
		if (-1 == ret)
		{
			throw DriverError("NetSpace... exception received:\n"
	  			  			  "Failed to allow signals\n");	
		}
		
		struct sigaction act;
		act.sa_handler = sig_handler_;
		act.sa_mask = sig_set;
		for (i = 0; i < sig_arr_size_; ++i)
		{
			ret = sigaction(sig_arr_[i], &act, NULL);
			if (-1 == ret)
			{
				throw DriverError("NetSpace... exception received:\n"
	  			  			  	  "Failed to allow signals\n");	
			}
		}
	}
};// namespace hrd31
