#include "json.hpp"
#include "serializable_text.hpp"

#include <iostream> // to be deleted
using namespace hrd31;

std::shared_ptr<DriverData> SerializableText::Deserialize(Buffer ser_buff_) 
{
	std::string data_str(ser_buff_->begin(), ser_buff_->end());

	nlohmann::json json = nlohmann::json::parse(data_str);

	nbd_request request; 
	DriverData driver_data(request);
	driver_data.m_type = json["req_type"];
	driver_data.m_handle = json["handle"];
	driver_data.m_offset = json["offset"];
	driver_data.m_len = json["len"];
	driver_data.m_status = json["status"];
	driver_data.m_data = json["data"];

	std::shared_ptr<DriverData> json_deserialized =
	std::make_shared<DriverData>(driver_data);

	return json_deserialized;
}

SerializableText::Buffer SerializableText::Serialize(std::shared_ptr<DriverData> data_) 
{
	size_t used_size = 73 + sizeof(data_->m_type)/sizeof(req_type_t) 
								   + sizeof(data_->m_status)/sizeof(status_t)
								   + CalculateNumOfDigits(data_->m_handle)
								   + CalculateNumOfDigits(data_->m_offset)
								   + CalculateNumOfDigits(data_->m_len);
	size_t data_size = data_->m_data.size();
	if (data_size)
	{
		data_size = data_size*2-1;
		used_size += data_size;
	}
	const size_t block_size = 4096;
	const size_t padding_size = used_size-block_size;
	std::vector<char> v; 
	std::cout << "sizeof v = " << sizeof(v) << std::endl;
	std::shared_ptr<std::vector<char>> padding = 
	std::make_shared<std::vector<char>>(padding_size-24,0);

	//std::cout << padding << std::endl;
	std::cout << "json calculated size = " << (used_size) << std::endl;
	nlohmann::json json {
							{"req_type", data_->m_type},
							{"handle", data_->m_handle},
							{"offset", data_->m_offset},
							{"len", data_->m_len},
							{"status", data_->m_status},
							{"data", data_->m_data},
							{"padding", 1}
						};
	std::string json_str = json.dump();
	std::cout << "json size = " << json_str.size() << std::endl;
	std::vector<char> json_vec(json_str.begin(), json_str.end());

	std::shared_ptr<std::vector<char>> json_serialized =
	std::make_shared<std::vector<char>>(json_vec);

	return json_serialized;
}

size_t SerializableText::CalculateNumOfDigits(size_t num_)
{
	size_t digits_count = 0;
	while (num_)
	{
		++digits_count;
		num_ /= 10;
	}
	return digits_count;
}

