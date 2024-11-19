/*
Description:
Author:
Reviewer:
Version:
    v0.1 - For mentor approval
*/
#ifndef __HRD31_DRIVER_DATA_HPP__
#define __HRD31_DRIVER_DATA_HPP__

#include <cstddef> //size_t
#include <linux/nbd.h>
#include <vector>

namespace hrd31
{
typedef enum {READ, WRITE} req_type_t;
typedef enum {SUCCESS, FAIL} status_t;

struct DriverData
{
    explicit DriverData(nbd_request& request_); //may throw bad_alloc, bad_read
    ~DriverData() = default;
    DriverData(const DriverData& other_) = default;
    DriverData& operator=(const DriverData& other_) = delete;

    req_type_t m_type;
	size_t m_handle;
	size_t m_offset;
	size_t m_len;
    status_t m_status;

    std::vector<char> m_data;
    
    private:
    static size_t Ntohll(size_t num_); 
};

}//namespace hrd31


//read(fd, &nbd_req, sizeof(nbd_req));
//DriverData req(nbd_req);
//  m_data(m_len);
//  read(fd, m_data.data(), req.len);

// struct nbd_reply {
// 	__be32 magic;
// 	__be32 error;		/* 0 = ok, else error	*/
// 	char handle[8];		/* handle you got from request	*/
// };

#endif //__HRD31_DRIVER_DATA_HPP__
