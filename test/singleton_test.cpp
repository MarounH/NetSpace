#include <iostream>
#include <thread>
#include <dlfcn.h>

#include "singleton.hpp"
#include "singleton_test_so.hpp"
static void ThreadFunc();

int main(void)
{
    int* A = Singleton<int>::GetInstance();
	
	std::cout << "call 1:" << A << std::endl;
	
	int* B = Singleton<int>::GetInstance();
	
	std::cout << "call 2:" << B << std::endl;
	
	//Function(); // implicit 
	
	void *handle = dlopen("./libsingleton.so", RTLD_LAZY);
	if (nullptr == handle)
	{
		std::cout << "couldn't dlopen.. exiting\n";
		exit(1);
	}	
	
	void (*fun)() = (void (*)())dlsym(handle, "_Z8Functionv");
	if (nullptr == fun)
	{
		std::cout << "couldn't dlsym.. exiting\n";
		exit(1);
	}
	fun();
	
	std::thread t1(ThreadFunc);
	std::thread t2(ThreadFunc);
	std::thread t3(ThreadFunc);

	t1.join();
	t2.join();
	t3.join();
	return 0;
}

static void ThreadFunc()
{
	int *C = Singleton<int>::GetInstance();
	
	std::cout << "thread call:" << C << std::endl;
}
