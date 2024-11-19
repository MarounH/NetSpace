/*
Description:
Author:
Reviewer:
Version:
    v0.1 - For mentor approval
*/

#ifndef __HRD31_NBD_DRIVER_COMMUNICATOR_HPP__
#define __HRD31_NBD_DRIVER_COMMUNICATOR_HPP__

#include <string>
#include <thread>
#include <stdexcept>

#include "i_driver_communicator.hpp"
#include "driver_data.hpp"
#include "logger.hpp"

namespace hrd31
{

class NBDError : public DriverError 
{
public:
	explicit NBDError(const std::string what_): DriverError(what_)
	{}
};

class NBDriverCommunicator : public IDriverCommunicator
{
public:
    explicit NBDriverCommunicator(const std::string& dev_path_,
            size_t storage_size_); //may throw bad_open, bad_read, bad_signal, bad_ioctl
    explicit NBDriverCommunicator(const std::string& dev_path_, size_t blocks_num_, size_t block_size_);
    ~NBDriverCommunicator() noexcept override;
    NBDriverCommunicator(const NBDriverCommunicator& other_) = delete;
    NBDriverCommunicator& operator=(const NBDriverCommunicator& other_) = delete;
    
    std::shared_ptr<DriverData> ReceiveRequest() override; //may throw bad_read
    void SendReply(std::shared_ptr<DriverData> data_) override; //may throw bad_write
    void Disconnect() override; //may throw bad_close, bad_ioctl, bad_join 
	int GetRequestFD() override;
	static int g_nbd_fd; // not good !! ok for now 
	static int g_app_socket;
private:
    int m_nbd_socket;
    int m_app_socket;
    int m_nbd_fd;
    std::thread m_set_clean;
    Logger *m_logger;
    
    // storage_size_ can act as storage size or as blocks num
    void Setup(const std::string& dev_path_, size_t num_of_blocks_, 
    		   size_t block_size_);
	static void SetSize(int nbd_fd_, size_t num_of_blocks_, size_t block_size_);
	static void DoIt(int nbd_fd_, int m_nbd_socket_);
	static void DisconnectHandler(int signal_);
	static void BlockAllSignals();
	static void AllowSignals(int sig_arr_[], int sig_arr_size_, 
							 void (*sig_handler_)(int));
};
}//namespace hrd31

#endif //__HRD31_NBD_DRIVER_COMMUNICATOR_HPP__
