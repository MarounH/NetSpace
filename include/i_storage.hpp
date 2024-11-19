/*
Description:
Author:
Reviewer:
Version:
    v0.1 - For mentor approval
*/

#ifndef __HRD31_I_STORAGE_HPP__
#define __HRD31_I_STORAGE_HPP__

#include <cstddef> /* size_t */
#include <memory>
#include <stdexcept>

#include "driver_data.hpp"

namespace hrd31
{
class StorageError : public std::runtime_error {};

class IStorage
{
public:
    explicit IStorage(size_t size_)
    {
        size_t unused = size_;
        unused++;
    } // was too lazy to make a cpp for it 
    virtual ~IStorage() noexcept = default;
    IStorage(const IStorage& other_) = delete;
    IStorage& operator=(const IStorage& other_) = delete;

    virtual void Read(std::shared_ptr<DriverData> read_data_) const = 0 ;
    virtual void Write(std::shared_ptr<DriverData> write_data_) = 0;
};

}//namespace hrd31
#endif //__HRD31_I_STORAGE_HPP__
