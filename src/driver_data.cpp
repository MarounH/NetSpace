#include <iostream>
#include <linux/nbd.h>
#include <arpa/inet.h>


#include "driver_data.hpp"

namespace hrd31
{
    DriverData::DriverData(nbd_request& request_): m_type(ntohl(request_.type)
    								   == NBD_CMD_READ ? READ:WRITE),
    								   m_handle(*(size_t*)request_.handle),
    								   m_offset(Ntohll(request_.from)),
    								   m_len(ntohl(request_.len)),
    								   m_status((status_t)ntohl(0)),
    								   m_data(m_len)
    								    //may throw bad_alloc, bad_read
    {}

    size_t DriverData::Ntohll(size_t num_)
    {
    	unsigned int low = num_ & 0xffffffff;
    	unsigned int high = num_ >> 32U;
    	low = ntohl(low);
    	high = ntohl(high);
    	return ((size_t) low) << 32U | high;
    } 
}// namespace hrd31
