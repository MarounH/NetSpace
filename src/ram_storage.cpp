#include <iostream>

#include "ram_storage.hpp"

namespace hrd31
{
    RAMStorage::RAMStorage(size_t size_): IStorage(size_), m_storage(size_) //may throw bad_alloc
    {}
    
    RAMStorage::~RAMStorage() 
    {}
    
    RAMStorage::RAMStorage(const RAMStorage& other_): IStorage(other_.m_storage.size()), m_storage(other_.m_storage) //may throw bad_alloc
    {}

    void RAMStorage::Read(std::shared_ptr<DriverData> read_data_) const//may throw bad_read, bad_write
    {
    	std::cout << "Read len " << read_data_->m_len << " from storage with offset " 
    	<< read_data_->m_offset << std::endl;
    	
    	std::copy(m_storage.begin() + read_data_->m_offset,
    			  m_storage.begin() + read_data_->m_offset + read_data_->m_len,
    			  read_data_->m_data.begin());
    }
    
    void RAMStorage::Write(std::shared_ptr<DriverData> write_data_) //may throw bad_write
    {
    	
    	std::cout << "Write len " << write_data_->m_len << " from storage with offset "
    	<< write_data_->m_offset << std::endl;
    	
    	std::copy(write_data_->m_data.begin(),
    			  write_data_->m_data.begin() + write_data_->m_len,
    			  m_storage.begin() + write_data_->m_offset);
    }
}//namespace hrd31
