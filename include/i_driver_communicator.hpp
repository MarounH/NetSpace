/*
Description:
Author:
Reviewer:
Version:
    v0.1 - For mentor approval
*/

#ifndef __HRD31_I_DRIVER_COMMUNICATOR_HPP__
#define __HRD31_I_DRIVER_COMMUNICATOR_HPP__

#include <memory> // shared_ptr
#include <stdexcept> //std::runetime_error 

#include "driver_data.hpp"

namespace hrd31
{
class DriverError : public std::runtime_error 
{
public:
	explicit DriverError(const std::string what_): runtime_error(what_)
	{}
};

class IDriverCommunicator
{
public:
    explicit IDriverCommunicator() = default;
    virtual ~IDriverCommunicator() noexcept = default;
    IDriverCommunicator(const IDriverCommunicator& other_) = delete;
    IDriverCommunicator& operator=(const IDriverCommunicator& other_) = delete;
    virtual std::shared_ptr<DriverData> ReceiveRequest() = 0;
    virtual void SendReply(std::shared_ptr<DriverData> data_) = 0;
    virtual void Disconnect() = 0;
    virtual int GetRequestFD() = 0;
};
}//namespace hrd31

#endif //__HRD31_I_DRIVER_COMMUNICATOR_HPP__
