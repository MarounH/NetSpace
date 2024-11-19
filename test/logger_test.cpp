#include <iostream>

#include "logger.hpp"

using namespace hrd31;

int main(void)
{
	Logger *logger = Singleton<Logger>::GetInstance();
	
	logger->Log(Logger::ERROR, "hi", __FILE__, __LINE__);
	
	logger->Log(Logger::INFO, "shouldn't print", __FILE__, __LINE__);
	
	logger->Log(Logger::ERROR, "Print to std::cout as well", __FILE__, __LINE__, 1);
	
	return 0;
}
