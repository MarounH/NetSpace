#include <iostream>

#include "serializable_text.hpp"

using namespace hrd31;

int main()
{
	nbd_request request;
	DriverData driver_data(request);
	driver_data.m_type = READ;
	driver_data.m_handle = 123456;
	driver_data.m_offset = 4096;
	driver_data.m_len = 64;
	driver_data.m_status = SUCCESS;
	std::vector<char> buffer(64,0);
	driver_data.m_data = buffer;

	std::shared_ptr<DriverData> driver_data_shared = 
	std::make_shared<DriverData>(driver_data);
	std::shared_ptr<SerializableText> serializer = 
	std::make_shared<SerializableText>();

	std::shared_ptr<std::vector<char>> serialized_buff = 
	serializer->Serialize(driver_data_shared);

	std::cout << serialized_buff->data() << std::endl;
	std::shared_ptr<DriverData> deserialized_driver_data = 
	serializer->Deserialize(serialized_buff);

	std::cout << "m_type = " << deserialized_driver_data->m_type 
			  << "m_handle = " << deserialized_driver_data->m_handle
			  << "m_offset = " << deserialized_driver_data->m_offset
			  << "m_len = " << deserialized_driver_data->m_len
			  << "m_status = " << deserialized_driver_data->m_status
			  << std::endl;
	return 0;
}
