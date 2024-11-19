/*
Description:
Author: HRD31
Reviewer:
Version:
    v0.1 - For mentor approval
*/

#ifndef __HRD31_I_SERIALIZABLE_HPP__
#define __HRD31_I_SERIALIZABLE_HPP__

#include <cstddef>
#include <memory>
#include <vector>

#include "driver_data.hpp"

namespace hrd31
{

class ISerializable
{
public:
    using Buffer = std::shared_ptr<std::vector<char>>;

    explicit ISerializable() = default;
    virtual ~ISerializable() noexcept = default;
    
    ISerializable(const ISerializable& other_) = delete;
    ISerializable& operator=(const ISerializable& other_) = delete;

    virtual std::shared_ptr<DriverData> Deserialize(Buffer ser_buff_) = 0;
    virtual Buffer Serialize(std::shared_ptr<DriverData> data_) = 0;
};

}//namespace hrd31

#endif //__HRD31_I_SERIALIZABLE_HPP__