#include <iostream>

#include "singleton.hpp"



void Function()
{
	int *so_instance = Singleton<int>::GetInstance();
	std::cout << "from so:" << so_instance << std::endl;	
}


